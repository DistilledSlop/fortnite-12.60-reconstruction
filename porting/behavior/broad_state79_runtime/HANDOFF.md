# Player-state dependency batch79

Four uncounted drafts from native-broad79, not build inputs. Source hashes and
addresses are in sources.json; hashes identify decompiler output, not original
bytes or parity. 14241bc10 performs two lookups, a class ancestry check, then
compares bytes returned by virtual+28 on two embedded receivers. 141e5f780
chooses an output operation based on a lookup at receiver+4548. 141a2d0f0 is a
class ancestry predicate whose significant return is AL. 1423d5330 is a
forwarding wrapper, not substantial newly discovered behavior: it sets R9B to
1, forwards the incoming registers to virtual+1fc8, and returns original RDX.

Adapters are per memory access/call; native types, argument and return ABI,
class ancestry table bounds, embedded receiver layout, ownership and side
effects of all callees remain open. No extra null checks or default values were
added. Assembly was consulted for register/AL details. No build, compiler, tests,
generation, canonical promotion or staging was run. Root owns review and
integration. Previous player-state message bodies are already drafted in batches
69 and 71; this batch does not duplicate them.
