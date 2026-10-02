#!/usr/bin/env bash
# fcitx5 입력기 그룹에 세모이를 더하고, 한글 입력기끼리 바꾸는 단축키를 건다.
#   keyboard-us ── Shift+Space ──> hangul(두벌식 순아래) 또는 semoi(세모이)
#   hangul <── Ctrl+Shift+Space ──> semoi   (keyboard-us 는 건너뜀)
# fcitx5 는 끝날 때 프로필을 덮어쓰므로 서비스를 멈춘 뒤 고치고 다시 띄운다.
set -euo pipefail

SERVICE=omarchy-fcitx5.service
CONF_DIR="$HOME/.config/fcitx5"

[[ -f /usr/share/fcitx5/addon/semoi.conf ]] || { echo "fcitx5-semoi 가 설치되어 있지 않습니다" >&2; exit 1; }

systemctl --user stop "$SERVICE"
cp "$CONF_DIR/profile" "$CONF_DIR/profile.bak-semoi"
cp "$CONF_DIR/config" "$CONF_DIR/config.bak-semoi"

python3 - "$CONF_DIR" <<'PY'
import re, sys
d = sys.argv[1]

# profile: 기본 그룹 끝에 semoi 를 더한다
p = f"{d}/profile"
s = open(p, encoding="utf-8").read()
if not re.search(r"^Name=semoi$", s, re.M):
    items = [int(n) for n in re.findall(r"^\[Groups/0/Items/(\d+)\]", s, re.M)]
    n = max(items) + 1
    block = f"[Groups/0/Items/{n}]\n# Name\nName=semoi\n# Layout\nLayout=\n\n"
    s = s.replace("[GroupOrder]", block + "[GroupOrder]")
    open(p, "w", encoding="utf-8").write(s)

# config: 한글 입력기끼리 바꾸는 단축키 (keyboard-us 는 건너뜀)
c = f"{d}/config"
s = open(c, encoding="utf-8").read()
if "[Hotkey/EnumerateForwardKeys]" not in s:
    s = s.rstrip("\n") + "\n\n[Hotkey]\nEnumerateSkipFirst=True\n\n[Hotkey/EnumerateForwardKeys]\n0=Control+Shift+space\n"
    open(c, "w", encoding="utf-8").write(s)
PY

systemctl --user start "$SERVICE"
echo "완료: Shift+Space 로 영문/한글, Ctrl+Shift+Space 로 순아래/세모이 전환"
