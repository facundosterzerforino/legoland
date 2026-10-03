# Running agent batches on this repo

Lessons from the first multi-agent sessions. Helper scripts and prompts live in `tools/agent/`.

## One-time setup in a fresh container
```sh
export PATH=$HOME/.local/bin:$PATH
# wibo is not preinstalled: download the release binary to ~/.local/bin/wibo (chmod +x)
uv run setup.py                                  # toolchain (needs SSL_CERT_FILE=/root/.ccr/ca-bundle.crt behind the proxy)
cmake --preset msvc6 && cmake --build build
uv run reccmp-project detect --search-path external   # writes reccmp-user.yml
cp build/reccmp-build.yml . && sed -i 's#^project: .*#project: .#' reccmp-build.yml   # tools/progress.py needs this at repo root
uv run tools/progress.py                         # progress table (expect ~2941 matched / 3255 at 90.4%)
```
`reccmp-build.yml` and `reccmp-user.yml` are gitignored. `uv run` rewrites `uv.lock`: `git checkout uv.lock` before committing.

## Helpers (`tools/agent/`)
- `status.py` prints `addr tu name pct` for every game function (needs the reccmp configs above).
- `partials.py` lists pure-C partial matches (excludes inline-asm functions) to /tmp/partials.json.
- `regress.sh BASELINE` compares against a saved `status.py` output: prints `REGRESSED` / `NEWLY MATCHED`.
  Save the baseline first: `uv run tools/agent/status.py > /tmp/baseline.txt`.
  Empty output means no change; make sure the build actually succeeded first (a failed build looks the same).
- `wt-setup.sh` prepares an agent worktree. `*-prompt.md` are the agent prompts used (paths point at /tmp/names).
- `score.py ADDR...` prints the match % of single functions (a few seconds; `EFFECTIVE` marks register/order-only matches that
  status.py counts as 100%). Much faster than a full `regress.sh` while iterating.
- `sbs.py ADDR...` prints the whole function side by side (original | ours), `~`/`-`/`+` on differing lines. `verify -v` only shows hunks.
- `diffsum.py`, `difftype.py` and `deadends.py` triage a list of addresses: size of the diff, whether the instruction multiset is
  identical (pure register/scheduling difference), and diffs on layout-dependent operands reccmp cannot normalize (skip those).

## Rules that worked
- **Triage partials before touching code.** Run `difftype.py` and `deadends.py` over the candidate list and work only
  functions with a small diff that is neither register-only nor layout-dependent. That rules out most dead ends before any attempt.
- **Cap each function at about 3 attempts** (one variant batch each), then record it in the dead-end list and move on.
- **Push to main** (`git push origin HEAD:main`, plain fast-forward) after each verified stage, and keep the working branch in sync.
- **Do not run clang-format.** The container's version differs from the repo's and rewrites untouched code (it also breaks `progress.py` by wrapping signatures).
- Max ~20 concurrent subagents. Sonnet agents with isolation=worktree: tell them to `git reset --hard` to the branch tip first
  (worktrees may start from an older commit, which makes regression baselines wrong).
- **Merging an agent's commit**: `git cherry-pick`, rebuild, run `regress.sh`. If it conflicts, apply the diff by hand; do not trust the
  agent's own "regressions" report (stale baseline) and do not trust a regress run on a conflicted tree (conflict markers = fake mass regressions).
- Agents that are stopped leave uncommitted WIP in their worktree: test it as a patch and keep it only if something goes up and nothing goes down.

## Naming passes (pure renames)
Pipeline: proposer agents (read-only, evidence required, skip when unsure) -> script merges and rejects duplicate/colliding names ->
verifier agents re-derive each claim independently (ACCEPT / REJECT / RENAME) -> one scripted word-boundary replace over
`src/legoland/*.[ch]` -> clean rebuild -> all 3255 scores must be identical -> commit + push.
`docs/naming-skipped.tsv` records every symbol already considered (named / skipped-no-evidence / rejected-by-verifier /
dropped-collision): exclude them from candidate lists. Only ~35% of candidates get a name; the easy ones are taken first.

## Partial matches: known dead ends (do not retry)
Several partials cannot be fixed from C because of how reccmp compares, not because of the code:
- A constant like `0x800000` that equals a data address is shown as `EditCursor+5184`: SetObjRectFlags, FUN_0045e080, AddBasicObject, AddObjectToMap.
- Indexed table calls `call [reg*4+TABLE]`: reccmp compares raw displacements: FUN_0046b2d0, FUN_0046a140, FUN_0041ec50, FUN_0041ec70, FUN_0041eca0.
- Loop-end pointers that land on CRT/float symbols in our layout: InitFreePlayLists, FUN_0049b350.
- SEH scope-table address in the prologue (layout dependent): WinMain, WriteModuleInfo.
Ones agents worked on and left unfinished (compiler scheduling/register differences): GameMain, LoadZoomer, FUN_00457a70, FUN_0043be70,
FUN_00415ae0, FUN_00429cf0, FUN_004232b0, FUN_00430b10, FUN_0040c8d0, FUN_0040f5b0, FUN_0040be00, FUN_004092b0,
FUN_0045fca0, FUN_0045fad0, DoMapAI, ObjectIsBuilt, GetTileCentre, FUN_0044fe80, FUN_00459970, FUN_0041a720, FUN_0042aa90.
Patterns that did match something: wrap a body in `do { ... } while (0)` to move register saves after an early return;
`for(;;)` with the early `return` inside the loop; `switch` instead of `if` for `dec/je` chains; reusing a pointer parameter as
a spill slot (ugly, "effective" match); replacing a temp pointer with an explicit if/else.
Best remaining value: partials between 60% and 95% in files not yet worked (see `tools/agent/partials.py`), then the
inline-asm stubs are out of scope (CLAUDE.md).

