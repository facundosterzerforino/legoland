# To review

Matches that work but use tricks worth a second look before we call them final.
Each entry: where, what the trick is, why it is questionable, what would settle it.

## 1. FUN_00462c60 — volatile cast to force a reload (commit d6074af)

- **Where:** `src/legoland/map_object.c`, two sites:
  `*(volatile unsigned int *)&node->field_10 & 0xff`
- **Trick:** the original reloads `node->field_10` as a full dword in each branch. `volatile` stops the compiler
  from reusing the earlier test load.
- **Why questionable:** it's the same kind of hack that was removed from AdvanceFlumeMover (e8c4858). It's
  not real source; nobody writes volatile there. It also isn't a clean 100%: the OverlayILF/field load order is
  still swapped (the commit says "effective match").
- **To settle:** find a plain-C form that reloads naturally:
  - read the field through a different expression in each branch;
  - a struct copy;
  - a call between test and use;
  - the field typed as a bit-field/char in the struct.
  The AdvanceFlumeMover lesson was to write the field read in each test, not cache it.
  If nothing works, decide whether volatile is acceptable and write the rule into CLAUDE.md.

## 2. JoustRemoveObject — unnamed bit-field in a local struct (commit 7370049)

- **Trick:** the local `source` struct now has `unsigned int : 32;` for its second member, so the initialiser
  `{2, x, y}` skips it and the slot stays unwritten, like the original.
- **Why questionable:** it's close to the "no padding or dummy variables" rule. On the other hand it models a
  real field (`SampleSource.bloke`, unused for type 2) and is commented.
- **To settle:** check whether the shared `struct SampleSource` type can be used instead of the anonymous
  struct, with field-by-field assignment that leaves `bloke` unset. If that matches, prefer it. Otherwise accept
  the bit-field and say in CLAUDE.md that it's allowed.

## 3. Global definitions moved/initialised for layout (commits 9e94251, 693de2d)

- **Trick:** globals were reordered in `globals.c` and given `= {0}` / had `= 0` dropped, so MSVC6 puts them in
  .data vs .bss in the original order. This made FUN_004294b0 match and FUN_004284d0 go 85.5% → 89.8%.
  - 9e94251: `DAT_004b6150[36]`, `DAT_006139c8[0xe90]` moved next to `DAT_004b61e0` / `DAT_00614858`.
  - 693de2d: `DAT_00611648`, `DAT_00611688[11]`.
- **Why to look:** this is probably fine, since layout follows the binary's addresses. But initialisers that
  exist only to pick a section can affect other functions referencing the same globals.
- **To settle:**
  - confirm the new order matches the original addresses;
  - run a full verify and diff per-function results against the run before each commit, to check nothing else
    dropped.

## Also open (not tricks, just unfinished)

- FUN_00412100 (ride_queue.c, 93.5%): only the final `start.x += dx` register choice differs.
- PickQueueTurn 0x401f30 (71.4%): the original frame has 8 unexplained bytes. A dummy variable would be needed,
  which is not allowed.
