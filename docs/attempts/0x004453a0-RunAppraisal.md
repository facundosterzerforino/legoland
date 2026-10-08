# RunAppraisal (0x004453a0, `src/legoland/challenge.c`)

**Best: 15.70%** (unchanged from start). Kind: C.

## What still differs

- (2026-10-08, tools/frame.py) Exact slot map. The original's frame, entry-relative:
  scalars -9172..-9092 (with -9140 never touched), coord (short) at -9086, 11 output variables -9084..-9044,
  the record array at -9040 (exactly 100 records of 19 ints, our rep[] + 9), fmtbuf at -1312, a 201-int area at the top.
  -9160/-9156/-9152/-9148 are a RECT {0x50, 0x6d, 0x1a4, 0x83} (layout: written once, read 105-208 times from memory -
  MSVC6 does not propagate constants through struct fields; our C has them as plain ints, so they fold away).
  -9144/-9136 are written 207 times and never read (dead stores MSVC6 only keeps for struct fields: a second RECT
  "cur" whose left/right are copied from layout at every page break; -9140 is its top, kept in edi; -9132 its bottom,
  compared with 0x1b5). -9172 is the page-start record index (ebp). The decompiler propagated the constants and deleted
  the overflow test of the first block and the cur copies at all 134 page-break sites.
- (2026-10-08) Prologue comparison: the original zeroes FIVE stack slots on entry ([0x10], [0x14], [0x48], [0x5c], [0x60], with ebx/ebp both zero) plus esi; ours zeroes four (xbase, tottotacc, passtotacc, flags). So the original has at least one more variable initialised to 0 at the top. The 20 missing bytes are five dword slots. A quick automatic slot census was unreliable (deferred argument pops); this needs a careful manual slot-by-slot mapping of the first ~200 instructions.
- Stack frame: original `sub esp, 0x23d4`, ours `0x23c0` (20 bytes short). `rep` starts at `[esp+0x70]` in the original and `[esp+0x68]` in ours.
- The out slots are NOT unused in the current C: `out58`, `out5c`, `out60`, `out64`, `out68`, `out6c` are all passed by address to FUN_00444bf0 / FUN_00444c70 / FUN_00444cd0 / FUN_00444d20 / FUN_00444d70 and read back (`DAT_00666028 <= out5c` etc.). MSVC keeps them; the frame is still short, so the problem is *where* they sit, not whether they exist.
- Findings about the original's slots (esp-relative, pre-push numbering):
  - `[0x10]`, `[0x14]`, `[0x18]` (xbase), `[0x1c]`..`[0x28]` (colstep/ystart/rectr/wstart) are loop state, as in ours.
  - `[0x48]`, `[0x4c]`, `[0x50]`: per-row counters, zeroed in the row loop (`mov [esp+0x50], ebx`), `[0x50]` incremented in the ReportFlags&0x80 block.
  - `[0x58]`: pointer `piVar10 = &rep[iVar*19+10]` (the `lea ecx,[esp+ecx+0x98]` gives rep base 0x70). Written as `mov [ecx],1` / `mov [eax],0` through it. Ours keeps this in a register.
  - `[0x5c]` and `[0x60]`: two running accumulators, zeroed on entry (`mov [esp+0x60], ebx; mov [esp+0x5c], ebx`). In the row loop they are shifted as a pair (`[0x60] += [0x50]`, `[0x5c] += [0x40]`, and `[0x58]`/`[0x5c]`/`[0x60]` rotate at the ends of blocks). These are the `tottotacc`/`passtotacc` pair; ours keeps them in registers.
  - `[0x66]`: a **short** at 0x66, written `mov word ptr [esp+0x66], dx` and passed as `&coord` to FUN_0044f360 (ours `short coord`). So `coord` is at 0x66, between 0x64 and 0x68.
  - `[0x68]`: read in the `ReportFlags & 0x8000` block (`cmp ecx, DAT_00666028` at 0x4457a6). This is the same value as `out5c` after the first FUN_00444bf0 call.
  - `[0x6c]`: `lea ecx,[esp+0x6c]` is the second argument to the first FUN_00444bf0 call; read back at 0x44587d and 0x4476b7.
  - Pending-push caveat: the first FUN_00444bf0 call sits after a `push 0x131` (GetString arg) that is only popped by the `add esp,0xc` after the call (MSVC defers the pop). So all esp offsets in that block are 4 lower than in straight-line code. The first call's first argument is read back as `[esp+0x8c]` after the pops. Put together, `out5c` appears at frame 0x68 and `out58` at frame 0x8c in the original. Frame 0x8c falls inside `rep` (rep[7] of record 0) if rep is at 0x70, so the mapping of out58 is still unclear.
- Buffers: ours and the original agree on `[esp+0x1ec4]` and `[esp+0x1ec8]` (fmtbuf/wavbuf). The top of the frame differs only by the 0x14 bytes of scalars below rep, and maybe the placement of the 0x1e48/0x1e50 vs 0x1e58 slots (wavbuf start).
- About 3,600 lines with many `goto LAB_...`: too big for one agent pass.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r1 | Haiku agent | Added a padding local to make up the 20 bytes | rejected: fake padding is not allowed |
| 2026-10-07 r2 | Haiku agent | Analysis only (the frame findings above) | no change |
| 2026-10-08 | Haiku 5.5 | Analysis of every `[esp+0x58..0x6c]` access in `orig`; noted that out58..out6c are already used | no change |
| 2026-10-08 | Haiku 5.5 | Moved `rep` above the six `out5x/out6x` declarations (declaration order probe) | frame still `0x23c0`, 15.39%; reverted |
| 2026-10-08 | permuter + Opus 5.5 | volatile passtotacc and tottotacc (both / each alone / plus piVar10): the totals stay in memory like the original, frame unchanged at 0x23c0 | 15.39 -> 15.60 (both); 15.35-15.42 others |
| 2026-10-08 | permuter + Opus 5.5 | each FUN_00444bf0/c70/cd0/d20/d70 output argument gets its own variable (11, the original's slots -9084..-9044, found with tools/frame.py): frame now 0x23d4 like the original | 15.60 -> 15.70 |

## Ideas not tried yet

- Pin the MSVC placement of `coord` (short at 0x66) and the `out5c = 0x68` slot: try declaring the scalars in the order that puts `coord` right below `out68`/`out6c` (declaration order alone did not change the frame; this needs a layout-level change, not a reorder).
- Remove or move the pending `push 0x131` (GetString call) out of the FUN_00444bf0 argument block so that the offsets line up with the original; then re-check the frame.
- Model the two accumulators (`[0x5c]`/`[0x60]`) as real `int` locals that are read and written in the row loop (instead of the register `tottotacc`/`passtotacc` pair). They are the most likely cause of the 2 missing scalar slots.
- Split the work: fix the frame first, then go block by block.
