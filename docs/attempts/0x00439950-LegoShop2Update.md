# LegoShop2Update (0x00439950, `src/legoland/shops.c`)

**Current: verify 75.11%, harness 78.40%.** Kind: C.

## Behaviour bug in the current C

Case 3 (pick a browse spot), `r == 2`: the original sets `dest = ((x - 3) << 8, y << 8)` (0x4399fd: `add ebp,-3;
shl ebp,8; mov [esi+0x24],ebp; shl edi,8; jmp 0x439b07`, which joins after case 1's `sub edi,0x80`). Ours
writes `(y << 8) - 0x80`. Case 5 `r == 2` (the same spot) already uses `y << 8`. The fixed C scores harness
46.70-48.86: with one less use of `y`, MSVC swaps the x/y registers (ebp/edi), and that shift spreads through every
case. The fix is not landed yet. It needs a source form that keeps x in ebp and y in edi.

## Original structure (for the next attempt)

- Header order: `next`, `tile = &node->tile`, `bloke`, `x = tile->pos.x + ride->x` (ebp), `y = ... + ride->y` (edi).
- The CalcMoveLine call setups are cross-jumped into three shared tails: B1 0x439a39 (case 0, case 3 default),
  B2 0x439b07 (case 1/9, case 3 r==2) and B3 0x439b31 (case 2, case 3 r==1, case 10). Case 11 has its own copy
  and falls into case 12.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-23 | Opus 5.5 | fix r==2, header in the original's order, 120 random orders of dest.x store vs y shift per case | 46.70-48.86 |
| 2026-10-23 | Opus 5.5 | fixed C with direct `dest = expr` stores (no x/y reassignment), int/uint, both x/y declaration orders, 226-order decl search | 46.70-48.86 |
