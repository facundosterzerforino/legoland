"""Project-side fixes for reccmp false positives. Import this before comparing.

1. Immediates that look like addresses in one image only.

reccmp decides for each image on its own whether an immediate operand is an address: it is if the
value is a relocation target or falls inside any known symbol. Our image lays data out differently from
the original, so a plain constant can land inside a symbol in one image and not the other. Example:
`mov ebx, 0x7fffff` is byte-identical in both, but the original shows `EditCursor+5183` and ours shows
`0x7fffff`, so the line counts as a difference.

The fix: when an instruction's raw (unsanitized) text is identical in both images and at least one side
left the value as a plain number, the two lines count as equal. If both sides resolved the value to a
symbol, the line is left alone: that may be a real pointer mismatch.

2. Calls through a table: `call dword ptr [edx*4 + 0x4b9d44]`. reccmp names the displacement of
`[reg + address]` operands for every instruction except `call`, where it only handles a plain
`[address]`. So a call through a function-pointer table (ScriptEventHandlers[type](node)) always kept
its raw address, which differs between the images. The fix gives calls the same relocated-displacement
naming as every other instruction.

3. Pointers one past the end of an array: `cmp esi, 0x4be718` for `entry < &DAT_004bdeb8[0x86]`. The
address after an array is named after whatever symbol happens to follow it in that image (here
`___piob` in ours, nothing in the original). A line that still differs is rebuilt from the raw text on
both sides, naming each address that is not inside a symbol as `<symbol the byte before is in>+1`. If
both sides then read the same, the line is equal.

4. The `__try` scope table: `push -1; push <scopetable>; push __except_handler3`. The table is compiler
data that is never annotated, so its address is a plain number that differs between images. A `push`
of a plain number directly before `push __except_handler3` counts as equal.
"""

from __future__ import annotations

import re
from difflib import SequenceMatcher

from reccmp.compare.asm.parse import ParseAsm, displace_replace_regex
from reccmp.compare.functions import FunctionComparator

_orig_sanitize = ParseAsm.sanitize
_orig_reset = ParseAsm.reset
_orig_compare = FunctionComparator._compare_function_assembly


def _sanitize(self, inst):
    result = _orig_sanitize(self, inst)
    address, _size, mnemonic, op_str = inst
    if result[0] == "call":
        result = (result[0], displace_replace_regex.sub(self.hex_replace_relocated, result[1]))
    self._raw_text[address] = f"{mnemonic} {op_str}".rstrip()
    return result


def _reset(self):
    _orig_reset(self)
    self._raw_text = {}


def _end_names(sanitizer, raw: str) -> str:
    """raw instruction text with every address-like number named, an address just past a symbol as
    `<that symbol>+1` (see fix 3)."""

    def name(m: re.Match) -> str:
        value = int(m.group(0), 16)
        if value < 0x10000:
            return m.group(0)
        inside = sanitizer.lookup(value - 1)
        if inside is not None and "+" in inside:
            return f"{inside}+1"
        exact = sanitizer.lookup(value)
        return exact if exact is not None else m.group(0)

    return re.sub(r"0x[0-9a-f]+", name, raw)


def _compare_function_assembly(self, orig, recomp, split_points):
    orig_raw = getattr(self.orig_sanitize, "_raw_text", {})
    recomp_raw = getattr(self.recomp_sanitize, "_raw_text", {})
    recomp = list(recomp)
    opcodes = SequenceMatcher(None, [t for _, t in orig], [t for _, t in recomp], autojunk=False).get_opcodes()
    for tag, i1, i2, j1, j2 in opcodes:
        if tag != "replace" or i2 - i1 != j2 - j1:
            continue
        for k in range(i2 - i1):
            o_addr, o_text = orig[i1 + k]
            r_addr, r_text = recomp[j1 + k]
            raw_o, raw_r = orig_raw.get(o_addr), recomp_raw.get(r_addr)
            if raw_o is None or raw_r is None:
                continue
            if raw_o != raw_r:
                # fix 4: the scope table pushed before __except_handler3
                nxt_o = orig[i1 + k + 1][1] if i1 + k + 1 < len(orig) else ""
                nxt_r = recomp[j1 + k + 1][1] if j1 + k + 1 < len(recomp) else ""
                if (re.fullmatch(r"push 0x[0-9a-f]+", raw_o) and re.fullmatch(r"push 0x[0-9a-f]+", raw_r)
                        and "__except_handler3" in nxt_o and nxt_o == nxt_r):
                    recomp[j1 + k] = (r_addr, o_text)
                    continue
                # fix 3: pointers one past the end of the same symbol
                alt_o = _end_names(self.orig_sanitize, raw_o)
                if alt_o != raw_o and alt_o == _end_names(self.recomp_sanitize, raw_r):
                    recomp[j1 + k] = (r_addr, o_text)
                continue
            if o_text == raw_o or r_text == raw_r:  # one side kept the plain number
                recomp[j1 + k] = (r_addr, o_text)
    return _orig_compare(self, orig, recomp, split_points)


ParseAsm.sanitize = _sanitize
ParseAsm.reset = _reset
FunctionComparator._compare_function_assembly = _compare_function_assembly
