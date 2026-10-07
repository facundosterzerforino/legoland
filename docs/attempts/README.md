# Attempt log for unmatched functions

One file per function that is not yet at 100%, named `<address>-<name>.md`. Each says what still differs
from the original, what has already been tried (with the score it gave), and ideas nobody has tried yet.

**Before working on a function, read its file and don't repeat an attempt listed there** unless you are
changing something else at the same time. **After working on it, add your attempts** to the table, even the
ones that failed: date, who (person, or model for agents), the change, the score. Update "Best" and "What
still differs" when they change. When a function reaches 100%, delete its file.

## What applies to most of them

- **Declaration order almost never changes MSVC6's register allocation.** Nearly every agent tried
  reordering locals; it changed nothing in all but one case. Don't spend cycles on it.
- **A frame-size difference (`sub esp, N`) means a local is missing or extra.** MSVC drops locals that are
  never used, so a declared-but-unused variable doesn't count. Find the real variable behind the
  original's extra slot (look at which `[esp+...]` offsets it uses). Don't add fake padding locals.
- **The original has `push ebp; mov ebp, esp` and no asm-only instructions?** Wrap the function in
  `#pragma optimize("y", off)` / `#pragma optimize("y", on)` (`decomp-tips.md`). FUN_00485fe0 went from 33% to 77%
  with that alone.
- **Late pushes of callee-saved registers** (after an early check) usually mean the rest of the function is
  inside an `if (...) { }` block in the source, not after an early `return`.
- **`repe cmpsd` comes from `memcmp`**, `rep stosd` from `memset` with a constant size (FUN_00452030).
- **MSVC already turns a tail call to the same function into a loop**; writing the loop by hand made
  SearchJunglePathConnected worse.
- **Hand-written asm:** `tools/asm2naked.py` writes `lea reg, [ebp-x]` with a 1-byte displacement; if the
  original uses the 4-byte form, the function comes out shorter and every later jump shifts. Emit those
  instructions as raw bytes with `_emit` (RenderTransSprite).
- **reccmp false positives:** a constant that falls inside a data symbol's range in only one of the two
  images shows as a diff on a byte-identical line (`decomp-tips.md`).

## Index

| Address | Function | File | Best |
|---|---|---|---|
| 0x00404630 | [CoptersPlaceRider](0x00404630-CoptersPlaceRider.md) | copters.c | 54.39% |
| 0x00408f90 | [FUN_00408f90](0x00408f90-FUN_00408f90.md) | log_flume.c | 40.96% |
| 0x0040ae90 | [FUN_0040ae90](0x0040ae90-FUN_0040ae90.md) | log_flume.c | 45.11% |
| 0x0040bab0 | [FUN_0040bab0](0x0040bab0-FUN_0040bab0.md) | log_flume.c | 44.09% |
| 0x0040c250 | [FUN_0040c250](0x0040c250-FUN_0040c250.md) | log_flume.c | 42.86% |
| 0x00411680 | [AdvanceFlumeMover](0x00411680-AdvanceFlumeMover.md) | log_flume.c | 40.52% |
| 0x0041df00 | [FUN_0041df00](0x0041df00-FUN_0041df00.md) | castle.c | 44.94% |
| 0x0041ef60 | [FUN_0041ef60](0x0041ef60-FUN_0041ef60.md) | castle.c | 43.10% |
| 0x0042bcf0 | [RenderCarousel](0x0042bcf0-RenderCarousel.md) | carousel.c | 41.03% |
| 0x0042fbb0 | [Restaurant2Update](0x0042fbb0-Restaurant2Update.md) | eatery.c | 41.89% |
| 0x00437260 | [SearchJunglePathConnected](0x00437260-SearchJunglePathConnected.md) | jungle_cruise.c | 38.41% |
| 0x0043a7a0 | [FUN_0043a7a0](0x0043a7a0-FUN_0043a7a0.md) | space_tower.c | 37.65% |
| 0x00442040 | [RemapTexCoordsToCell](0x00442040-RemapTexCoordsToCell.md) | render3d.c | 41.17% |
| 0x004431f0 | [GetLegColourOfBloke](0x004431f0-GetLegColourOfBloke.md) | render3d.c | 25.00% |
| 0x00443220 | [GetArmColourOfBloke](0x00443220-GetArmColourOfBloke.md) | render3d.c | 25.00% |
| 0x004453a0 | [RunAppraisal](0x004453a0-RunAppraisal.md) | challenge.c | 15.39% |
| 0x0044e010 | [__BMPLoader](0x0044e010-__BMPLoader.md) | gfx.c | 39.57% |
| 0x00452030 | [FUN_00452030](0x00452030-FUN_00452030.md) | controller.c | 33.15% |
| 0x00455370 | [BubbleHelp](0x00455370-BubbleHelp.md) | text.c | 24.23% |
| 0x0045acc0 | [GetTileBounds](0x0045acc0-GetTileBounds.md) | tilemap.c | 28.30% |
| 0x0045ade0 | [FUN_0045ade0](0x0045ade0-FUN_0045ade0.md) | tilemap.c | 43.41% |
| 0x0045bcd0 | [PointToIsoPlane](0x0045bcd0-PointToIsoPlane.md) | tilemap.c | 25.70% |
| 0x0045c9c0 | [FUN_0045c9c0](0x0045c9c0-FUN_0045c9c0.md) | tilemap.c | 49.38% |
| 0x0045ca90 | [FUN_0045ca90](0x0045ca90-FUN_0045ca90.md) | tilemap.c | 42.86% |
| 0x0045d5d0 | [FUN_0045d5d0](0x0045d5d0-FUN_0045d5d0.md) | tilemap.c | 42.28% |
| 0x0045d770 | [FUN_0045d770](0x0045d770-FUN_0045d770.md) | tilemap.c | 67.69% |
| 0x00461290 | [FUN_00461290](0x00461290-FUN_00461290.md) | map_object.c | 43.92% |
| 0x00465850 | [FUN_00465850](0x00465850-FUN_00465850.md) | draw.c | 34.23% |
| 0x0046f9a0 | [FUN_0046f9a0](0x0046f9a0-FUN_0046f9a0.md) | icon.c | 41.45% |
| 0x00471ca0 | [RemoveNewObject](0x00471ca0-RemoveNewObject.md) | popupinfo.c | 63.16% |
| 0x00485fe0 | [FUN_00485fe0](0x00485fe0-FUN_00485fe0.md) | print_sprite.c | 77.03% |
