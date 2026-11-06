#!/usr/bin/env python3
"""Build the vtable slot name map for the engine and game classes that the blocked bodies call.

Inputs (read only):
  - UE 4.25 engine headers (ENGINE_SRC): the MSVC virtual declaration order of the engine chain.
  - The runtime dump image porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin
    (file offset = VA - 0x140000000) and its PE section table and .pdata.
  - porting/generated/reflection/native_class_index.json: the GetPrivateStaticClass address of each class.
  - porting/generated/reflection/class_parent_map.json: the parent chain of each class.

Method (see porting/behavior/vtable_slots.md):
  1. Model: parse each engine header with the Shipping macro set, take the virtual declarations of each class in
     order, and build the table as MSVC does (base table first, an override keeps its slot, a new virtual is
     appended; a virtual destructor is one slot, the scalar deleting destructor).
  2. Dump: find each class's primary vtable from its constructor (GetPrivateStaticClass -> InternalConstructor
     thunk -> constructor -> the last `lea rax,[vtable]; mov [reg],rax` with no displacement).
  3. Validate: the model against the dump, see vtable_slots.md for each test.
Output: porting/behavior/vtable_slots.json and porting/behavior/vtable_slots.md (run with python3 -I).
"""
import bisect
import collections
import concurrent.futures
import json
import mmap
import os
import re
import struct
import subprocess
import sys
import tempfile

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
ENGINE_SRC = '/opt/fortnite/UnrealEngine/Engine/Source/Runtime'
BIN = os.path.join(REPO, 'porting', 'generated', 'dump', 'FortniteClient-Win64-Shipping.runtime.bin')
GPSC_INDEX = os.path.join(REPO, 'porting', 'generated', 'reflection', 'native_class_index.json')
PARENT_MAP = os.path.join(REPO, 'porting', 'generated', 'reflection', 'class_parent_map.json')
OUT_JSON = os.path.join(REPO, 'porting', 'behavior', 'vtable_slots.json')
OUT_MD = os.path.join(REPO, 'porting', 'behavior', 'vtable_slots.md')
BASE = 0x140000000

# Shipping client macro values for the preprocessor (UE 4.25 Build.h and CoreMiscDefines.h).
MACROS = {
    'WITH_EDITOR': 0, 'WITH_EDITORONLY_DATA': 0, 'WITH_ENGINE': 1, 'WITH_SERVER_CODE': 1,
    'UE_BUILD_SHIPPING': 1, 'UE_BUILD_TEST': 0, 'UE_BUILD_DEVELOPMENT': 0, 'UE_EDITOR': 0,
    'UE_SERVER': 0, 'STATS': 0, 'ENABLE_STATNAMEDEVENTS_UOBJECT': 0, 'WITH_HOT_RELOAD': 0,
    'USE_MALLOC_PROFILER': 0, 'DO_CHECK': 0, 'HACK_HEADER_GENERATOR': 0, 'WITH_UNREAL_DEVELOPER_TOOLS': 0,
    'WITH_AUTOMATION_TESTS': 0, 'WITH_PLUGIN_SUPPORT': 0, 'WITH_PHYSX': 1, 'WITH_CHAOS': 0,
    '_MSC_VER': 1924, 'USE_LIGHTWEIGHT_UOBJECT_STATS_FOR_HITCH_DETECTION': 0,
}

# Engine chain: class -> header under ENGINE_SRC. Primary base is read from the header.
HEADERS = {
    'UObjectBase': 'CoreUObject/Public/UObject/UObjectBase.h',
    'UObjectBaseUtility': 'CoreUObject/Public/UObject/UObjectBaseUtility.h',
    'UObject': 'CoreUObject/Public/UObject/Object.h',
    'AActor': 'Engine/Classes/GameFramework/Actor.h',
    'APawn': 'Engine/Classes/GameFramework/Pawn.h',
    'ACharacter': 'Engine/Classes/GameFramework/Character.h',
    'AController': 'Engine/Classes/GameFramework/Controller.h',
    'APlayerController': 'Engine/Classes/GameFramework/PlayerController.h',
    'AInfo': 'Engine/Classes/GameFramework/Info.h',
    'AGameStateBase': 'Engine/Classes/GameFramework/GameStateBase.h',
    'AGameState': 'Engine/Classes/GameFramework/GameState.h',
    'APlayerState': 'Engine/Classes/GameFramework/PlayerState.h',
    'AGameModeBase': 'Engine/Classes/GameFramework/GameModeBase.h',
    'AGameMode': 'Engine/Classes/GameFramework/GameMode.h',
    'UActorComponent': 'Engine/Classes/Components/ActorComponent.h',
    'USceneComponent': 'Engine/Classes/Components/SceneComponent.h',
    'UPrimitiveComponent': 'Engine/Classes/Components/PrimitiveComponent.h',
    'UNavMovementComponent': 'Engine/Classes/GameFramework/NavMovementComponent.h',
    'UMovementComponent': 'Engine/Classes/GameFramework/MovementComponent.h',
    'UPawnMovementComponent': 'Engine/Classes/GameFramework/PawnMovementComponent.h',
    'UCharacterMovementComponent': 'Engine/Classes/GameFramework/CharacterMovementComponent.h',
    'APlayerCameraManager': 'Engine/Classes/Camera/PlayerCameraManager.h',
}

# Stub bodies folded by the linker (blocks/porting notes): an inline empty body is this address,
# an inline `return true;` is the next one, `return false;` the third.
STUB_ADDR = {'empty': 0x1406801c0, 'true': 0x1406801d0, 'false': 0x1406801e0}

# ---------------------------------------------------------------- preprocessing

def strip_comments_and_strings(text):
    """Remove comments; blank out the contents of string and char literals (keeps the quotes)."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == '/' and i + 1 < n and text[i + 1] == '/':
            j = text.find('\n', i)
            i = n if j < 0 else j
            continue
        if c == '/' and i + 1 < n and text[i + 1] == '*':
            j = text.find('*/', i + 2)
            seg = text[i:n if j < 0 else j + 2]
            out.append('\n' * seg.count('\n'))
            i = n if j < 0 else j + 2
            continue
        if c in '"\'':
            q = c
            out.append(q)
            i += 1
            while i < n and text[i] != q:
                if text[i] == '\\':
                    i += 1
                elif text[i] == '\n':
                    break
                i += 1
            out.append(q)
            i += 1
            continue
        out.append(c)
        i += 1
    s = ''.join(out)
    return s.replace('\\\n', ' ')


class _Expr:
    """Tiny #if expression evaluator (defined, !, &&, ||, comparisons, parentheses, integers)."""
    TOK = re.compile(r'\s*(defined|[A-Za-z_]\w*|0x[0-9a-fA-F]+|\d+|&&|\|\||==|!=|>=|<=|[()!<>])')

    def __init__(self, text, macros):
        self.t = [m.group(1) for m in self.TOK.finditer(text)]
        self.i = 0
        self.m = macros

    def val(self):
        return self.expr()

    def peek(self):
        return self.t[self.i] if self.i < len(self.t) else None

    def take(self):
        tok = self.t[self.i]
        self.i += 1
        return tok

    def expr(self):
        v = self.and_()
        while self.peek() == '||':
            self.take()
            v = self.and_() or v
        return v

    def and_(self):
        v = self.unary()
        while self.peek() == '&&':
            self.take()
            r = self.unary()
            v = bool(v) and bool(r)
        return v

    def unary(self):
        if self.peek() == '!':
            self.take()
            return not self.unary()
        return self.cmp()

    def cmp(self):
        a = self.prim()
        if self.peek() in ('==', '!=', '>=', '<=', '<', '>'):
            op = self.take()
            b = self.prim()
            return {'==': a == b, '!=': a != b, '>=': a >= b, '<=': a <= b, '<': a < b, '>': a > b}[op]
        return a

    def prim(self):
        tok = self.take()
        if tok == '(':
            v = self.expr()
            self.take()
            return v
        if tok == 'defined':
            if self.peek() == '(':
                self.take()
                name = self.take()
                self.take()
            else:
                name = self.take()
            return name in self.m
        if tok.startswith('0x'):
            return int(tok, 16)
        if tok.isdigit():
            return int(tok)
        if tok in self.m:
            return self.m[tok]
        return 0


def preprocess(text):
    """Keep only the lines of the active conditional branches (Shipping macro set)."""
    lines = text.split('\n')
    out = []
    stack = []  # (active_here, taken_any, parent_active)
    active = True
    for ln in lines:
        s = ln.strip()
        if s.startswith('#'):
            d = re.match(r'#\s*(\w+)\s*(.*)$', s)
            kw, rest = (d.group(1), d.group(2)) if d else ('', '')
            if kw in ('if', 'ifdef', 'ifndef'):
                if kw == 'if':
                    cond = bool(_Expr(rest, MACROS).val())
                elif kw == 'ifdef':
                    cond = rest.split()[0] in MACROS if rest.split() else False
                else:
                    cond = not (rest.split()[0] in MACROS if rest.split() else False)
                parent = active
                stack.append((parent, cond, cond))
                active = parent and cond
            elif kw == 'elif':
                parent, _, _ = stack[-1]
                cond = bool(_Expr(rest, MACROS).val())
                taken = stack[-1][2] or cond
                stack[-1] = (parent, cond and not stack[-1][2], taken)
                active = parent and stack[-1][1]
            elif kw == 'else':
                parent, cur, taken = stack[-1]
                stack[-1] = (parent, not taken, True)
                active = parent and not taken
            elif kw == 'endif':
                if stack:
                    parent, _, _ = stack.pop()
                    active = parent
            out.append('')
            continue
        out.append(ln if active else '')
    return '\n'.join(out)


