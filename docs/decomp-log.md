# Decomp process log

A running, worked-example log of how functions get matched in this project.

## The loop, in one paragraph

Pick an unmatched function (`STUB()` body). Look at the **original** machine code at its address.
Work out what C would produce it. Write that C, rebuild with the *original* MSVC6 compiler, and
let `reccmp` compare the compiled bytes against the original. Adjust the C until the function reports
100%. Addresses are normalized, so only instruction content/order/registers matter.

## Commands used (all from `~/legoland` in WSL)

```sh
cmake --build build                      # recompile with MSVC6 (via wibo)
./tools/verify                           # per-function match % + totals
./tools/verify -v 0x00451e20             # asm diff for one function
uv run tools/progress.py                 # per-file progress table
objdump -d -M intel --start-address=0x451e20 --stop-address=0x451f70 external/legoland.exe
```

Note: `verify -v` only shows as many lines as *our* function has, so a stub shows one line.
To read the full original, disassemble it with `objdump` (Intel syntax) as above.

---

## Example 1: `FUN_00451e20` (certificate.c) — 0% → 100% first try

### 1. Read the original asm

```
push ecx                    ; reserve 4 bytes of stack = one local (time_t)
lea  eax,[esp]              ; &local
push edi
push eax
call _time                  ; time(&local)
lea  ecx,[esp+8]            ; &local again (esp moved by 2 pushes)
push ecx
call _localtime             ; localtime(&local)
push eax
call _asctime               ; asctime(tm)
mov  edx,eax
or   ecx,-1 / mov edi,edx / xor eax,eax / repnz scasb / not ecx / dec ecx
                            ; inlined strlen(str)
push edx ; push 0x80ffa0 ; push 0x4b86fc
mov  [ecx+edx-1],al         ; al == 0  ->  str[strlen(str)-1] = 0   (strip the '\n')
call 0x451740               ; f(0x4b86fc, 0x80ffa0, str)
add  esp,0x18               ; 6 pushes = time/localtime/asctime args + 3 args
neg eax / sbb eax,eax / neg eax   ; return (result != 0)
```

### 2. Identify what everything is

- The CRT calls have names in `data/crt.csv` (`grep -i 49fbc6 data/crt.csv`): `_time`, `_localtime`,
  `_asctime`. So we just `#include <time.h>` — the real MSVC6 headers are used.
- `0x80ffa0` is already a known global (`DAT_0080ffa0` in `globals.h`).
- `0x4b86fc` is in `.rdata`; reading the bytes from the exe gives the string `"EGC.bmp"`. It is annotated
  with `// STRING: LEGOLAND 0x004b86fc` so reccmp knows about it.
- `strlen` inline via `repnz scasb` is just how MSVC6 /O2 expands `strlen()`.
- `neg/sbb/neg` is the MSVC6 idiom for `x != 0` (giving 0 or 1).

### 3. The C

```c
int FUN_00451e20(void) {
    time_t now;
    char *str;

    time(&now);
    str = asctime(localtime(&now));
    str[strlen(str) - 1] = '\0';
    // STRING: LEGOLAND 0x004b86fc
    return FUN_00451740("EGC.bmp", (char *)&DAT_0080ffa0, str) != 0;
}
```

The callee `FUN_00451740` got a real signature `int (char*, char*, char*)` in `certificate.h`
(still a `STUB()` body; that is the next one to do).

### 4. Result

`FUN_00451e20 is 100.00% similar` — total went from 3426 to 3430 implemented.

Takeaway: for small functions the process is mostly *reading* — naming calls, addresses and idioms.
The MSVC6 idioms (inline `strlen`, `neg/sbb/neg`, `add esp` batching) come out on their own when
the source is written in the natural way.

---

## Example 2: `AdjustPSampleFreq` (sound_sfx.c, 128 bytes) — 0% → 100% first try

