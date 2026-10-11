# CreateSampleFromWAV (0x00492380, `src/legoland/sound_sfx.c`)

**Best: 97.49%** (start 97.49%)

## Tried
| Date | Model | Change | Result |
|---|---|---|---|
| 2026-10-07 | claude-haiku-5-5 | `if (SoundAvailable != 0) file = RES_OpenFile(...)` instead of early return | 97.49% (no change) |
| 2026-10-07 | claude-haiku-5-5 | `if (file == NULL) return NULL;` before the do/while | 97.49% (no change) |

## What still differs
- Original keeps the `SoundAvailable == 0` early return as its own inline epilogue (`jne` over 10 bytes of
  pops and `ret`). Ours jumps to the shared bottom `return NULL` epilogue, because MSVC merges the two
  identical `return NULL` blocks.
- Success path: ours duplicates `pop ebp; pop ebx; add esp; ret` after the `RES_CloseFile` tail.
- (2026-10-11, Opus 5.5) The original's layout is a cleanup ladder: every failure path falls into
  `buffer->Release` / `free(data)` / `free(format)` / `RES_CloseFile(file)` / `return 0` (entering at
  different steps), and the success path (`sample != NULL`) is the last block of the function. That is the
  multi-label `goto` cleanup pattern; the project allows only one shared `goto fail`, so it cannot be
  reproduced as written. Revisit only if multi-label cleanup gotos become acceptable.

## Ideas not tried yet
- Make the bottom `return NULL` differ from the early one so the compiler cannot merge them.