# ---------------------------------------------------------------- header parsing

ATTR_MACROS = re.compile(r'\b(UFUNCTION|UPROPERTY|UMETA|UPARAM|DEPRECATED|UE_DEPRECATED|GENERATED_BODY|'
                         r'GENERATED_UCLASS_BODY|GENERATED_UINTERFACE_BODY|UE_DEPRECATED_FORGAME|'
                         r'DEPRECATED_FORGAME|ENGINE_API|COREUOBJECT_API|FORCEINLINE|FORCEINLINE_DEBUGGABLE|'
                         r'GAMEPLAYTASKS_API|UMG_API|DEPRECATED_MACRO|ENGINE_VTABLE)\b')


def _strip_attrs(s):
    # remove MACRO(...) groups with balanced parentheses
    out = []
    i = 0
    while i < len(s):
        m = ATTR_MACROS.match(s, i)
        if m and (i == 0 or not (s[i - 1].isalnum() or s[i - 1] == '_')):
            j = m.end()
            while j < len(s) and s[j] == ' ':
                j += 1
            if j < len(s) and s[j] == '(':
                depth = 0
                k = j
                while k < len(s):
                    if s[k] == '(':
                        depth += 1
                    elif s[k] == ')':
                        depth -= 1
                        if depth == 0:
                            break
                    k += 1
                i = k + 1
                out.append(' ')
                continue
            i = m.end()
            out.append(' ')
            continue
        out.append(s[i])
        i += 1
    return ''.join(out)


def _split_top(s, sep=','):
    parts, depth, cur = [], 0, ''
    for ch in s:
        if ch in '(<[{':
            depth += 1
        elif ch in ')>]}':
            depth -= 1
        if ch == sep and depth == 0:
            parts.append(cur)
            cur = ''
        else:
            cur += ch
    if cur.strip():
        parts.append(cur)
    return parts


_QUALIFIERS = {'const', 'volatile', 'unsigned', 'signed', 'long', 'short', 'struct', 'class', 'enum'}
_TYPE_WORDS = {'const', 'volatile', 'unsigned', 'signed', 'long', 'short', 'int', 'char', 'bool', 'float', 'double', 'void', 'auto', 'int8', 'int16', 'int32', 'int64', 'uint8', 'uint16', 'uint32', 'uint64', 'TCHAR', 'ANSICHAR', 'WIDECHAR', 'SIZE_T', 'typename'}


def _norm_param(p):
    p = p.strip()
    # drop default argument
    depth = 0
    for k, ch in enumerate(p):
        if ch in '(<[':
            depth += 1
        elif ch in ')>]':
            depth -= 1
        elif ch == '=' and depth == 0:
            p = p[:k].strip()
            break
    p = re.sub(r'\b(class|struct|enum|typename)\s+', '', p)
    p = re.sub(r'\s+', ' ', p).strip()
    # drop a trailing parameter name (after a type, with or without a space before it)
    m = re.match(r'^(.*?)([A-Za-z_]\w*)(\s*\[[^\]]*\])?$', p)
    if m and m.group(1).strip() and m.group(2) not in _TYPE_WORDS:
        before = m.group(1).rstrip()
        last = before[-1]
        prev_word = re.search(r'([A-Za-z_]\w*)$', before)
        if last in '*&>' or (m.group(1)[-1] == ' ' and not (prev_word and prev_word.group(1) in _QUALIFIERS)):
            p = before
    if p in ('void', ''):
        return ''
    return re.sub(r'\s*([*&<>,])\s*', r'\1', p)


def collect_virtual_macros(stripped):
    """Object-like function macros (`NAME()`) whose body declares virtuals, e.g. HIDE_ACTOR_TRANSFORM_FUNCTIONS."""
    macros = {}
    for m in re.finditer(r'^[ \t]*#\s*define\s+([A-Za-z_]\w*)\(\)\s*(.*)$', stripped, re.M):
        if re.search(r'\b(virtual|override)\b', m.group(2)):
            macros[m.group(1)] = m.group(2)
    return macros


def parse_class_body(text, cname, macros=None):
    """Return (base_names, body_text, header_line_of_decl) for the class definition named cname."""
    m = re.search(r'\b(?:class|struct)\s+(?:[A-Z][A-Z0-9_]*_API\s+)?' + re.escape(cname) +
                  r'\b\s*(?:final\s*)?(:\s*[^{;]*?)?\{', text)
    if not m:
        return None, None, None
    bases = []
    if m.group(1):
        for part in _split_top(m.group(1)[1:]):
            mm = re.search(r'([A-Za-z_]\w*)\s*$', part.strip())
            if mm:
                bases.append(mm.group(1))
    start = m.end() - 1
    depth, i = 0, start
    while i < len(text):
        if text[i] == '{':
            depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0:
                break
        i += 1
    body = text[start + 1:i]
    for name, mbody in (macros or {}).items():
        # expand the invocation in the class body (the macro's declarations belong to this class)
        body = re.sub(r'\b%s\s*\(\s*\)\s*;?' % re.escape(name), lambda _m: ' ' + mbody + ' ', body)
    line = text.count('\n', 0, m.start()) + 1
    return bases, body, line


def split_statements(body):
    stmts, cur = [], ''
    depth = 0
    pdepth = 0
    for ch in body:
        if ch == '{':
            depth += 1
        elif ch == '}':
            depth -= 1
            cur += ch
            if depth == 0 and pdepth == 0:
                # a function body or a nested definition ends here
                if re.search(r'\)\s*((const|volatile|override|final|noexcept)\s*)*$', cur.split('{')[0].rstrip()) or \
                   re.match(r'\s*(template\s*<[^>]*>\s*)?(class|struct|union|enum)\b', cur):
                    stmts.append(cur)
                    cur = ''
                continue
            continue
        if ch == '(':
            pdepth += 1
        elif ch == ')':
            pdepth -= 1
        if ch == ';' and depth == 0 and pdepth == 0:
            stmts.append(cur)
            cur = ''
            continue
        cur += ch
    return stmts


LABEL = re.compile(r'^\s*(public|protected|private)\s*:\s*')


def parse_virtuals(body):
    """Yield dicts for each own virtual declaration of a class body, in source order."""
    res = []
    for raw in split_statements(body):
        s = raw
        while True:
            m = LABEL.match(s)
            if not m:
                break
            s = s[m.end():]
        s = s.strip()
        if not s:
            continue
        if re.match(r'(template\s*<[^>]*>\s*)?(class|struct|union|enum|namespace|typedef|using)\b', s):
            continue
        if not re.search(r'\b(virtual|override|final)\b', s):
            continue
        inline_body = None
        bi = s.find('{')
        if bi >= 0:
            # inline body: text between the outermost braces
            inline_body = s[bi + 1:s.rfind('}')].strip()
            head = s[:bi]
        else:
            head = s
        head = _strip_attrs(head)
        head = re.sub(r'\bvirtual\b', ' ', head)
        head = re.sub(r'\s+', ' ', head).strip()
        pi = head.find('(')
        if pi < 0:
            continue
        before = head[:pi].strip()
        nm = re.search(r'(operator\s*\S+|~?\s*[A-Za-z_]\w*)\s*$', before)
        if not nm:
            continue
        name = re.sub(r'\s+', '', nm.group(1))
        # parameter list: balanced parentheses from pi
        depth, k = 0, pi
        while k < len(head):
            if head[k] == '(':
                depth += 1
            elif head[k] == ')':
                depth -= 1
                if depth == 0:
                    break
            k += 1
        params_txt = head[pi + 1:k]
        suffix = head[k + 1:]
        is_const = bool(re.search(r'\bconst\b', suffix))
        params = [p for p in (_norm_param(x) for x in _split_top(params_txt)) if p]
        is_pure = bool(re.search(r'=\s*0\s*$', s))
        ret = before[:nm.start()].strip() if nm else ''
        if name.startswith('~'):
            key = 'dtor'
        else:
            key = re.sub(r'\s+', '', '%s(%s)%s' % (name, ','.join(params), 'const' if is_const else ''))
        cat = None
        if inline_body is not None:
            b = re.sub(r'\s+', ' ', inline_body).strip()
            if b == '':
                cat = 'empty'
            elif re.fullmatch(r'return\s+true\s*;?', b):
                cat = 'true'
            elif re.fullmatch(r'return\s+false\s*;?', b):
                cat = 'false'
            else:
                cat = 'body'
        res.append({'key': key, 'name': name, 'ret': ret, 'params': params, 'const': is_const,
                    'pure': is_pure, 'inline': cat, 'sig': re.sub(r'\s+', ' ', head).strip()})
    return res


def model_chain_from_headers(bases_needed=None):
    """Parse each engine header; returns {class: {'base': str|None, 'virtuals': [...], 'header': rel, 'line': n}}."""
    out = {}
    stripped_all = {}
    macros = {}
    for cname, rel in HEADERS.items():
        path = os.path.join(ENGINE_SRC, rel)
        with open(path, encoding='utf-8', errors='replace') as fh:
            stripped_all[rel] = strip_comments_and_strings(fh.read())
        macros.update(collect_virtual_macros(stripped_all[rel]))
    for cname, rel in HEADERS.items():
        text = preprocess(stripped_all[rel])
        bases, body, line = parse_class_body(text, cname, macros)
        if body is None:
            out[cname] = {'base': None, 'virtuals': [], 'header': rel, 'line': None, 'error': 'class not found'}
            continue
        out[cname] = {'base': bases[0] if bases else None, 'bases': bases,
                      'virtuals': parse_virtuals(body), 'header': rel, 'line': line}
    return out


