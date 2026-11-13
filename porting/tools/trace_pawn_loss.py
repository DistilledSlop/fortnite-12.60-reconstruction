# Attach ONCE to the matching running server, with matching debug symbols:
#   PAWN_LOSS_LOG=/absolute/writable/path/pawn-loss.log \
#     gdb -q -nx -batch -p "$SERVER_PID" \
#       -x porting/tools/trace_pawn_loss.py -ex continue
# The log is created exclusively; choose a new path for each run. GDB stays
# attached until the server exits. Stop GDB with SIGINT and detach if needed.
# Breakpoints briefly suspend the inferior; no inferior functions are called.
import datetime
import os
import traceback
import gdb

path = os.environ.get('PAWN_LOSS_LOG')
if not path or not os.path.isabs(path):
    raise RuntimeError('PAWN_LOSS_LOG must be an absolute, new writable file path')
log = open(path, 'x', buffering=1, encoding='utf-8')
gdb.execute('set pagination off')
gdb.execute('set confirm off')
gdb.execute('set breakpoint pending off')
gdb.execute('handle SIGTRAP nostop noprint nopass')


def write(message):
    log.write(message + '\n')
    log.flush()


def dynamic(value):
    if value is None or int(value) == 0:
        return None
    try:
        return value.cast(value.dynamic_type)
    except Exception:
        return value


def field(value, *names):
    for name in names:
        if value is None:
            return None
        try:
            if value.type.strip_typedefs().code == gdb.TYPE_CODE_PTR:
                if int(value) == 0:
                    return None
                value = value.dereference()
            value = value[name]
        except Exception:
            return None
    return value


def is_type(value, wanted):
    try:
        typ = dynamic(value).type.strip_typedefs()
        if typ.code == gdb.TYPE_CODE_PTR:
            typ = typ.target().strip_typedefs()
        def walk(t, seen):
            t = t.strip_typedefs()
            name = t.tag or t.name or str(t)
            if name in seen:
                return False
            seen.add(name)
            if name in wanted:
                return True
            return any(walk(f.type, seen) for f in t.fields() if f.is_base_class)
        return walk(typ, set())
    except Exception:
        return False


# UE reflection survives -fno-rtti. Compare actual UClass pointers with the
# generated static class pointers, walking UStruct::SuperStruct without calls.
# None means class membership could not be determined from available symbols.
def ue_is_type(value, name):
    try:
        if value is None or int(value) == 0:
            return None
        mangled = "_ZZN{}{}21GetPrivateStaticClassEvE18PrivateStaticClass".format(len(name), name)
        target = gdb.parse_and_eval("*(UClass**)&{}".format(mangled))
        if int(target) == 0:
            return None
        cls = field(value, 'ClassPrivate')
        seen = set()
        while cls is not None and int(cls) != 0:
            address = int(cls)
            if address == int(target):
                return True
            if address in seen or len(seen) >= 128:
                return None
            seen.add(address)
            cls = field(cls.cast(gdb.lookup_type('UStruct').pointer()), 'SuperStruct')
            if cls is None:
                return None
        return False
    except Exception:
        return None


def matches(value, name):
    if is_type(value, {name}):
        return True
    return ue_is_type(value, name)


def cast_as(value, name):
    try:
        return value.cast(gdb.lookup_type(name).pointer()) if value is not None else None
    except Exception:
        return value


def show(name, value):
    try:
        write('  {}: {}'.format(name, '<unavailable>' if value is None else value))
    except Exception as exc:
        write('  {}: <unavailable: {}>'.format(name, exc))


def arg(name):
    try:
        return gdb.parse_and_eval(name)
    except Exception:
        return None


