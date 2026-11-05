# Quest parser singleton and grammar evidence

`0x140331ee0` initializes singleton `0x14815f750` using constructor `0x1423faf50` and registers `0x14587d000` for teardown; that callback invokes destructor `0x142434020`. This is process-static initialization, not a lazily allocated instance. Captures are under ignored `porting/generated/quest-grammar-probe{,2}`. The constructor is 7,322 native bytes and also constructs evaluator tables; it is not fully translated.

The constructor installs 21 consumers in this exact order at singleton +0x50 (addresses are callback / node-type GUID getter):

| Order | Matched text | Callback | Type getter |
| --- | --- | --- | --- |
| 1 | `(` | `0x1423f74a0` | `0x142419df0` |
| 2 | `)` | `0x1423f7390` | `0x142419d70` |
| 3 | `+` | `0x1423f7060` | `0x142419b70` |
| 4 | `-` | `0x1423f6ae0` | `0x142419870` |
| 5 | `*` | `0x1423f7280` | `0x142419cf0` |
| 6 | `/` | `0x1423f6610` | `0x1424194f0` |
| 7 | `%` | `0x1423f6f50` | `0x142419af0` |
| 8 | `^` | `0x1423f7170` | `0x142419bf0` |
| 9–14 | `<=`, `<`, `>=`, `>`, `!=`, `==` | `0x1423f6a30`, `0x1423f6980`, `0x1423f67d0`, `0x1423f6720`, `0x1423f6da0`, `0x1423f6510` | `0x1424197f0`, `0x142419770`, `0x1424195f0`, `0x142419570`, `0x1424199f0`, `0x142419470` |
| 15–19 | `||` or `or`; `&&` or `and`; `!` or `not`; `~` or `HasTag`; `MissingTag` | `0x1423f6e50`, `0x1423f6410`, `0x1423f6ca0`, `0x1423f6880`, `0x1423f6bf0` | `0x142419a70`, `0x1424193f0`, `0x142419970`, `0x142419670`, `0x1424198f0` |
| 20–21 | custom consumers | `0x14240db90`, `0x14240d980` | both `0x142419c70` |

The two custom consumers construct the same node type; `0x14240db90` delegates to `0x142c59330`, while `0x14240d980` uses the character predicate `0x14240a540`, constructs a string and converts it to `FName`. Their complete accepted syntax is not yet established. Symbol callback captures are in `porting/generated/quest-token-callbacks/`; matched strings were read directly from the pinned local image and callbacks. Parenthesis and arithmetic callbacks also try a second string determined by `0x1423f8930` when the primary match fails; that behavior is not yet identified. Do not infer keyword boundaries or case sensitivity from this list.

The constructor populates the grammar at +0x68: grouping pairs the `(` and `)` GUIDs, and the pre-unary set contains `+`, `-`, and `!`. The binary-operator map registrations carry these raw precedence numbers: `*`, `/`, `^`: 3; `+`, `-`, `~`/`HasTag`, `MissingTag`: 4; `<`, `<=`, `>`, `>=`, `!=`, `==`: 5; `||`/`or` and `&&`/`and`: 6. Each shown registration stores zero for the remaining operator parameter bytes. This resembles UE 4.25 `FOpParameters` right-to-left/no-short-circuit defaults, but exact packed argument layout should be checked before asserting associativity or short-circuit behavior. `%` is lexed, yet no corresponding `0x142419af0` binary grammar insertion appears in this constructor; preserve that observation rather than assuming modulo is accepted.

Remaining blockers include the two custom token consumers, alternate strings and case/boundary behavior, exact grammar registration layouts, operator evaluation implementations, short-circuit rules, and singleton lifetime/ownership. A grammar-only port would not yet recover the parser. No native body has been marked complete, and no build or runtime validation was performed. Ghidra probes report project metadata changes despite rolled-back probe transactions.
