# Remaining work (as of 2026-10-05, `be61569`)

`./tools/verify` reported `Progress: 94.72%` for this build. That figure is the sum of every function's match score divided
by all 3634 functions reccmp counts, which include CRT and import entries. Counted per game function (`// FUNCTION:`):

| State | Functions |
|---|---|
| 100% (exact or "effective") | 2958 |
| Pure-C partials (1-99%) | 248 |
| Inline asm, partial | 5 + 3 (see below) |
| Inline asm, still `STUB()` (0%) | 44 |
| Total | 3255 |

Recount with `uv run tools/agent/status.py` (per function) and `uv run tools/agent/partials.py` (pure-C partials).

## Inline-asm functions: match with `__asm`

**All 44 functions still at 0% are hand-written assembly in the original.** MSVC6 never emits these instructions, so pure C
cannot match them (see "Functions With an ebp Frame" in `decomp-tips.md`); they need inline `__asm`, allowed since
2026-10-05. Each still at `STUB()` has a comment that names the fingerprint. The port replaces them with plain-C equivalents tagged `// [library:asm]`.
`find_inline_asm` in `tools/progress.py` detects them, and `partials.py` leaves them out.

| TU | Count | Functions (fingerprint) |
|---|---|---|
| castle | 21 | FUN_0041e130, FUN_00420e90, FUN_00423140, FUN_004234e0, FUN_00428cb0, FUN_004292f0 (rdtsc); FUN_0041f8d0, FUN_0041fa10, FUN_0041fba0, FUN_0041fd80, FUN_0041ff80, FUN_00423350, FUN_00428860 (xchg); FUN_00420810, FUN_00420a20, FUN_00420c40 (fistp + rdtsc); FUN_004236f0, FUN_00423730 (fldcw/fstcw); FUN_00426250, FUN_004263a0, FUN_0042a2f0 (fistp) |
| draw | 13 | FUN_00464480 (xchg, rep movsw/stosw); ZBufferHelper, FUN_00465240 (pusha, shrd, xchg); FUN_00464ee0, SoftPrint_XBltFast (pusha); FUN_00466d80, FUN_00467180, FUN_004673f0, FUN_004677b0, FUN_00467b00, FUN_00467d10, FUN_00468040, FUN_00468410 (rol, rep movsw/stosw) |
| render | 4 | FUN_00486590, FUN_004877b0 (shrd, xchg); FUN_00486c70, FUN_00487d40 (fistp, shrd, xchg) |
| man3d | 3 | FUN_0043fa80, SetPersonRotation (fistp); FUN_00440a30 (fistp, shrd) |
| render3d | 2 | FUN_00441980 (fistp); TransformVectorsL (shrd) |
| copters | 1 | FUN_00404630 (fistp) |

Inline asm, partly written in C (finish with `__asm`): Render3DPerson 51.6%, RenderTransSprite 18.8%, SoftPrint_Clear 18.9%,
FUN_00488730 17.5%, ApplyObjectOrientationToPerson 7.0%; HASM_lego_sqrtf and HASM_lego_invsqrtf 95.2% (`__declspec(naked)`),
LLSPlay 16.2% (`int 3`). Inline asm already at 100%: _ftol, RenderingComplete, ReadBigEndianU32, ReadBigEndianU16.

## Pure-C partials that have not been worked on

Every other partial has had at least one matching attempt (`agent-workflow.md` lists the rounds and the dead ends). These were
written once and never iterated on, highest score first:

| % | Function | TU |
|---|---|---|
| 95.48 | RenderCursor | map_object |
| 95.06 | FUN_004227c0 | castle |
| 83.61 | FUN_00433840 | jungle_cruise |
| 81.95 | LoadBaseMap | map_object |
| 81.68 | RenderBuildObjectIcon | icon |
| 81.60 | RenderFullMap | mapscreen |
| 81.26 | FUN_004198a0 | boating_school |
| 79.72 | FUN_00402150 | ride_bloke |
| 79.43 | PrintCertificate | certificate |
| 78.85 | FUN_0043e410 | plane_ride |
| 78.63 | FUN_0040bf70 | log_flume |
| 77.47 | FUN_004064d0 | fort |
| 76.96 | FUN_00405bd0 | driving_school |
| 76.47 | FUN_00406660 | fort |
| 76.36 | CoptersUpdate | copters |
| 75.35 | FUN_00417430 | temple_slide |
| 75.09 | FUN_00415220 | safari_ride |
| 74.30 | FUN_0043c950 | spinning_barrels |
| 73.85 | RES_OpenFileFromVolume | resource |
| 72.99 | FUN_0042a020 | castle |
| 72.33 | FUN_0043a1e0 | shops |
| 70.04 | FUN_00477bd0 | gamemain |
| 69.93 | ParseScriptResFile | gamemain |
| 69.44 | FUN_0043ea30 | dialog |
| 69.04 | FUN_00407c30 | joust |
| 69.03 | FUN_00402780 | ride_bloke |
| 67.67 | FUN_00423a10 | castle |
| 67.51 | FUN_0043bac0 | space_tower |
| 63.32 | FUN_004025d0 | ride_bloke |
| 62.16 | DrawNewObjectPopup | popupinfo |
| 61.81 | FUN_00401f30 | ride_bloke |
| 61.66 | LogFlumeEntranceAddObject | log_flume |
| 61.50 | FUN_00466770 | draw |
| 61.48 | LoadPos | man3d |
| 60.85 | FUN_004316f0 | eatery |
| 60.26 | PrintProfileDetails | profile |
| 57.56 | RES_OpenFile | resource |
| 56.52 | FUN_004608c0 | map_object |
| 56.52 | FUN_0041c940 | boating_school |
| 54.66 | FUN_0043f0b0 | dialog |
| 46.91 | FUN_00439950 | shops |
| 45.11 | FUN_0040ae90 | log_flume |
| 41.89 | FUN_0042fbb0 | eatery |
| 41.17 | FUN_00442040 | render3d |
| 40.52 | FUN_00411680 | log_flume |
| 38.41 | SearchJunglePathConnected | jungle_cruise |
| 33.54 | FUN_00485fe0 | print_sprite |

This is the round-5 "not attempted yet" list from `agent-workflow.md` (minus RenderLogFlumeCorner, worked on since), plus
FUN_0042a020 and FUN_00485fe0, which no list covered. Some got score gains while they were first being written (RenderCursor
43% -> 95.5%, FUN_004227c0, CoptersUpdate), but none went through a round's triage-and-3-attempts pass. FUN_00477bd0 and
ParseScriptResFile only got behaviour fixes since.

Also not attempted, but triaged as unlikely to match from C (`agent-workflow.md`, round 5): 37 layout-dependent and
8 register-only partials. Try them after the table above.
