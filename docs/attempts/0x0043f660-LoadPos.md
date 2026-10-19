# LoadPos (0x0043f660)

## 2026-10-19: back to C

The rule that allowed a documented naked `__asm` transcription for compiler choices C cannot reach was withdrawn.
The function is the plain C that was the `#else` branch again: 42.19%. Matching has to come from C now.
Then the best C from before the asm version was put back (it is better than the plain `#else` C; its `volatile`
or extra local is commented in the source): 66.91%.