### Round 3 (single session, no subagents)
Matched: FUN_004766f0, FUN_00441830, FUN_0042e560, FUN_00481170, FUN_0046da20, FindCarouselNode, FindWaterNodeByKey,
GetFirstRenderObject.
Improved: FUN_00407ad0 (95.0), FUN_00411fa0 (85.1), FUN_00469c80 (93.2), CreateSampleFromWAV (97.5).
What worked (look for these shapes first, they are cheap):
- `lea reg,[p+off]` immediately overwritten by `mov reg16,word ptr [p+off]` then `cmp reg16,[key]`: an inlined
  `memcmp(&node->id, key, 2)` (needs `<string.h>`). Node finders are `if (!node) return NULL;
  while (memcmp(&node->id, key, sizeof(node->id)) != 0) { node = node->next; if (!node) return NULL; } return node;`.
- A `break`/flag block placed after the function's `ret` (out of line) means the loop is a plain `while (cond)`, not
  `if (cond) do { } while (cond)` (FUN_004766f0).
- `and eax,0xffff; and eax,0xff` after a `unsigned short` call: `int i = Call() & 0xffff;` then index with `(unsigned char)i`
  (same shape as GetHedgeSpriteInfo).
- Decompiler-style strength-reduced loops (`offset += 0x14`, `x >= width` re-checks) match when rewritten as the natural
  `for (y...) for (x...) { tile = in-bounds ? &GameMap[y][x] : NULL; ... }` (FUN_00481170).
- `return` inside a nested `if` of a branch shares epilogues differently from an `if/else` with one `return` after it (FUN_0046da20).
- RIFF chunk scanners: `while (Read(&tag, 4) == 4) { if (tag == 'data') { ...; break; } skip chunk }` (CreateSampleFromWAV).
- Unsigned char fields read into an `int`/`short` local: the original `xor reg,reg; mov regl,[..]` vs our `movsx` tells the type.
- A `word` global split with `mov cl, dh` and no `and ecx,0xff` is two bytes: `((unsigned char *)&g)[0]` / `[1]`, not
  `(unsigned char)(g >> 8)` (GetFirstRenderObject).
Dead ends found this round (tried 5-15 variants each, register allocation or block placement only):
SetBlokePositionFromBNV (x87 stack order in the 3rd sqrt), SpeechParseWavHeader and SaveScripts and UnlinkGardenerOrder and
InitDirectSound and OpenAviAnim (early `return 0` blocks merged/duplicated differently from the original), FUN_00476d20,
ConvertWaveToPcm16, RES_OpenVolume, FUN_0046f2e0 (our compiler emits setcc), FUN_0041e4a0/FUN_0041e4b0 (movsx byte load is
narrowed away in C; probably C++ in the original), FUN_00462c60, PushSetTarget, FUN_00451390, FUN_00444a70 and FUN_00457970
(original has an extra `push ecx` local where ours reuses a dead parameter slot), FUN_0041ee40, FUN_004829c0, InsertChildIntoList,
LightUpthisDeleteIcon, FUN_00402490, FUN_00424700, FUN_0043ad90, FUN_004610f0, FUN_00418710, SetupControllers, FUN_0040e440,
FUN_0043aac0, FUN_00413450, FUN_004019c0, FUN_0042f0f0, FUN_004401b0, FUN_004070b0, FUN_00463460, RemoveObjectFromMap,
FUN_004966a0 (ours tail-duplicates the final call into each switch case), FUN_004779d0, PutObjOnMap.
Layout dead ends (flagged by `deadends.py`): LoadObjectClassAliasElements and FUN_004860f0 (loop-end pointer lands on a float
symbol), FUN_0041ed50, FUN_0041ece0, FUN_0041ed00, DoLowLevelAI (indexed table calls), FUN_00444970 (EditCursor+5184).

### Round 4 (triage first, 3 attempts per function)
Triage: 81 untried partials between 60% and 95% -> 17 layout-dependent, 0 register-only, 45 with large diffs -> 19 worked.
Improved: FUN_004428f0 (75.2), OpenAviMovie (89.1), RenderUsingRin (72.8), Calc_Item_Attractiveness (73.1), FUN_00488c80 (89.0).
Patterns that helped:
- Early-return blocks: `if (handle == NULL) { cleanup; return NULL; }` placed the failure block where the original has it (OpenAviMovie).
- Success block placed inline after the last check, with `goto ok` from the first test, matched the original block order (FUN_00488c80).
- Reusing an earlier local for the result (`rating = ...; counter = rating;`) kept it in eax like the original (Calc_Item_Attractiveness).
Dead ends (3 attempts each, scheduling/register allocation or tail merging): LLIDB_LoadTSFData, ValidateCursor,
UpdateControllerFromMouseData (the original reloads a field our compiler caches; only a `volatile` cast helps), PrintSavedGameDetails,
LoadPalette, FUN_00429f30 (a float argument goes through the FPU in the original), FUN_0046d850 and FUN_00402dc0 and FUN_00439ef0
(the original shares the tails of different switch cases/branches), FreePlayObjectList, FUN_0040f050, FUN_00465ee0,
FUN_00425e20 (our compiler reads the source of a struct copy instead of the copied global), EnterSaveGameDetails (larger stack frame).
