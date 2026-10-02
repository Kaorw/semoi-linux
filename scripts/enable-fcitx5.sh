#!/usr/bin/env bash
# fcitx5 입력기 그룹에 세모이를 더하고(또는 빼고), 한글 입력기끼리 바꾸는 단축키를 건다.
#
#   scripts/enable-fcitx5.sh                 세모이 추가 + Ctrl+Shift+Space 로 입력기 순환
#   scripts/enable-fcitx5.sh --hotkey KEY    순환 단축키를 KEY 로 (예: Control+Shift+Hangul)
#   scripts/enable-fcitx5.sh --no-hotkey     단축키는 건드리지 않음
#   scripts/enable-fcitx5.sh --remove        입력기 그룹에서 세모이를 뺌
#
# 순환 단축키는 첫 입력기(보통 keyboard-us)를 건너뛰고 나머지 입력기를 돈다.
# 한글 입력기가 세모이 하나뿐이면 fcitx5 의 기본 전환 키(기본값 Ctrl+Space)만으로 충분하다.
#
# fcitx5 는 끝날 때 프로필을 덮어쓰므로, 돌고 있으면 멈춘 뒤 고치고 다시 띄운다.
# 고치기 전 파일은 *.bak-semoi 로 남긴다.
set -euo pipefail

action=add
hotkey=Control+Shift+space
while (($#)); do
    case "$1" in
        --remove) action=remove ;;
        --hotkey) hotkey="${2:?--hotkey 에 키를 주세요}"; shift ;;
        --no-hotkey) hotkey= ;;
        -h|--help) sed -n '2,13p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "알 수 없는 옵션: $1" >&2; exit 2 ;;
    esac
    shift
done

conf_dir="${XDG_CONFIG_HOME:-$HOME/.config}/fcitx5"

if [[ $action == add ]] && ! compgen -G '/usr/share/fcitx5/addon/semoi.conf' >/dev/null \
    && ! compgen -G '/usr/local/share/fcitx5/addon/semoi.conf' >/dev/null; then
    echo "fcitx5-semoi 가 설치되어 있지 않습니다. 먼저 설치해 주세요." >&2
    exit 1
fi

# ── 돌고 있는 fcitx5 멈추기 ─────────────────────────────
# systemd 사용자 서비스로 돌면 그 서비스를, 아니면 프로세스를 같은 인자로 다시 띄운다.
# (SEMOI_NO_RESTART=1 이면 fcitx5 를 건드리지 않는다: 테스트용)
service=
cmdline=()
pid=
if [[ -n ${SEMOI_NO_RESTART:-} ]]; then
    :
elif command -v systemctl >/dev/null; then
    service=$(systemctl --user list-units --type=service --state=running --no-legend 2>/dev/null \
        | awk '{print $1}' | grep -i fcitx5 | head -n1 || true)
fi
[[ -z ${SEMOI_NO_RESTART:-} && -z $service ]] && pid=$(pgrep -u "$(id -u)" -x fcitx5 | head -n1 || true)
if [[ -n $service ]]; then
    systemctl --user stop "$service"
elif [[ -n $pid ]]; then
    mapfile -d '' -t cmdline < "/proc/$pid/cmdline"
    kill "$pid"
    for _ in $(seq 50); do kill -0 "$pid" 2>/dev/null || break; sleep 0.1; done
fi

restart() {
    if [[ -n $service ]]; then
        systemctl --user start "$service"
    elif ((${#cmdline[@]})); then
        # -d 로 데몬이 되므로 이 스크립트가 끝나도 남는다
        setsid "${cmdline[@]}" -d >/dev/null 2>&1 < /dev/null || true
    fi
}
trap restart EXIT

# ── 설정 고치기 ──────────────────────────────────────
mkdir -p "$conf_dir"
for f in profile config; do
    [[ -f $conf_dir/$f ]] && cp "$conf_dir/$f" "$conf_dir/$f.bak-semoi"
done

python3 - "$conf_dir" "$action" "$hotkey" <<'PY'
import os, re, sys

conf_dir, action, hotkey = sys.argv[1:]

def read(path):
    """fcitx 설정(INI) → [(섹션, [줄...])]. 섹션 앞의 줄은 섹션 이름 ''."""
    secs = [["", []]]
    if os.path.exists(path):
        for line in open(path, encoding="utf-8").read().splitlines():
            m = re.match(r"^\[(.+)\]\s*$", line)
            if m:
                secs.append([m.group(1), []])
            else:
                secs[-1][1].append(line)
    return secs

def write(path, secs):
    out = []
    for name, lines in secs:
        if name:
            out.append(f"[{name}]")
        while lines and not lines[-1].strip():
            lines.pop()
        out += lines + [""]
    open(path, "w", encoding="utf-8").write("\n".join(out).lstrip("\n"))

def section(secs, name):
    for s in secs:
        if s[0] == name:
            return s
    s = [name, []]
    secs.append(s)
    return s

def setkey(sec, key, value):
    for i, line in enumerate(sec[1]):
        if re.match(rf"^{re.escape(key)}=", line):
            sec[1][i] = f"{key}={value}"
            return
    while sec[1] and not sec[1][-1].strip():
        sec[1].pop()
    sec[1].append(f"{key}={value}")

def get(sec, key):
    for line in sec[1]:
        if line.startswith(key + "="):
            return line.split("=", 1)[1]
    return None

# profile: 첫 그룹(Groups/0)의 입력기 목록
p = os.path.join(conf_dir, "profile")
secs = read(p)
item = re.compile(r"^Groups/0/Items/(\d+)$")
items = [(int(item.match(n).group(1)), get([n, l], "Name")) for n, l in secs if item.match(n)]
names = [name for _, name in sorted(items)]

if action == "add" and "semoi" not in names:
    if not names:
        names = ["keyboard-us"]
    names.append("semoi")
elif action == "remove":
    names = [n for n in names if n != "semoi"]

# 첫 그룹의 항목을 다시 쓴다 (다른 그룹·설정은 그대로 둔다)
secs = [s for s in secs if not item.match(s[0])]
g = section(secs, "Groups/0")
setkey(g, "Name", get(g, "Name") or "Default")
setkey(g, "Default Layout", get(g, "Default Layout") or "us")
if get(g, "DefaultIM") == "semoi" and action == "remove":
    setkey(g, "DefaultIM", names[-1] if names else "keyboard-us")
elif not get(g, "DefaultIM"):
    setkey(g, "DefaultIM", names[-1] if names else "keyboard-us")
at = secs.index(g) + 1
for i, name in enumerate(names):
    secs.insert(at + i, [f"Groups/0/Items/{i}", [f"Name={name}", "Layout="]])
order = section(secs, "GroupOrder")
if not get(order, "0"):
    setkey(order, "0", get(g, "Name"))
write(p, secs)
print("입력기 그룹:", " / ".join(names))

# config: 입력기 순환 단축키 (첫 입력기는 건너뜀)
if action == "add" and hotkey:
    c = os.path.join(conf_dir, "config")
    secs = read(c)
    setkey(section(secs, "Hotkey"), "EnumerateSkipFirst", "True")
    fwd = section(secs, "Hotkey/EnumerateForwardKeys")
    keys = [line.split("=", 1)[1] for line in fwd[1] if re.match(r"^\d+=", line)]
    if hotkey not in keys:
        setkey(fwd, str(len(keys)), hotkey)
    write(c, secs)
    print("입력기 순환:", hotkey)
PY
