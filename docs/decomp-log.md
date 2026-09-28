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
