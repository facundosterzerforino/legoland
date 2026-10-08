"""Project-side fixes for two reccmp false positives. Import this before comparing.

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
"""

from __future__ import annotations

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
            if raw_o is None or raw_o != raw_r:
                continue
            if o_text == raw_o or r_text == raw_r:  # one side kept the plain number
                recomp[j1 + k] = (r_addr, o_text)
    return _orig_compare(self, orig, recomp, split_points)


ParseAsm.sanitize = _sanitize
ParseAsm.reset = _reset
FunctionComparator._compare_function_assembly = _compare_function_assembly
