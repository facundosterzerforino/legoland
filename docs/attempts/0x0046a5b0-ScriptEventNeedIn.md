# ScriptEventNeedIn (0x0046a5b0, `src/legoland/nerps.c`)

**Best: 74.32%**

## What still differs

(not analysed yet)

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-08 | permuter + Opus 5.5 | permuter 76.4% made y unsigned (then `y >= 0` is always true) - not kept | rejected |
| 2026-10-09 | Opus 5.5 | `int y` (the original compares y signed, `jg`); + `volatile int count` / `int count[1]` | verify 74.32 (was 75.14), not kept; the GameMap/lpConfig hoisting differs like ScriptEventClearArea |

## Ideas not tried yet

- 

## Inline-helper checks

Refreshed baseline: harness 76.40, full verify 75.14. Removing the dead
count expression and making y signed scores 75.56 in the harness. A normal
static __inline tile lookup, or an inline counting helper taking &count,
produce the same 75.56; they do not restore the original loop reloads.
No variant retained, no volatile added. The original signed-y difference
remains documented; avoid treating the current unsigned comparison as proof
of the original type.
