# CarouselUpdate (0x0042c820, `src/legoland/carousel.c`)

**Best: 85.71%**, verified 2026-10-18. Source unchanged.

## What still differs

- Original stack frame is `0x3c`; current frame is `0x38`. The boarding case has an
  extra original store of projected x after both coordinate shifts. Do not add padding.
- Boarding-case x/y registers, projection arithmetic scheduling, and the later
  screen-offset calculations differ. Most other cases match apart from stack offsets
  and branch displacements.
- The leaving case loads both screen coordinates before doubling them in the original;
  ours interleaves the first doubling/store with the second load.

## Tried

The standalone compiler harness scores the unchanged source at 86.69%; compare
variants to that baseline rather than to the full reccmp score of 85.71%.

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-18 | Codex | Reuse tile_w/tile_h or x/y for projected coordinates; change projection order while preserving the map inputs | 13 trials including baseline: 55.07-86.69%; no gain |
| 2026-10-18 | Codex | Swap map-coordinate load order; separate sums/differences/products, reuse x/y for intermediate sums, swap final view/scroll expression order | 30 trials: 56.05-86.69%; no gain |
| 2026-10-18 | Codex | Snapshot both leaving coordinates using Point, x/y, or short locals; reverse load order; separate doubling from stores or use compound updates | 7 trials: 73.61-86.69%; no gain |

## Ideas not tried yet

- Identify the original projected-x spill's lifetime and relationship to the other
  stack locals before changing the boarding projection again.
