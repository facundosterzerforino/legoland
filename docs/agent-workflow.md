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

## Rules that worked
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
