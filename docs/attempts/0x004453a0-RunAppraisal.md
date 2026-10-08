# RunAppraisal (0x004453a0, `src/legoland/challenge.c`)

**Best: 26.50%** (rewrite, 2026-10-08; was 15.70%). Kind: C.

## What still differs

State after the rewrite (2026-10-08). With registers, stack offsets and jump distances ignored, 95.3% of the original's
8,086 instructions match (93-97% per section), and ours has the same instruction count. Record-field offsets match
everywhere. The registers follow the original: ebx = 0, esi = i, edi = y, ebp = pagestart (copied to -9172) until the
advice head, then the row offset. What is left is the stack frame, which shifts nearly every stack operand:

- Ours is 0x23e8 bytes, the original 0x23d4: 20 bytes more.
  - 16 of them are section 10's `RECT r`, the text and bar rectangles passed by value. One of its fields is spilled,
    so it gets a frame slot; the original builds both rectangles in the argument area and never gives them a home.
  - 4 of them are one more scalar slot than the original has: the slots are packed differently.
- Order. MSVC6 sorts frame slots by weight, heaviest at the bottom. Small tests show this: names and declaration order
  change nothing, an extra use moves a variable down, and equal weights come out in an unstable order.
  - Ours: row-offset temporary (i*0x4c), rc, pagestart, x, first, ...
  - Original: pagestart, x, row-offset temporary (shared with n and the render-object count), rc, ...
  - Our row-offset temporary is heavier because the loop sections' strength-reduced offset is merged into it (70
    extra uses in sections 2 and 3). In the original that offset has its own slot, -9112.
- Packing. The original shares these slots:
  - -9124: xp, and the loop sections' pass counter (a different variable from section 1's);
  - -9112: section 1's pass counter, the loop offset and the tile count;
  - -9104: the loop sections' row pointer, 19*i and the speech index;
  - -9096: passacc and the display's first row;
  - -9116: flags and the queued-speech count.

  Ours shares them differently. See the analysis file for the full slot map.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r1 | Haiku agent | Added a padding local to make up the 20 bytes | rejected: fake padding is not allowed |
| 2026-10-07 r2 | Haiku agent | Analysis only (the frame findings above) | no change |
| 2026-10-08 | Haiku 5.5 | Analysis of every `[esp+0x58..0x6c]` access in `orig`; noted that out58..out6c are already used | no change |
| 2026-10-08 | Haiku 5.5 | Moved `rep` above the six `out5x/out6x` declarations (declaration order probe) | frame still `0x23c0`, 15.39%; reverted |
| 2026-10-08 | permuter + Opus 5.5 | volatile passtotacc and tottotacc (both / each alone / plus piVar10): the totals stay in memory like the original, frame unchanged at 0x23c0 | 15.39 -> 15.60 (both); 15.35-15.42 others |
| 2026-10-08 | permuter + Opus 5.5 | each FUN_00444bf0/c70/cd0/d20/d70 output argument gets its own variable (11, the original's slots -9084..-9044, found with tools/frame.py): frame now 0x23d4 like the original | 15.60 -> 15.70 |
| 2026-10-08 | Opus 5.5 + 9 section-writer agents (workflow) | Full rewrite: a skeleton (struct AppraisalRow, the layout/cur rectangles pinned with `if (0)`, page-break macros), ten sections written against a per-section harness (scratchpad ra/h.py), each part's shared restart block written as an explicit label. Then an adversarial behaviour review (12 agents) found one real bug, fixed: section 9 used the nids pointer before section 8 set it | 15.70 -> 26.50 (structure 95.3%) |
| 2026-10-08 | Opus 5.5 | Frame probes on copies: rc declared first; separate subpass/rowp variables in the loop sections | frame order unchanged, score identical |

## Ideas not tried yet

- Give the loop sections' strength-reduced offset its own temporary, as the original does. That should bring the
  row-offset temporary's weight below pagestart's and x's.
- Remove section 10's RECT home: build the two rectangles so that no field is spilled.
- Pack like the original: n and the render-object count into the row-offset temporary's slot, and so on (see
  "Packing" above).
