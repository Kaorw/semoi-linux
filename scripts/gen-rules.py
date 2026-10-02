#!/usr/bin/env python3
"""sebeol-type 의 세모이 결합 규칙(RULES_SRC)을 C++ 포함 파일로 옮긴다.

  python3 scripts/gen-rules.py ~/Work/sebeol-type/src/layout/semoe.ts > src/rules.inc
"""
import re, sys

src = open(sys.argv[1], encoding="utf-8").read()
body = src[src.index("const RULES_SRC"):]
body = body[: body.index(";")]
rules = " ".join(re.findall(r"'([^']*)'", body)).split()
print("// 자동 생성: scripts/gen-rules.py — 손으로 고치지 말 것")
print("// 원본: sebeol-type src/layout/semoe.ts RULES_SRC (OHI K3_Semoe_combination_table 차례)")
for r in rules:
    ab, res = r.split("=")
    a, b = ab.split("+")
    print(f'{{"{a}", "{b}", "{res}"}},')
