# RestoreBaseMap (0x0045da60, `src/legoland/tilemap.c`)

**Best: 100%** (documented naked __asm since 2026-10-19; the plain C in the `#else` branch is lower). Kind: C.

## What still differs

- The original ends with `and dx, 0x20` before `ret`: the flag test on `TileSpriteInfo[id].sprite` is kept
  even though its result is discarded. Ours drops it.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 | Haiku agent | Original `(void)(flags & 0x20);` with volatile read (current best) | 95.65% |
| 2026-10-07 | Haiku agent | `flags &= 0x20;` after the store | 95.65% (no change, `and` dropped) |
| 2026-10-07 | Haiku agent | Non-volatile read plus `if (flags & 0x20) { tile->tile = id; }` | 72.73% (worse) |
| 2026-10-09 | Opus 5.5 | identical if/else branches (also via `switch`, `?:` statement, short temp `f = sprite & 0x20`) | 76.92 (branches merge, leaves `test bl,32`: load narrowed to byte, `push ebx`) |
| 2026-10-09 | Opus 5.5 | `(sprite & 0x20) == 0x20` with identical branches | 80 (`and dl,32; cmp dl,32` survive, byte load) |
| 2026-10-09 | Opus 5.5 | volatile word load + merged if / `== 0x20` / empty if / `f = flags & 0x20` | 40-95.65 |
| 2026-10-09 | Opus 5.5 | dead local set in an if, `while (0)`, `if (f == 0) return;` | 60-80 (all eliminated) |

## Ideas not tried yet

- Find a construct MSVC6 keeps as a dead `and` on a 16-bit register (for example a `volatile` store of a
  flag-derived value to a stack local whose slot is later reused).

## 2026-10-19 Opus 5.5: 95.65% -> 100% (documented naked __asm)

The original really ends `mov word ptr [eax+8],cx; and dx,0x20; ret` (raw bytes `66 83 e2 20 c3` at 0x45da8f;
the function listing stops one instruction early because the size comes from ours). That dead `and` is the
leftover of a flag test whose branch MSVC6 removed late. None of the C forms above keep it, so the function is
now an exact naked transcription. The `#else` branch has the plain C (`tile->field_8 = tile->field_a;`) with a
comment about the discarded read.
