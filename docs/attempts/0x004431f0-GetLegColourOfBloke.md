# GetLegColourOfBloke (0x004431f0, `src/legoland/render3d.c`)

**Best: 25.00%**. Kind: C.

## What still differs

- The original zeroes both accumulators (`xor ecx,ecx` and `xor edx,edx`) before dereferencing `param_1->field_4`; ours zeroes one after it. The byte stores are `mov ch,R` / `mov dl,B` / `mov cl,G`, which suggests a source form that keeps both accumulators live from the start.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r2 | Haiku agent | About eight variants, including byte-combining `|=` / `<<=` | no change |

## Ideas not tried yet

- Two accumulators initialised to 0 at declaration, before the pointer is read.
- A union or struct of bytes for the colour instead of shifts.
- Whatever matches here likely fixes GetArmColourOfBloke too.
