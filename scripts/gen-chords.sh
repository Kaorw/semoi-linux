#!/usr/bin/env bash
# sebeol-type(https://github.com/Kaorw/sebeol-type)의 세모이 엔진으로 test/chords.tsv 를 다시 만든다.
#   scripts/gen-chords.sh /path/to/sebeol-type
# Node 22.6 이상(타입 제거 지원)이 필요하다.
set -euo pipefail
src="${1:?sebeol-type 경로를 주세요}/src"
here="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"; trap 'rm -rf "$tmp"' EXIT

cp -r "$src"/{engine,layout,hangul} "$tmp/"
# node 의 타입 제거는 확장자 없는 상대 import 와 값처럼 쓴 타입 import 를 못 읽으므로 고쳐 준다
sed -i -E "s#(from '\.\.?/[^']+)'#\1.ts'#" "$tmp"/*/*.ts
sed -i 's/SEMOE_SHIFT_PUNCT, Tok }/SEMOE_SHIFT_PUNCT, type Tok }/' "$tmp/engine/semoe.ts"
cp "$here/scripts/gen-chords.ts" "$tmp/"

python3 "$here/scripts/gen-rules.py" "$src/layout/semoe.ts" > "$here/src/rules.inc"
node "$tmp/gen-chords.ts" > "$here/test/chords.tsv"
wc -l "$here/test/chords.tsv"
