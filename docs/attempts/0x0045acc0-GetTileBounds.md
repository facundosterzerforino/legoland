# GetTileBounds (0x0045acc0, `src/legoland/tilemap.c`)

**Best: 75.63%**. Kind: C.

## What still differs

- Register assignment: the original loads `size` into `edx` and keeps `ref` in `ecx`.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r2 | Haiku agent | Inlined the sprite lookup | no change or worse |
| 2026-10-07 r2 | Haiku agent | `int` vs `short` for the size | no change or worse |
| 2026-10-07 r2 | Haiku agent | A `dia` local | no change or worse |
| 2026-10-07 r2 | Haiku agent | Store into a `left` local, then `out[0]` | 14.43% (much worse) |
| 2026-10-08 | permuter + Opus 5.5 | new permuter (12 min): `volatile int top`, size assigned as a statement after the declarations | 28.30 -> 75.63 |
| 2026-10-08 | permuter + Opus 5.5 | by hand, before the permuter: out[2]/out[3] in the original's operand order, int size / dbl locals, out[3] first - all identical code | 28.30 (no change) x5 |

## Ideas not tried yet

- None recorded.