def build_engine_tables(model):
    """MSVC-style primary tables: entries are dicts with owner, index, key, name."""
    tables = {}

    def table_of(c):
        if c in tables:
            return tables[c]
        info = model.get(c)
        if info is None or info.get('error'):
            return None
        base = info.get('base')
        tab = []
        if base and base in model and not model[base].get('error'):
            tab = [dict(e) for e in table_of(base)]
        if tab and not any(v['key'] == 'dtor' for v in info['virtuals']):
            # no user-declared destructor: the implicit one still overrides the base's virtual destructor
            info = dict(info, virtuals=[{'key': 'dtor', 'name': '~' + c, 'ret': '', 'params': [], 'const': False,
                                         'pure': False, 'inline': None, 'sig': '~%s()' % c}] + info['virtuals'])
        for v in info['virtuals']:
            key = v['key']
            idx = None
            for j, e in enumerate(tab):
                if e['key'] == key:
                    idx = j
                    break
            ent = {'key': key, 'name': v['name'], 'owner': c, 'ret': v['ret'], 'params': v['params'],
                   'const': v['const'], 'pure': v['pure'], 'inline': None if key == 'dtor' else v['inline'], 'sig': v['sig'],
                   'header': info['header'], 'line': None}
            if idx is None:
                tab.append(ent)
            else:
                tab[idx] = ent
        for j, e in enumerate(tab):
            e['index'] = j
        tables[c] = tab
        return tab

    for c in model:
        table_of(c)
    return tables


# ---------------------------------------------------------------- dump image