class Trace(gdb.Breakpoint):
    def __init__(self, spec, kind):
        super().__init__(spec, internal=True)
        self.silent = True
        self.kind = kind
        self.count = 0
        self.unknown = 0
        write('installed {} locations={}'.format(spec, len(self.locations)))

    def stop(self):
        try:
            obj = dynamic(arg('this'))
            pawn = dynamic(arg('inPawn')) if self.kind == 'pending' else (
                dynamic(field(obj, 'Pawn')) if self.kind == 'unpossess' else obj)
            # AFortPlayerPawn subclasses remain identifiable after Controller is cleared.
            player_pawn = matches(pawn, 'AFortPlayerPawn')
            fort_pawn = matches(pawn, 'AFortPawn')
            if fort_pawn or player_pawn:
                pawn = cast_as(pawn, 'AFortPawn')
            controller = obj if self.kind in ('pending', 'unpossess') else dynamic(field(pawn, 'Controller'))
            player_controller = matches(controller, 'APlayerController')
            if not (player_pawn or (fort_pawn and player_controller)):
                if self.unknown < 3:
                    self.unknown += 1
                    write('{}: filtered/unclassifiable pawn={} controller={} (sample {})'.format(
                        self.kind, pawn, controller, self.unknown))
                    write('  class results: player_pawn={} fort_pawn={} player_controller={} (None=unavailable)'.format(
                        player_pawn, fort_pawn, player_controller))
                return False
            pawn = cast_as(pawn, 'AFortPawn')
            self.count += 1
            if self.count > 5:
                return False
            write('\n{} {} event={} pawn={} controller={}'.format(
                datetime.datetime.now(datetime.timezone.utc).isoformat(), self.kind,
                self.count, pawn, controller))
            root = dynamic(field(pawn, 'RootComponent'))
            movement = dynamic(field(pawn, 'CharacterMovement'))
            health = dynamic(field(pawn, 'HealthSet'))
            outer = dynamic(field(pawn, 'OuterPrivate'))
            world = dynamic(field(cast_as(outer, 'ULevel'), 'OwningWorld')) if matches(outer, 'ULevel') else None
            level = dynamic(field(world, 'PersistentLevel'))
            settings = dynamic(field(level, 'WorldSettings'))
            for label, value in (
                ('position (component-to-world translation)', field(root, 'ComponentToWorld', 'Translation')),
                ('root relative location (relative to attach parent if attached)', field(root, 'RelativeLocation')),
                ('root attach parent', field(root, 'AttachParent')),
                ('velocity', field(movement, 'Velocity')),
                ('movement mode', field(movement, 'MovementMode')),
                ('ignore client movement correction', field(movement, 'bIgnoreClientMovementErrorChecksAndCorrection')),
                ('accept client authoritative position', field(movement, 'bServerAcceptClientAuthoritativePosition')),
                ('floor', field(movement, 'CurrentFloor')),
                ('movement base', field(pawn, 'BasedMovement', 'MovementBase')),
                ('role', field(pawn, 'Role')), ('remote role', field(pawn, 'RemoteRole')),
                ('world time', field(world, 'TimeSeconds')),
                ('initial lifespan', field(pawn, 'InitialLifeSpan')),
                ('lifespan timer handle', field(pawn, 'TimerHandle_LifeSpanExpired')),
                ('creation time', field(pawn, 'CreationTime')),
                ('KillZ', field(settings, 'KillZ')), ('health attribute', field(health, 'Health')),
            ):
                show(label, value)
            try:
                write(gdb.execute('bt 20', to_string=True).rstrip())
            except Exception as exc:
                write('  backtrace unavailable: {}'.format(exc))
            if self.count == 5:
                write('{}: event limit reached'.format(self.kind))
        except Exception:
            write('trace error:\n' + traceback.format_exc())
        return False


write('pawn-loss trace started UTC={} pid={} (limit five events per hook)'.format(
    datetime.datetime.now(datetime.timezone.utc).isoformat(), gdb.selected_inferior().pid))
for spec, kind in (
    ('AController::PawnPendingDestroy(APawn*)', 'pending'),
    ('APawn::Destroyed()', 'destroyed'),
    ('AActor::FellOutOfWorld(UDamageType const&)', 'fell'),
    ('AController::UnPossess()', 'unpossess'),
):
    try:
        Trace(spec, kind)
    except Exception as exc:
        write('FAILED to install {}: {}'.format(spec, exc))
write('setup complete; continue inferior')
