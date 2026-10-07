# RenderCarousel (0x0042bcf0, `src/legoland/carousel.c`)

**Best: 41.03%**. Kind: C.

## What still differs

- Frame: ours `sub esp, 0x60`, original `0x68`. Both put `local_28` against the return address, so the original has 8 more bytes of locals below it; every stack offset in the body is off by that.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r2 | Haiku agent | Moved `ride = param_1->ride` after the zero-fill (so `param_1` lands in `edx`) | 40.21% (worse) |
| 2026-10-07 r2 | Haiku agent | Block-scoped `layerres` into the GetLayer call | no change |

## Ideas not tried yet

- Find the missing 8 bytes of locals (a second `struct Point`, or a 2-int array).