class Image:
    def __init__(self, path):
        self.f = open(path, 'rb')
        self.m = mmap.mmap(self.f.fileno(), 0, access=mmap.ACCESS_READ)
        e = struct.unpack_from('<I', self.m, 0x3c)[0]
        n = struct.unpack_from('<H', self.m, e + 6)[0]
        osz = struct.unpack_from('<H', self.m, e + 20)[0]
        so = e + 24 + osz
        self.secs = []
        for i in range(n):
            name = self.m[so + i * 40: so + i * 40 + 8].rstrip(b'\0').decode()
            vs, va, rs, ro = struct.unpack_from('<IIII', self.m, so + i * 40 + 8)
            ch = struct.unpack_from('<I', self.m, so + i * 40 + 36)[0]
            self.secs.append((name, va, vs, ch))
        pd = [s for s in self.secs if s[0] == '.pdata'][0]
        raw = self.m[pd[1]:pd[1] + pd[2]]
        ents = set()
        for b, en, _ in struct.iter_unpack('<III', raw[:len(raw) // 12 * 12]):
            if b and en > b:
                ents.add((b, en))
        self.pdata = sorted(ents)
        self.pstarts = [a for a, _ in self.pdata]

    def sec(self, va):
        r = va - BASE
        for name, sva, vs, ch in self.secs:
            if sva <= r < sva + vs:
                return name, ch
        return None, 0

    def u64(self, va):
        return struct.unpack_from('<Q', self.m, va - BASE)[0]

    def func_of(self, va):
        r = va - BASE
        i = bisect.bisect_right(self.pstarts, r) - 1
        if i >= 0 and self.pdata[i][0] <= r < self.pdata[i][1]:
            return self.pdata[i][0] + BASE, self.pdata[i][1] + BASE
        return None

    def disasm(self, lo, hi, scratch):
        data = self.m[lo - BASE: hi - BASE]
        fd, p = tempfile.mkstemp(dir=scratch, suffix='.bin')
        with os.fdopen(fd, 'wb') as fh:
            fh.write(data)
        try:
            out = subprocess.run(['objdump', '-D', '-b', 'binary', '-mi386:x86-64',
                                  '--adjust-vma=0x%x' % lo, p],
                                 capture_output=True, text=True, timeout=120).stdout
        finally:
            os.remove(p)
        res = []
        for line in out.splitlines():
            mm = re.match(r'^\s*([0-9a-f]+):\t([0-9a-f ]+)\t(.*)$', line)
            if mm:
                res.append((int(mm.group(1), 16), mm.group(3).strip()))
        return res


def ctor_of(img, gpsc, scratch):
    """Constructor reached through InternalConstructor (the thunk `if (*this) Ctor(*this, this)`)."""
    fn = img.func_of(gpsc)
    ins = img.disasm(*fn, scratch=scratch) if fn else img.disasm(gpsc, gpsc + 0x200, scratch=scratch)
    for a, t in ins:
        mm = re.match(r"^lea\s+-?0x[0-9a-f]+\(%rip\),%\w+\s+# 0x([0-9a-f]+)$", t)
        if not mm:
            continue
        c = int(mm.group(1), 16)
        cins = img.disasm(c, c + 0x20, scratch=scratch)
        txt = [x[1] for x in cins[:6]]
        if any(re.match(r'^mov\s+\(%rcx\),%r\w+$', x) for x in txt) and any(x.startswith('test') for x in txt):
            LAST_THUNK[0] = c
            for _, t2 in cins:
                m2 = re.match(r'^(jmp|call)\s+.*?0x([0-9a-f]+)\s*$', t2)
                if m2:
                    return int(m2.group(2), 16)
    return None


LAST_THUNK = [None]


def thunk_store_candidates(img, thunk, scratch):
    """Vtable stores inside an InternalConstructor thunk (some thunks store the vptr after the constructor call)."""
    out = []
    regs = {}
    for a, t in img.disasm(thunk, thunk + 0x60, scratch=scratch):
        mm = re.match(r"^lea\s+-?0x[0-9a-f]+\(%rip\),%(\w+)\s+# 0x([0-9a-f]+)$", t)
        mv = re.match(r"^mov\s+%(\w+),\(%\w+\)$", t)
        if mm:
            regs[mm.group(1)] = int(mm.group(2), 16)
        elif mv and mv.group(1) in regs:
            tgt = regs[mv.group(1)]
            if img.sec(tgt)[0] == '.rdata' and img.sec(img.u64(tgt))[0] == '.text':
                out.append(tgt)
        if t.startswith('ret') or t.startswith('jmp'):
            break
    return out


CALLER_SAVED = ('rax', 'rcx', 'rdx', 'r8', 'r9', 'r10', 'r11')


def primary_vptr(img, ctor, scratch):
    """The last `lea reg,[vtable]; mov [base],reg` (no displacement) in the constructor whose target is a vtable."""
    fn = img.func_of(ctor)
    lo, hi = fn if fn else (ctor, ctor + 0x2000)
    ins = img.disasm(lo, hi, scratch=scratch)
    regs, stores = {}, []
    for a, t in ins:
        mm = re.match(r"^lea\s+-?0x[0-9a-f]+\(%rip\),%(\w+)\s+# 0x([0-9a-f]+)$", t)
        mv = re.match(r"^mov\s+%(\w+),\(%(\w+)\)$", t)
        if mm:
            regs[mm.group(1)] = int(mm.group(2), 16)
            continue
        if mv and mv.group(1) in regs:
            tgt = regs[mv.group(1)]
            if img.sec(tgt)[0] == '.rdata' and img.sec(img.u64(tgt))[0] == '.text':
                stores.append(tgt)
        if t.startswith('ret') and fn is None:
            break
        if t.startswith('call'):
            for r in CALLER_SAVED:
                regs.pop(r, None)
            continue
        dm = re.search(r',%(\w+)$', t)
        if dm:
            regs.pop(dm.group(1), None)
    return stores[-1] if stores else None


def vptr_candidates(img, ctor, scratch, depth=0):
    """Every `lea reg,[vtable]; mov [base],reg` (no displacement) in the constructor, in order.
    A constructor that only tail-jumps to the real one is followed (up to two hops)."""
    fn = img.func_of(ctor)
    lo, hi = fn if fn else (ctor, ctor + 0x2000)
    ins = img.disasm(lo, hi, scratch=scratch)
    tail = None
    for a, t in ins:
        mj = re.match(r'^jmp\s+0x([0-9a-f]+)\s*$', t)
        if mj and img.sec(int(mj.group(1), 16))[0] == '.text':
            tail = int(mj.group(1), 16)
    regs, stores = {}, []
    for a, t in ins:
        mm = re.match(r"^lea\s+-?0x[0-9a-f]+\(%rip\),%(\w+)\s+# 0x([0-9a-f]+)$", t)
        mv = re.match(r"^mov\s+%(\w+),\(%(\w+)\)$", t)
        if mm:
            regs[mm.group(1)] = int(mm.group(2), 16)
            continue
        if mv and mv.group(1) in regs:
            tgt = regs[mv.group(1)]
            if img.sec(tgt)[0] == '.rdata' and img.sec(img.u64(tgt))[0] == '.text':
                stores.append(tgt)
        if t.startswith('call'):
            for r in CALLER_SAVED:
                regs.pop(r, None)
            continue
        dm = re.search(r',%(\w+)$', t)
        if dm:
            regs.pop(dm.group(1), None)
    if not stores and tail is not None and depth < 2:
        return vptr_candidates(img, tail, scratch, depth + 1)
    return stores


def table_entries(img, va, limit=4096):
    """Code pointers from va until the first entry that is not code (the table end)."""
    out = []
    for i in range(limit):
        t = img.u64(va + 8 * i)
        if img.sec(t)[0] != '.text':
            break
        out.append(t)
    return out



def load_gpsc(index_path):
    """GetPrivateStaticClass address per class (native_class_index.json, gpsc_targets with one entry)."""
    with open(index_path) as fh:
        idx = json.load(fh)
    out = {}
    for name, e in idx.items():
        g = e.get('gpsc_targets') or []
        if len(g) == 1:
            out[name] = int(g[0], 16)
    return out


def load_parent_chain(parent_map_path, cls):
    """Parent chain (most derived first) from class_parent_map.json."""
    with open(parent_map_path) as fh:
        pm = json.load(fh)
    for v in pm.values():
        if v.get('name') == cls:
            return v.get('chain', [cls])
    return [cls]



def gather_dump(argv=None):
    """Constructor vtables and dump tables for the engine and game classes."""
    scratch = os.path.join(REPO, 'porting', 'generated', 'vtable_slots_scratch')
    os.makedirs(scratch, exist_ok=True)
    model = model_chain_from_headers()
    tables = build_engine_tables(model)
    img = Image(BIN)
    gpsc = load_gpsc(GPSC_INDEX)
    chains = {c: load_parent_chain(PARENT_MAP, c) for c in GAME_CLASSES}
    want = list(dict.fromkeys(ENGINE_CLASSES + GAME_CLASSES))
    ctor_info, vt, cand_map, thunk_map = {}, {}, {}, {}
    for c in want:
        g = gpsc.get(c)
        if g is None:
            ctor_info[c] = {'status': 'no GetPrivateStaticClass in native_class_index.json'}
            continue
        LAST_THUNK[0] = None
        ctor = ctor_of(img, g, scratch)
        if ctor is None:
            ctor_info[c] = {'gpsc': '0x%x' % g, 'status': 'InternalConstructor thunk not found'}
            continue
        cands = vptr_candidates(img, ctor, scratch)
        cand_map[c] = cands
        thunk_map[c] = LAST_THUNK[0]
        ctor_info[c] = {'gpsc': '0x%x' % g, 'ctor': '0x%x' % ctor, 'candidates': ['0x%x' % x for x in cands]}
    # choose the primary vtable: the candidate that agrees most with the nearest ancestor's table (parents first)
    order = sorted(cand_map, key=lambda c: len((chains.get(c) or [c])))
    for c in order:
        cands = cand_map[c]
        if not cands:
            continue
        anc = None
        for x in (chains.get(c) or [c])[1:] + [model.get(c, {}).get('base')]:
            if x and x in vt:
                anc = x
                break
        # a constructor store of an ancestor's vtable is not this class's own table
        own_ancestor_addrs = {vt[a] for a in vt if a != c and a in ((chains.get(c) or [c])[1:] + [model.get(c, {}).get('base')])}
        cands = [x for x in cands if x not in own_ancestor_addrs]
        if not cands and thunk_map.get(c):
            # the constructor only reached an ancestor's store: the thunk stores this class's own vtable
            cands = [x for x in thunk_store_candidates(img, thunk_map[c], scratch) if x not in own_ancestor_addrs]
        cands = cands or cand_map[c]
        if anc is None or len(cands) == 1:
            v = cands[-1]
            how = 'last store' if anc is None else 'single store'
        else:
            ref = table_entries(img, vt[anc], limit=2000)
            best, bestscore = None, -1
            for x in cands:
                tab = table_entries(img, x, limit=2000)
                score = sum(1 for i in range(min(len(ref), len(tab))) if ref[i] == tab[i])
                if score > bestscore or (score == bestscore):
                    best, bestscore = x, score
            v = best
            how = 'most agreement with %s (%d of %d)' % (anc, bestscore, len(ref))
        vt[c] = v
        ctor_info[c]['vtable'] = '0x%x' % v
        ctor_info[c]['choice'] = how
    # table scan stops at the next known table start (adjacent vtables must not extend a table)
    starts = sorted(set(vt.values()))
    dump = {}
    for c, v in vt.items():
        nxt = [a for a in starts if a > v]
        limit = ((nxt[0] - v) // 8) if nxt else 4096
        dump[c] = table_entries(img, v, limit=min(limit, 4096))
    return dict(model=model, tables=tables, img=img, scratch=scratch, ctor_info=ctor_info, vt=vt,
                dump=dump, chains=chains)


def main(argv=None):
    return gather_dump(argv)


# ---------------------------------------------------------------- targets

ENGINE_CLASSES = ['UObject', 'AActor', 'APawn', 'ACharacter', 'AController', 'APlayerController', 'AInfo',
                  'AGameStateBase', 'AGameState', 'APlayerState', 'AGameModeBase', 'AGameMode',
                  'UActorComponent', 'USceneComponent', 'UPrimitiveComponent', 'UNavMovementComponent',
                  'UMovementComponent', 'UPawnMovementComponent', 'UCharacterMovementComponent',
                  'APlayerCameraManager', 'UAudioComponent']
GAME_CLASSES = ['AFortPlayerStateAthena', 'AFortGameStateAthena', 'AFortPlayerControllerAthena',
                'AFortPlayerPawn', 'AFortPawn', 'AFortPlayerPawnAthena', 'AFortGameModeAthena']
# Blocker offsets named in porting/decompilation_notes.md (Twelfth pass) and BLOCKING_CALLEES.md row 15.
BLOCKERS = [
    ('0x142414c60..0x1419e89a0 (ServerClearSkydiveLeader)', 'AFortPlayerControllerAthena', ['0x2190', '0x700', '0x590']),
    ('AFortPlayerPawn::HandleInStasis 0x142a7cc50', 'AFortPlayerPawn', ['0x710', '0x728', '0x590', '0x5b8', '0x420', '0x50']),
    ('callee queue 0x1419ae980 (location colour)', 'AFortGameStateAthena', ['0xaf0']),
    ('callee queue (GetWorld on the object)', 'UObject', ['0x150']),
    ('row 15 0x1440c99f0 (float computation)', 'AFortPawn', ['0x468', '0x498', '0x4c8', '0x860']),
]
ANCHORS = [
    # (class, offset, expected name, evidence)
    ('UObject', '0x210', 'ProcessEvent', 'Object.h line 1164 (UObject::ProcessEvent(UFunction*,void*)); 66*8 = 0x210. '
     'Multicast dispatcher 0x1406a9dc0 calls +0x210 with two arguments (decompilation_notes.md, Tenth pass).'),
    ('UObject', '0x150', 'GetWorld', 'Object.h line 557 (UObject::GetWorld() const); 42*8 = 0x150. 0x1419d3ff0 calls +0x150 '
     'and then reads UWorld::GameState at +0x130 (Eighth pass).'),
    ('AFortPlayerStateAthena', '0x770', 'GetPlayerTeam', 'Dump target 0x1424192a0 is mov rax,[rcx+0x640]; ret '
     '(checked from the dump bytes); PlayerTeam is the reflected Object property at 0x640 of AFortPlayerState '
     '(reflection JSON). Slot 238 is game-declared (above the engine part of APlayerState).'),
]


def load_dump_tables(ctx):
    return ctx['dump']


def game_engine_ancestor(ctx, cls):
    """First engine class in the game chain (class_parent_map.json), and that class's model table."""
    chain = ctx['chains'].get(cls) or [cls]
    for x in chain:
        if x in ENGINE_CLASSES:
            return x
    return None


def validate_all(ctx):
    tables, dump, vt, model = ctx['tables'], ctx['dump'], ctx['vt'], ctx['model']
    classes = {}
    totals = collections.Counter()
    divergences = []

    # ---- engine classes: stub test (absolute alignment) and identity test (inherited slots)
    for c in ENGINE_CLASSES:
        if c not in tables or c not in vt:
            classes[c] = {'kind': 'engine', 'status': 'no vtable found in the dump', 'slots': []}
            continue
        tab, d = tables[c], dump[c]
        rec = {'kind': 'engine', 'status': 'checked', 'vtable': '0x%x' % vt[c], 'header': model[c]['header'],
               'bases': model[c].get('bases', []), 'model_slots': len(tab), 'dump_slots_bounded': len(d),
               'stub': Counter2(), 'identity': Counter2(), 'override': Counter2(), 'slots': []}
        for i, e in enumerate(tab):
            check, ok = 'none', None
            if e['inline'] in ('empty', 'true', 'false'):
                check, ok = 'stub', (i < len(d) and d[i] == STUB_ADDR[e['inline']])
                rec['stub'].add(ok)
                rec.setdefault('own_stub', {}).setdefault(e['owner'], Counter2()).add(ok)
            elif e['owner'] != c:
                o = e['owner']
                if o in vt and i < len(dump[o]) and i < len(d):
                    check, ok = 'identity', d[i] == dump[o][i]
                    rec['identity'].add(ok)
            else:
                base = model[c].get('base')
                if base in vt and i < len(dump[base]) and i < len(d) and i < len(tables[base]):
                    check, ok = 'override', d[i] != dump[base][i]
                    rec['override'].add(ok)
            totals[(check, ok)] += 1
            rec['slots'].append({'entry': e, 'check': check, 'ok': ok})
        classes[c] = rec

    # ---- game classes: engine part only (their table starts with the nearest engine ancestor's table)
    for g in GAME_CLASSES:
        e_cls = game_engine_ancestor(ctx, g)
        if e_cls is None or e_cls not in tables:
            classes[g] = {'kind': 'game', 'status': 'no engine ancestor'}
            continue
        if g not in vt:
            classes[g] = {'kind': 'game', 'status': 'no vtable (constructor store not found)', 'engine_ancestor': e_cls}
            continue
        tab, d = tables[e_cls], dump[g]
        rec = {'kind': 'game', 'status': 'engine part only', 'vtable': '0x%x' % vt[g], 'engine_ancestor': e_cls,
               'game_part_from': len(tab), 'identity': Counter2(), 'slots': []}
        for i, e in enumerate(tab):
            check, ok = 'none', None
            if e['inline'] in ('empty', 'true', 'false') and i < len(d):
                check, ok = 'stub', d[i] == STUB_ADDR[e['inline']]
                rec.setdefault('stub', Counter2()).add(ok)
            elif e['owner'] != g and e['owner'] in vt and i < len(dump[e['owner']]) and i < len(d):
                check, ok = 'identity', d[i] == dump[e['owner']][i]
                rec['identity'].add(ok)
            totals[('game_' + check, ok)] += 1
            rec['slots'].append({'entry': e, 'check': check, 'ok': ok})
        classes[g] = rec
    return classes, totals


class Counter2(collections.Counter):
    """pass / fail counter."""
    def add(self, ok):
        self['pass' if ok else 'fail'] += 1

    def rate(self):
        n = self['pass'] + self['fail']
        return (self['pass'] / n) if n else None


def region_alignment(rec, owner):
    """Alignment of the slots that `owner` declares: at least 2 of its inline stubs agree with the dump and at most
    one in ten fails (a fork divergence). Inherited slots are confirmed by the table that declares them."""
    st = rec.get('own_stub', {}).get(owner)
    return bool(st) and st['pass'] >= 2 and st['fail'] * 10 <= st['pass']


def class_alignment(rec):
    """Whole-table view (reporting only): >= 90% of at least 5 inline stubs agree."""
    st = rec.get('stub') or Counter2()
    n = st['pass'] + st['fail']
    return n >= 5 and st['pass'] / n >= 0.9


def slot_record(c, rec, item, classes):
    """validated: the dump entry at this index is the header's function (stub body, or the same entry as the owner's
    own table). The index is trusted when the owner table's inline stubs confirm its alignment (class_alignment)."""
    e, check, ok = item['entry'], item['check'], item['ok']
    i = e['index']
    own = classes.get(e['owner']) or rec
    aligned = region_alignment(own, e['owner'])
    if check == 'stub' and ok:
        validated, validation, conf = True, 'stub: the dump slot is the folded body the header declares inline', 'HIGH'
    elif check == 'identity' and ok:
        validated, validation = True, 'identity: inherited slot equals the owner table entry at this index'
        conf = 'HIGH' if aligned else 'MEDIUM'
    elif check == 'override' and ok:
        validated, validation = False, 'override: differs from the parent entry (index not pinned)'
        conf = 'MEDIUM' if aligned else 'LOW'
    elif check in ('stub', 'identity', 'override'):
        validated, validation, conf = False, 'dump disagrees with the header model (%s)' % check, 'LOW'
    else:
        validated, validation = False, 'no dump check (new virtual with a non-trivial body)'
        conf = 'MEDIUM' if aligned else 'LOW'
    return {
        'slot': i, 'offset': '0x%x' % (i * 8), 'name': e['name'], 'owner': e['owner'],
        'signature': e['sig'], 'header': e['header'], 'inline_body': e['inline'],
        'validation': validation, 'validated': validated, 'confidence': conf,
        'dump_check': None if ok is None else bool(ok),
    }


def build_outputs(ctx, classes, totals):
    out_classes = {}
    divergences = []
    for c, rec in classes.items():
        if rec.get('status') not in ('checked', 'engine part only'):
            out_classes[c] = {k: v for k, v in rec.items() if k != 'slots'}
            continue
        slots = [slot_record(c, rec, it, classes) for it in rec['slots']]
        for sr, it in zip(slots, rec['slots']):
            if it['ok'] is False:
                divergences.append({'class': c, 'slot': sr['slot'], 'offset': sr['offset'], 'name': sr['name'],
                                    'owner': sr['owner'], 'check': it['check']})
        entry = {k: v for k, v in rec.items() if k not in ('slots', 'stub', 'identity', 'override')}
        for k in ('stub', 'identity', 'override'):
            if k in rec:
                entry[k] = {'pass': rec[k]['pass'], 'fail': rec[k]['fail']}
        entry['alignment_confirmed'] = region_alignment(rec, c) if rec['kind'] == 'engine' else None
        entry['own_stub'] = {k: {'pass': v['pass'], 'fail': v['fail']} for k, v in rec.get('own_stub', {}).items()}
        entry['slots'] = slots
        out_classes[c] = entry
    # validation summary
    tot = {('%s' % (k[0],), k[1]): v for k, v in totals.items()}
    summary = {}
    for kind in ('stub', 'identity', 'override', 'game_stub', 'game_identity'):
        p = sum(v for (k, ok), v in totals.items() if k == kind and ok is True)
        f = sum(v for (k, ok), v in totals.items() if k == kind and ok is False)
        if p + f:
            summary[kind] = {'pass': p, 'fail': f, 'hit_rate': round(p / (p + f), 4)}
    p_all = sum(v['pass'] for v in summary.values())
    f_all = sum(v['fail'] for v in summary.values())
    summary['all'] = {'pass': p_all, 'fail': f_all, 'hit_rate': round(p_all / (p_all + f_all), 4) if p_all + f_all else None}
    return out_classes, summary, divergences


def reflection_offset(cls, prop, ref_path=None):
    """Byte offset of a reflected property (fortnite_game_reflection.json), or None."""
    path = ref_path or os.path.join(REPO, 'porting', 'generated', 'reflection', 'fortnite_game_reflection.json')
    if not os.path.exists(path):
        return None
    with open(path) as fh:
        doc = json.load(fh)
    with open(GPSC_INDEX) as fh:
        ix = json.load(fh)
    blk = (ix.get(cls) or {}).get('block_va')
    for r in doc.get('classes_in_groups_with_fort_names', []):
        if r.get('block_va') == blk:
            for p_ in r.get('properties', []):
                if p_.get('name') == prop:
                    return p_.get('offset')
    return None


def body_bytes(img, va, n, scratch):
    ins = img.disasm(va, va + n, scratch=scratch)
    return [t for _, t in ins]


def anchor_results(ctx, out_classes):
    """Anchor checks. Each row says what was checked and whether it passed."""
    res = []
    dump, tables, img, scratch = ctx['dump'], ctx['tables'], ctx['img'], ctx['scratch']
    for cls, off, name, ev in ANCHORS:
        slot = int(off, 16) // 8
        row = {'class': cls, 'offset': off, 'slot': slot, 'expected_name': name, 'evidence': ev, 'checks': {}}
        if name == 'GetPlayerTeam':
            tgt = dump[cls][slot] if cls in dump and slot < len(dump[cls]) else None
            row['dump_target'] = ('0x%x' % tgt) if tgt else None
            body = body_bytes(img, tgt, 8, scratch) if tgt else []
            row['checks']['body'] = any(t.startswith('mov    0x640(%rcx),%rax') for t in body[:1]) and \
                any(t.startswith('ret') for t in body[:3])
            pt = reflection_offset('AFortPlayerState', 'PlayerTeam')
            row['checks']['reflection_PlayerTeam_offset_0x640'] = (pt == 0x640)
            row['validated'] = bool(row['checks']['body'] and row['checks']['reflection_PlayerTeam_offset_0x640'])
            row['model_name'] = None
            row['name_matches'] = row['validated']
        else:
            ent = out_classes.get(cls, {}).get('slots', [])
            ent = ent[slot] if slot < len(ent) else None
            row['model_name'] = ent['name'] if ent else None
            row['name_matches'] = bool(ent and ent['name'] == name)
            # the same entry in the Athena controller, the actor table and this table (overrides are fine)
            tgt = dump[cls][slot] if cls in dump and slot < len(dump[cls]) else None
            row['checks']['owner_table_entry'] = ('0x%x' % tgt) if tgt else None
            act = dump.get('AActor', [])
            row['checks']['AActor_entry'] = ('0x%x' % act[slot]) if slot < len(act) else None
            apc = dump.get('AFortPlayerControllerAthena', [])
            row['checks']['AFortPlayerControllerAthena_entry'] = ('0x%x' % apc[slot]) if slot < len(apc) else None
            row['checks']['AFortPlayerControllerAthena_equals_AActor'] = bool(slot < len(act) and slot < len(apc) and act[slot] == apc[slot])
            row['validated'] = bool(row['name_matches'] and row['checks']['AFortPlayerControllerAthena_equals_AActor'])
        res.append(row)
    return res


def apply_anchors(out_classes, anchors):
    """Anchored slots take the anchor name and are validated (the anchor checks are listed in the JSON)."""
    for a in anchors:
        rec = out_classes.get(a['class'])
        if not rec:
            continue
        rec.setdefault('anchors', []).append({'slot': a['slot'], 'offset': a['offset'], 'name': a['expected_name'],
                                              'validated': a['validated'], 'evidence': a['evidence'],
                                              'checks': a['checks']})
        if 'slots' in rec and a['slot'] < len(rec['slots']) and a['validated'] and rec['slots'][a['slot']]['name'] == a['expected_name']:
            sl = rec['slots'][a['slot']]
            sl['validated'] = True
            sl['validation'] = 'anchor: ' + a['evidence'][:160]
            sl['confidence'] = 'HIGH'


def blocker_results(ctx, out_classes):
    rows = []
    for site, cls, offs in BLOCKERS:
        for off in offs:
            slot = int(off, 16) // 8
            cands = {}
            for c in [cls, 'AFortPlayerControllerAthena', 'AFortPawn', 'AFortPlayerPawn', 'AActor', 'UPrimitiveComponent', 'AFortGameStateAthena', 'AFortPlayerStateAthena']:
                rec = out_classes.get(c)
                if not rec or 'slots' not in rec:
                    continue
                if slot < len(rec['slots']):
                    s = rec['slots'][slot]
                    cands[c] = {'name': s['name'], 'owner': s['owner'], 'confidence': s['confidence'],
                                'validated': s['validated']}
                elif rec.get('game_part_from') is not None:
                    cands[c] = {'name': None, 'note': 'game-introduced index (>= %d, engine part of %s)' % (rec['game_part_from'], rec.get('engine_ancestor'))}
                else:
                    cands[c] = {'name': None, 'note': 'beyond the engine part'}
            rows.append({'site': site, 'class': cls, 'offset': off, 'slot': slot, 'candidates': cands})
    return rows


def write_outputs(ctx, classes, totals):
    out_classes, summary, divergences = build_outputs(ctx, classes, totals)
    anchors = anchor_results(ctx, out_classes)
    apply_anchors(out_classes, anchors)
    blockers = blocker_results(ctx, out_classes)
    doc = {
        'schema': 1,
        'generator': 'porting/behavior/build_vtable_slots.py (python3 -I porting/behavior/build_vtable_slots.py)',
        'image': 'porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin (file offset = VA - 0x140000000)',
        'engine_headers': ENGINE_SRC + ' (UE 4.25 declarations; Shipping macro set in MACROS)',
        'macros': MACROS,
        'validation_summary': summary,
        'divergences': divergences,
        'anchors': anchors,
        'blockers': blockers,
        'constructor_info': ctx['ctor_info'],
        'classes': out_classes,
    }
    doc['vtables'] = build_all_vtables(out_classes)
    write_json(doc)
    return doc


def write_json(doc):
    """vtable_slots.json: the named-class part as before (indent 1), then the all-class part, one class per line."""
    vt = doc['vtables']
    base = json.dumps({k: v for k, v in doc.items() if k != 'vtables'}, indent=1)
    lines = ['  "vtables": {', '   "summary": %s,' % json.dumps(vt['summary'], separators=(',', ':')),
             '   "labels": %s,' % json.dumps(vt['labels'], separators=(',', ':')),
             '   "status": %s,' % json.dumps(vt['status'], separators=(',', ':')),
             '   "wrapper_columns": ["native", "class", "function", "kind", "slot_offset", "target", "owner", "decompile"],',
             '   "wrappers": [']
    lines.append(',\n'.join('    ' + json.dumps(w, separators=(',', ':')) for w in vt['wrappers']))
    lines += ['   ],', '   "classes": {']
    lines.append(',\n'.join('    %s: %s' % (json.dumps(n), json.dumps(r, separators=(',', ':')))
                            for n, r in vt['classes'].items()))
    lines += ['   }', '  }']
    with open(OUT_JSON, 'w') as fh:
        fh.write(base[:-1].rstrip() + ',\n' + '\n'.join(lines) + '\n}\n')


class _Ctx(dict):
    pass


def main_build():
    ctx = _Ctx(main(sys.argv))
    classes, totals = validate_all(ctx)
    doc = write_outputs(ctx, classes, totals)
    return doc




def write_markdown(doc):
    S = doc['validation_summary']
    cls = doc['classes']
    lines = []
    w = lines.append
    w('# Vtable slot name map (engine and game classes of the blocked bodies)')
    w('')
    w('Generated by `porting/behavior/build_vtable_slots.py` (`python3 -I porting/behavior/build_vtable_slots.py`, about 5 s).')
    w('Data: `porting/behavior/vtable_slots.json`. Lookup: `python3 -I porting/behavior/vtable_lookup.py <class> <hexoffset>`')
    w('or `--class <class> --slot <hexoffset>` (target, label, owner, cached decompile).')
    w('Inputs are read-only: the UE 4.25 engine headers, the runtime dump image (`porting/generated/dump/`), `native_class_index.json`')
    w('(GetPrivateStaticClass addresses) and `class_parent_map.json` (parent chains). Nothing under `porting/native/` is touched.')
    w('')
    w('## Method')
    w('')
    w('1. **Model (header order, MSVC rule).** Each engine header is parsed with the Shipping macro set (`MACROS`: WITH_EDITOR 0,')
    w('   WITH_EDITORONLY_DATA 0, UE_BUILD_SHIPPING 1, WITH_ENGINE 1, ...). A class table starts as a copy of its primary base table;')
    w('   a declaration whose signature matches an inherited entry replaces it (override keeps the slot), any other one is appended.')
    w('   A virtual destructor is one slot (the scalar deleting destructor), and every class with a virtual base destructor has its own')
    w('   slot 0. Macro-declared virtuals (`HIDE_ACTOR_TRANSFORM_FUNCTIONS()` in Actor.h) are expanded in the classes that invoke them.')
    w('   Secondary bases (`INavAgentInterface`, `IInterface_AssetUserData`) are not in the primary table.')
    w('2. **Dump tables.** `GetPrivateStaticClass` (native_class_index) -> `InternalConstructor` thunk (`if (*this) Ctor(*this, this)`)')
    w('   -> constructor. The primary vptr is the `lea reg,[vtable]; mov [base],reg` store with no displacement. Where several stores')
    w('   qualify (inlined base constructors), the one that agrees most with the ancestor table is chosen. The table scan stops at the')
    w('   next known table start.')
    w('3. **Validation (per slot, against the dump).**')
    w('   - *stub*: an inline empty / `return true` / `return false` body must be the folded stub the linker kept at that slot')
    w('     (`0x1406801c0`, `0x1406801d0`, `0x1406801e0`). This pins the absolute index.')
    w('   - *identity*: an inherited slot must hold the same entry as its owner class table at that index.')
    w('   - *override*: a slot the class declares must differ from its parent entry (weak).')
    w('   - *anchors*: `UObject` 66 (ProcessEvent), 42 (GetWorld), and `AFortPlayerStateAthena` 238 (GetPlayerTeam, body check).')
    w('4. **Confidence.** `validated` = the dump check passed (stub or identity). HIGH = validated, and the owner table is aligned')
    w('   (at least 90% of at least 5 of its inline stubs match). MEDIUM = not validated, but the owner table is aligned. LOW = the')
    w('   owner table is not aligned, or the dump disagrees with the model.')
    w('5. **Game classes.** Their table starts with the nearest engine ancestor table (indices below the engine length are exact).')
    w('   Game-declared virtuals sit above that length (`game_part_from`) and have no names here: the game headers are not')
    w('   available, and the generated headers declare almost no virtuals.')
    w('')
    w('## Validation numbers')
    w('')
    w('| Check | pass | fail | hit rate |')
    w('|---|---:|---:|---:|')
    for k, label in [('stub', 'engine stub (absolute index)'), ('identity', 'engine identity (inherited)'),
                     ('override', 'engine override (weak)'), ('game_stub', 'game engine-part stub'),
                     ('game_identity', 'game engine-part identity'), ('all', 'all checks')]:
        if k in S:
            v = S[k]
            w('| %s | %d | %d | %s |' % (label, v['pass'], v['fail'], ('%.1f%%' % (100 * v['hit_rate'])) if v['hit_rate'] is not None else '-'))
    w('')
    eng = [c for c in cls if cls[c].get('kind') == 'engine' and 'slots' in cls[c]]
    gam = [c for c in cls if cls[c].get('kind') == 'game' and 'slots' in cls[c]]
    n_eng_slots = sum(len(cls[c]['slots']) for c in eng)
    n_high = sum(1 for c in eng + gam for x in cls[c]['slots'] if x['confidence'] == 'HIGH')
    n_val = sum(1 for c in eng + gam for x in cls[c]['slots'] if x['validated'])
    w('Named slots: %d engine classes (%d slots) and %d game classes (engine part only). Validated slots: %d (HIGH: %d).'
      % (len(eng), n_eng_slots, len(gam), n_val, n_high))
    w('')
    w('| Class | vtable | slots | stub pass/fail | identity pass/fail | aligned |')
    w('|---|---|---:|---:|---:|---|')
    for c in eng + gam:
        r = cls[c]
        st = r.get('stub') or {'pass': 0, 'fail': 0}
        idn = r.get('identity') or {'pass': 0, 'fail': 0}
        w('| %s | %s | %d | %d/%d | %d/%d | %s |' % (c, r.get('vtable', ''), len(r['slots']), st['pass'], st['fail'],
                                                  idn['pass'], idn['fail'],
                                                  ('yes' if r.get('alignment_confirmed') else ('via engine ancestor %s' % r.get('engine_ancestor') if r.get('kind') == 'game' else 'no'))))
    w('')
    w('## Divergences (the dump disagrees with the 4.25 header model)')
    w('')
    w('Engine classes (the header model is wrong for the shipped build, or a UHT-generated override is not in the headers):')
    w('')
    eng_div = [d for d in doc['divergences'] if cls.get(d['class'], {}).get('kind') == 'engine']
    grp = collections.Counter((d['owner'], d['name'], d['check']) for d in eng_div)
    for (o, n, ch), k in sorted(grp.items(), key=lambda kv: -kv[1])[:40]:
        w('- %s::%s (owner %s, %s check): %d class table(s)' % (o, n, o, ch, k))
    w('')
    w('Game classes: a divergence there is an override the game makes (the engine table is the model). They are counted in the game rows only.')
    w('')
    w('## Anchors')
    w('')
    for a in doc['anchors']:
        w('- `%s` slot %d (`%s`) %s: validated %s. %s' % (a['class'], a['slot'], a['offset'], a['expected_name'],
                                                    'yes' if a.get('validated') else 'no', a['evidence']))
        for k, v in a.get('checks', {}).items():
            w('  - %s: %s' % (k, v))
    w('')
    w('## Blocker offsets (what the map resolves)')
    w('')
    w('A candidate is the name the map gives at that index for that receiver class. It is not a resolution: the receiver of')
    w('each call site must be established first (the queue notes give the receivers found from reflection). Several candidates')
    w('are engine virtuals with a return type that does not fit the blocked use (for example a 12-byte result is copied from the')
    w('slot 0x700 of the skydive body, while the model name of that index in APlayerCameraManager returns void).')
    w('')
    for r in doc['blockers']:
        cands = '; '.join('%s: %s' % (c, (v.get('name') or v.get('note', '-')) + (' (%s)' % v['confidence'] if v.get('confidence') else '')) for c, v in list(r['candidates'].items())[:4])
        w('- %s, `%s` (slot %d), receiver %s: %s' % (r['site'], r['offset'], r['slot'], r['class'], cands))
    w('')
    V = doc.get('vtables')
    if V:
        S2 = V['summary']
        w('## All constructed classes (`vtables` in the JSON)')
        w('')
        w('Every class in `native_class_index.json` whose GetPrivateStaticClass leads to a constructor is scanned the same way as')
        w('above (no headers, no names: the slots are target addresses). Slot targets are read from the late-capture image')
        w('(`%s`), checked against the dump image (mismatches: %d). A table is the primary vptr of the class; its `own` map holds only' % (S2['slot_image'], S2['slot_table_mismatch_vs_dump']))
        w('the slots that differ from the nearest ancestor with a table (the rest are inherited). Labels come from exec addresses in')
        w('`native_funcs.json`, ported addresses in `MANIFEST.json` and constructors.')
        w('')
        for k in ('classes_with_constructor', 'classes_with_primary_vtable', 'classes_without_primary_vtable', 'table_slots',
                  'own_entries', 'secondary_tables', 'labelled_targets', 'agree_native_class_index', 'agree_rich_classes',
                  'rich_classes_compared', 'natives_total', 'natives_unported', 'wrapper_single_dispatch',
                  'wrapper_short_dispatch_only', 'wrapper_with_resolved_target'):
            w('- %s: %s' % (k, S2[k]))
        w('')
    w('## Limits')
    w('')
    w('- The name is the 4.25 declaration. The shipped engine and the Fortnite fork diverge (APlayerController 203..232, UHT-generated')
    w('  `ValidateGeneratedRepEnums`, `PostDuplicate(bool)`); those slots are reported as divergences, not as names.')
    w('- Game virtuals (index >= `game_part_from`) are not named. Only RPC thunk call sites (`decompilation_notes.md`, Eighth/Tenth pass) name')
    w('  some of them, from inference, not from this map.')
    w('- AFortPlayerPawnAthena has no constructor vptr found (its constructor is a short tail jump that the scan does not follow).')
    w('- Own virtuals of a class are confirmed only by stubs that class declares (`region_alignment`). APlayerCameraManager')
    w('  declares none, so its own slots are LOW: the dump body at slot 224 returns a 12-byte value, the model says void,')
    w('  and slot 219 (GetCameraRotation in the model) is a bool-returning body. The shipped table differs from the 4.25 model here.')
    w('- Some constructors store the vptr inside the InternalConstructor thunk after the call (UPawnMovementComponent); those are')
    w('  read from the thunk when the constructor only reaches an ancestor store.')
    w('- Secondary vtables (displaced stores in the constructor, `secondary` in the all-class part) are candidates only: they are')
    w('  not validated and a displaced store into another object would be recorded too.')
    w('- The all-class part has no names beyond the labels; an unlabelled slot has only its target address.')
    w('')
    with open(OUT_MD, 'w') as fh:
        fh.write('\n'.join(lines) + '\n')


# ---------------------------------------------------------------- every constructed class ('vtables' in the JSON)
# Constructor vptrs for every class in native_class_index.json (not only the named ones above), its primary and
# secondary tables, slot targets read from the late-capture image, labels for known function addresses, and the
# unported vtable-dispatch wrapper natives with the target they resolve to.

EVIDENCE_IMAGE = '/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin'
NATIVE_FUNCS = os.path.join(REPO, 'porting', 'generated', 'reflection', 'native_funcs.json')
MANIFEST_JSON = os.path.join(REPO, 'porting', 'behavior', 'MANIFEST.json')
GENERATED = os.path.join(REPO, 'porting', 'generated')
MAX_TABLE = 4096
WORKERS = 16
SHORT_LINES = 15
_LEA_RIP = re.compile(r"^lea\s+-?0x[0-9a-f]+\(%rip\),%(\w+)\s+# 0x([0-9a-f]+)$")
_THUNK_LEA = re.compile(r"^lea\s+-?0x[0-9a-f]+\(%rip\),%\w+\s+# 0x([0-9a-f]+)$")
_STORE_NODISP = re.compile(r'^mov\s+%(\w+),\(%\w+\)$')
_STORE_DISP = re.compile(r'^mov\s+%(\w+),(-?0x[0-9a-f]+)\(%\w+\)$')
_DISPATCH = re.compile(r'\(\*+\(code \**\)\(\*(\w+) \+ 0x([0-9a-fA-F]+)\)\)\s*\(')
_CODE_CAST = re.compile(r'\(code \**\)')
_CALL = re.compile(r'(?<![\w\)\]])([A-Za-z_]\w*)\s*\(')
_KEYWORDS = {'if', 'while', 'for', 'switch', 'return', 'sizeof'}
_DECL = re.compile(r'^(?:\w+\s+)+\**\s*\w+(?:\[\d+\])?$')


def ctor_and_thunk(img, gpsc, scratch):
    """(thunk, constructor) reached from GetPrivateStaticClass, as ctor_of does."""
    fn = img.func_of(gpsc)
    ins = img.disasm(*fn, scratch=scratch) if fn else img.disasm(gpsc, gpsc + 0x200, scratch=scratch)
    for a, t in ins:
        mm = _THUNK_LEA.match(t)
        if not mm:
            continue
        c = int(mm.group(1), 16)
        cins = img.disasm(c, c + 0x20, scratch=scratch)
        txt = [x[1] for x in cins[:6]]
        if any(re.match(r'^mov\s+\(%rcx\),%r\w+$', x) for x in txt) and any(x.startswith('test') for x in txt):
            for _, t2 in cins:
                m2 = re.match(r'^(jmp|call)\s+.*?0x([0-9a-f]+)\s*$', t2)
                if m2:
                    return c, int(m2.group(2), 16)
    return None, None


def ctor_stores(img, ctor, scratch, depth=0):
    """(displacement, vtable) for every vtable store in a constructor (a tail-jumped constructor is followed)."""
    fn = img.func_of(ctor)
    lo, hi = fn if fn else (ctor, ctor + 0x2000)
    ins = img.disasm(lo, hi, scratch=scratch)
    tail = None
    for a, t in ins:
        mj = re.match(r'^jmp\s+0x([0-9a-f]+)\s*$', t)
        if mj and img.sec(int(mj.group(1), 16))[0] == '.text':
            tail = int(mj.group(1), 16)
    regs, out = {}, []
    for a, t in ins:
        mm = _LEA_RIP.match(t)
        if mm:
            regs[mm.group(1)] = int(mm.group(2), 16)
            continue
        disp, src = None, None
        m0 = _STORE_NODISP.match(t)
        m1 = _STORE_DISP.match(t)
        if m0:
            disp, src = 0, m0.group(1)
        elif m1:
            disp, src = int(m1.group(2), 16), m1.group(1)
        if src in regs:
            tgt = regs[src]
            if img.sec(tgt)[0] == '.rdata' and img.sec(img.u64(tgt))[0] == '.text':
                out.append((disp, tgt))
        if t.startswith('call'):
            for r in CALLER_SAVED:
                regs.pop(r, None)
            continue
        dm = re.search(r',%(\w+)$', t)
        if dm:
            regs.pop(dm.group(1), None)
    if not out and tail is not None and depth < 2:
        return ctor_stores(img, tail, scratch, depth + 1)
    return out


def class_ctor_record(img, gpsc, scratch):
    thunk, ctor = ctor_and_thunk(img, gpsc, scratch)
    if ctor is None:
        return {'status': 'InternalConstructor thunk not found'}
    stores = ctor_stores(img, ctor, scratch)
    rec = {'ctor': ctor, 'stores': stores}
    if thunk:
        rec['thunk_stores'] = thunk_store_candidates(img, thunk, scratch)
    return rec


def read_table(img, va, starts):
    """Code pointers from va up to the next known table start (or MAX_TABLE)."""
    nxt = bisect.bisect_right(starts, va)
    limit = MAX_TABLE
    if nxt < len(starts):
        limit = min(limit, (starts[nxt] - va) // 8)
    return table_entries(img, va, limit=limit)


def resolve_slot(recs, cls, idx):
    """(target, owner) of slot idx of cls: the nearest class (cls first) whose own delta holds the slot."""
    c, hops = cls, 0
    while c in recs and hops < 64:
        r = recs[c]
        if idx >= r['slots']:
            return None, None
        t = r['own'].get(str(idx))
        if t:
            return t, c
        c, hops = r['parent'], hops + 1
    return None, None


def analyze_wrapper(text):
    """('single'|'short', slot offset) for a body whose only call is a dispatch through this (param_1) vtable."""
    i, j = text.find('{'), text.rfind('}')
    if i < 0 or j <= i:
        return None
    body = re.sub(r'/\*.*?\*/|//[^\n]*', '', text[i + 1:j], flags=re.S)
    if len(_CODE_CAST.findall(body)) != 1:
        return None
    disp = _DISPATCH.findall(body)
    if len(disp) != 1 or disp[0][0] != 'param_1':
        return None
    if [m for m in _CALL.findall(body) if m not in _KEYWORDS]:
        return None
    off = int(disp[0][1], 16)
    if off % 8:
        return None
    stmts = [s.strip() for s in body.split(';')]
    stmts = [s for s in stmts if s and s != 'return' and not _DECL.match(s)]
    kind = 'single' if len(stmts) == 1 and _DISPATCH.search(stmts[0]) else None
    if kind is None and sum(1 for l in body.splitlines() if l.strip()) > SHORT_LINES:
        return None
    return (kind or 'short'), off


def decompile_index(wanted):
    """Cached 0x<addr>.c files under porting/generated/*; decomp dirs are searched first."""
    found = {}
    dirs = sorted(os.listdir(GENERATED), key=lambda d: (d not in ('decomp', 'decomp_improved'), d))
    for d in dirs:
        p = os.path.join(GENERATED, d)
        if not os.path.isdir(p):
            continue
        for n in os.listdir(p):
            if n.startswith('0x') and n.endswith('.c'):
                try:
                    a = int(n[2:-2], 16)
                except ValueError:
                    continue
                if a in wanted and a not in found:
                    found[a] = os.path.relpath(os.path.join(p, n), REPO)
    return found


def build_all_vtables(rich_classes, limit=None):
    scratch = os.path.join(REPO, 'porting', 'generated', 'vtable_slots_scratch')
    os.makedirs(scratch, exist_ok=True)
    img = Image(BIN)
    slot_img, slot_src = (Image(EVIDENCE_IMAGE), EVIDENCE_IMAGE) if os.path.exists(EVIDENCE_IMAGE) else (img, BIN)
    gpsc = load_gpsc(GPSC_INDEX)
    items = sorted(gpsc.items())[:limit]
    with open(PARENT_MAP) as fh:
        pm = json.load(fh)
    chain = {}
    for v in pm.values():
        if v.get('name'):
            chain[v['name']] = v.get('chain') or [v['name']]
    with open(GPSC_INDEX) as fh:
        gindex = json.load(fh)

    def work(item):
        name, g = item
        try:
            return name, class_ctor_record(img, g, scratch)
        except Exception as exc:  # one odd constructor must not stop the pass
            return name, {'status': 'error: %s' % exc}

    with concurrent.futures.ThreadPoolExecutor(WORKERS) as ex:
        raw = dict(ex.map(work, items))

    # primary vptr: as in gather_dump, parents first; secondary = displaced stores of another table
    vt, ctor_of_cls, sec, status = {}, {}, {}, {}
    order = sorted(raw, key=lambda c: (len(chain.get(c, [c])), c))
    for c in order:
        r = raw[c]
        if 'stores' not in r:
            status[c] = r.get('status', 'no vtable store')
            continue
        ch = chain.get(c, [c])
        anc = [vt[a] for a in ch[1:] if a in vt]
        ancset = set(anc)
        prim = [t for d, t in r['stores'] if d == 0]
        thunk_t = r.get('thunk_stores', [])
        cands = ([t for t in prim if t not in ancset] or [t for t in thunk_t if t not in ancset]
                 or prim or thunk_t)
        if not cands:
            status[c] = 'no primary store'
            continue
        ref = table_entries(img, anc[0], limit=2000) if anc else None
        if len(cands) == 1 or ref is None:
            best = cands[-1]
        else:
            best, bs = None, -1
            for x in cands:
                tb = table_entries(img, x, limit=2000)
                s = sum(1 for i in range(min(len(ref), len(tb))) if ref[i] == tb[i])
                if s >= bs:
                    best, bs = x, s
        vt[c] = best
        ctor_of_cls[c] = r['ctor']
        m = {}
        for d, t in r['stores']:
            if d and 0 < d < 0x1000 and t != best:
                m[d] = t
        sec[c] = m

    starts = sorted(set(vt.values()) | {t for m in sec.values() for t in m.values()})
    tcache, mismatch = {}, 0
    for va in starts:
        tcache[va] = read_table(slot_img, va, starts)
        if read_table(img, va, starts) != tcache[va]:
            mismatch += 1
    par = {}
    for c in order:
        if c in vt:
            par[c] = next((a for a in chain.get(c, [c])[1:] if a in vt), None)
    recs = {}
    for c in order:
        if c not in vt:
            continue
        tb = tcache[vt[c]]
        pt = tcache[vt[par[c]]] if par[c] else []
        own = {str(i): '0x%x' % t for i, t in enumerate(tb) if i >= len(pt) or pt[i] != t}
        rec = {'vtable': '0x%x' % vt[c], 'slots': len(tb), 'parent': par[c], 'ctor': '0x%x' % ctor_of_cls[c], 'own': own}
        if sec[c]:
            rec['secondary'] = {'0x%x' % d: {'vtable': '0x%x' % t, 'slots': len(tcache[t])}
                                for d, t in sorted(sec[c].items())}
        recs[c] = rec

    # labels: exec addresses from native_funcs, ported addresses from MANIFEST, constructors
    with open(NATIVE_FUNCS) as fh:
        nf = json.load(fh)
    natives = {}
    for cls, fns in nf.items():
        for fn, e in fns.items():
            if e.get('exec_addr'):
                natives.setdefault(int(e['exec_addr'], 16), (cls, fn))
    with open(MANIFEST_JSON) as fh:
        manifest = json.load(fh)
    ported = {}
    for e in manifest:
        if e.get('status') == 'ported' and e.get('address'):
            try:
                ported.setdefault(int(e['address'], 16), (e.get('class'), e.get('function')))
            except (TypeError, ValueError):
                pass
    ctors = {int(r['ctor'], 16): c for c, r in recs.items()}
    referenced = set()
    for rec in recs.values():
        referenced.update(int(t, 16) for t in rec['own'].values())
    # a label needs one name in its source: folded or shared addresses (the linker's stubs, aliases) are not named
    exec_names, ported_names = collections.defaultdict(set), collections.defaultdict(set)
    for cls, fns in nf.items():
        for fn, e in fns.items():
            if e.get('exec_addr'):
                exec_names[int(e['exec_addr'], 16)].add('%s::%s' % (cls, fn))
    for e in manifest:
        if e.get('status') == 'ported' and e.get('address') and e.get('class') and e.get('function'):
            try:
                ported_names[int(e['address'], 16)].add('%s::%s' % (e['class'], e['function']))
            except (TypeError, ValueError):
                pass
    ctor_names = {k: {'%s::ctor' % v} for k, v in ctors.items()}
    # the declaring class of a label must be a class whose own table introduces that entry
    owners = collections.defaultdict(set)
    for c, rec in recs.items():
        for t in rec['own'].values():
            owners[int(t, 16)].add(c)
    labels = {}
    for a in referenced:
        if a in STUB_ADDR.values():
            continue
        for names, src in ((exec_names, 'exec'), (ported_names, 'ported'), (ctor_names, 'ctor')):
            if len(names.get(a, ())) == 1:
                name = next(iter(names[a]))
                if name.split('::', 1)[0] in owners[a]:
                    labels[a] = [name, src]
                break

    # unported natives that are vtable-dispatch wrappers, resolved in the primary table of their class
    unported = [(a, cls, fn) for a, (cls, fn) in sorted(natives.items()) if a not in ported]
    decomp = decompile_index({a for a, _, _ in unported})
    wrappers, counts = [], collections.Counter()
    for a, cls, fn in unported:
        path = decomp.get(a)
        if not path:
            counts['unported_without_decompile'] += 1
            continue
        with open(os.path.join(REPO, path), encoding='utf-8', errors='replace') as fh:
            res = analyze_wrapper(fh.read())
        if res is None:
            continue
        kind, off = res
        counts['wrapper_' + kind] += 1
        target, owner = resolve_slot(recs, cls, off // 8)
        if target:
            counts['wrapper_resolved'] += 1
        wrappers.append(['0x%x' % a, cls, fn, kind, '0x%x' % off, target, owner, path])

    own_entries = sum(len(r['own']) for r in recs.values())
    cls_rich = {name: rec.get('vtable') for name, rec in rich_classes.items()}
    summary = {
        'natives_total': len(natives),
        'natives_ported': sum(1 for a in natives if a in ported),
        'natives_unported': len(unported),
        'unported_without_decompile': counts['unported_without_decompile'],
        'wrapper_single_dispatch': counts['wrapper_single'],
        'wrapper_short_dispatch_only': counts['wrapper_short'],
        'wrapper_with_resolved_target': counts['wrapper_resolved'],
        'classes_with_constructor': sum(1 for r in raw.values() if 'ctor' in r),
        'classes_with_primary_vtable': len(recs),
        'classes_without_primary_vtable': len(status),
        'secondary_tables': sum(len(r.get('secondary', {})) for r in recs.values()),
        'table_slots': sum(r['slots'] for r in recs.values()),
        'own_entries': own_entries,
        'labelled_targets': len(labels),
        'slot_image': slot_src,
        'slot_table_mismatch_vs_dump': mismatch,
        'agree_native_class_index': sum(1 for c in recs if gindex.get(c, {}).get('vtable_addr') == recs[c]['vtable']),
        'agree_rich_classes': sum(1 for c in recs if c in cls_rich and cls_rich[c] == recs[c]['vtable']),
        'rich_classes_compared': sum(1 for c in recs if c in cls_rich and cls_rich[c]),
    }
    return {
        'summary': summary,
        'labels': {'0x%x' % a: v for a, v in sorted(labels.items())},
        'classes': recs,
        'status': status,
        'wrappers': wrappers,
    }


if __name__ == '__main__':
    doc = main_build()
    write_markdown(doc)
    print(json.dumps(doc['validation_summary'], indent=1))
    print('divergences:', len(doc['divergences']))
    print('classes:', {k: (len(v.get('slots', [])) if 'slots' in v else v.get('status')) for k, v in doc['classes'].items()})