Picking a target: a small script lists stubs with their size (distance to the next function's address)
so we can choose by size. Named functions (not `FUN_...`) are easier: the name usually says what to do.

### Original asm, decoded

```
and  edi,0xffff            ; p = (unsigned short)param_2
call _rand ; cdq
lea  ecx,[edi+edi] ; idiv ecx   ; edx = rand() % (2*p)   (signed remainder)
mov  esi,edx ; sub esi,edi ; add esi,0x64    ; pct = rem - p + 100
call 0x492a60(sample)      ; current frequency   (FUN_00492a60)
imul ecx,esi               ; freq * pct
mov eax,0x51eb851f ; imul ; sar edx,5 ; shr eax,31 ; add edx,eax   ; /100 (signed magic-number divide)
call 0x492a20(sample, ...) ; SetSampleFrequency(sample, freq*pct/100)
```

Idioms worth knowing: `mov eax,0x51eb851f ... imul ... sar 5` is how MSVC divides by the constant 100;
`cdq; idiv` means signed `%`. So the intent is: *randomly detune the sample by ±p percent*.

### The C

```c
LEGO_EXPORT void AdjustPSampleFreq(struct Sample *sample, unsigned int param_2) {
    unsigned short range = (unsigned short)param_2;
    int percent = rand() % (range * 2) - range + 100;

    SetSampleFrequency(sample, (int)FUN_00492a60(sample) * percent / 100);
}
```

`FUN_00492a60` is really "get frequency" but is typed as returning a pointer (it reuses its `sample`
parameter as an out-slot); a cast at the call site was enough, so it was left untouched.

Result: 100%. Progress 74.53% → 74.69%.

### Cost of this one

About 8k tokens end to end (stub search, disassembly, two lookups, one edit, build+verify) — cheap when it
matches first time. The expensive cases are the ones that don't (each retry = edit + build + verify).

---

## Scaling up: batches of agents (Progress 74.5% -> 80.8%)

After the two hand-done examples the work was parallelised. Each agent gets one file (or one address
range of a big file) in its **own git worktree** with its own build directory, so they never share a
`build/`. The main tree merges their branches, rebuilds clean, and re-verifies everything: agent-reported
numbers are only claims until `tools/verify` on the merged tree says so.

Practical setup notes
- Header changes need `--clean-first` (no header dependency tracking).
- Shared files (`globals.h`, `globals.c`, `*.h`) use git's `merge=union` (`.git/info/attributes`) so purely
  additive edits from different agents combine. Real conflicts do still happen when two agents retype the
  same function: keep the implemented body, fix the header prototype, then rebuild.
- "Implemented" (share of annotated items with a body) barely moves; **Progress = Implemented x Accuracy**
  is the number to watch.
- `100.00%*` means reccmp accepted the function as an effective match with a small difference (typically a
  register swap or instruction order); a clean 100.00% is exact.
- Cheap, focused Sonnet agents clear small stubs; the stronger model was used only for the recurring hard
  cases and near-misses.

## Recipes found (all pure C)

**Linked-list lookup by 16-bit key** (defeated ~7 attempts). The original compares with an inlined
2-byte `memcmp`, which MSVC6 never hoists out of the loop:
```c
struct T *cur = HEAD;
if (cur != NULL) {
    do {
        if (memcmp(&cur->id, key, 2) == 0) return cur;
        cur = cur->next;
    } while (cur != NULL);
}
return NULL;
```
`if (cur) do {...} while (cur)` gives the rotated loop with the compare duplicated; every `==` form gets
hoisted (`cmp [eax],cx`). Linked-list *unlink* (with a `&cur->next` walker) is still unsolved.

Other tricks that decided matches
- A 2-byte `TileId` union passed by value is one 4-byte stack slot; pass `&tile` to helpers.
- `if (x == 0) x = K;` beats a ternary when constants are pushed inside branches; write the final call in
  both branches so the compiler merges the tail (`push 0xe; jmp`).
- Flag updates as separate statements (`f &= ~M; ... f |= B;`), not one expression.
- A parameter used once is loaded at the point of use; used twice it is cached in a callee-saved register
  (extra push/pop). Use counts change register allocation.
- `float m[4][4]` parameters indexed `m[i][j]` (vs a flat `float *`) fixed a 27% match.
- Cast both sides of a pointer-loop bound to `(int)` for the signed `jle` compare.
- Small clears: `memset(node, 0, sizeof *node)` reproduces the original's inlined memset.
- A callee returning `edx:eax` can be declared `unsigned __int64` and split with `(unsigned)r` / `(unsigned)(r >> 32)`.
- Index an array (`tbl[i]`) instead of walking a struct pointer when the original loop compares signed.
- Struct definitions must sit above the `// FUNCTION` tag, or reccmp reports "Failed to find function symbol".
- Float constants: whether to use an `extern float` at the exe address or a literal depends on whether
  the address is listed in `data/floats.csv`; a wrong choice shows as `(FLOAT)` vs `(DATA)` and costs a few %.
- Not reachable in pure C (skipped): `push ebp` frames, `fstcw`/`fldcw`/`rdtsc`, inline x87 exp sequences.
