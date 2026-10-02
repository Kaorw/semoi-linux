# semoi-linux

세모이(세벌식 모아치기 e) 자판을 리눅스 Fcitx5에서 쓰게 해 주는 입력기 애드온(`fcitx5-semoi`)입니다. 한 글자의 첫소리·가운뎃소리·받침 키를 한꺼번에 눌렀다가 모두 떼면 그 글자가 들어갑니다.

리눅스에서 세모이를 쓰려면 지금까지는 libhangul을 패치해 빌드하거나 Wine으로 날개셋을 띄워야 했습니다. 이 애드온은 Fcitx5에 바로 붙고 다른 한글 입력기를 바꾸지 않습니다. `fcitx5-hangul` 같은 기존 한글 입력기와 같은 입력기 목록에 두고 단축키로 바꿔 쓸 수 있습니다.

## 설치

### Arch Linux 계열 (Omarchy, Manjaro, EndeavourOS 등)

```sh
git clone https://github.com/Kaorw/semoi-linux
cd semoi-linux/packaging
makepkg -si                  # 빌드 + 테스트 + pacman 패키지로 설치
../scripts/enable-fcitx5.sh  # 입력기 목록에 세모이 추가
```

### 다른 배포판

CMake로 빌드합니다. Fcitx5 5.1 이상과 그 개발 헤더, `extra-cmake-modules`, C++20 컴파일러가 필요합니다. 패키지 이름은 배포판마다 다릅니다. 예를 들어 Debian·Ubuntu는 `libfcitx5core-dev fcitx5-modules-dev extra-cmake-modules cmake g++`, Fedora는 `fcitx5-devel extra-cmake-modules cmake gcc-c++`입니다. Arch 밖에서는 아직 시험해 보지 않았습니다.

```sh
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build       # 선택
sudo cmake --install build
scripts/enable-fcitx5.sh
```

`/usr`이 아닌 경로에 설치하면 Fcitx5가 애드온을 찾지 못합니다. 그럴 때는 Fcitx5와 같은 접두어를 쓰세요.

### 입력기 목록에 넣기

`scripts/enable-fcitx5.sh`가 하는 일은 다음과 같습니다.

- `~/.config/fcitx5/profile`의 첫 입력기 그룹 끝에 세모이를 더합니다. 설정 파일이 없으면 `keyboard-us / semoi`로 새로 만듭니다.
- `~/.config/fcitx5/config`에 입력기 순환 단축키 **Ctrl+Shift+Space**를 겁니다. 이 키는 첫 입력기(영문)를 건너뛰고 한글 입력기들 사이만 돕니다.
- Fcitx5가 끝날 때 설정을 덮어쓰기 때문에, 돌고 있는 Fcitx5를 잠깐 멈추고 고친 뒤 다시 띄웁니다. systemd 사용자 서비스로 돌고 있으면 그 서비스를 다시 시작하고, 아니면 같은 인자로 다시 실행합니다.
- 고치기 전 파일은 `*.bak-semoi`로 남깁니다. 여러 번 실행해도 결과는 같습니다.

```sh
scripts/enable-fcitx5.sh --hotkey Control+Shift+Hangul   # 순환 단축키 바꾸기
scripts/enable-fcitx5.sh --no-hotkey                     # 단축키는 그대로 두기
scripts/enable-fcitx5.sh --remove                        # 목록에서 세모이 빼기
```

스크립트 대신 `fcitx5-configtool`에서 입력기 목록에 "Semoi"(세모이)를 더해도 됩니다.

### 전환

| 키 | 동작 |
|---|---|
| Fcitx5 전환 키 (기본 Ctrl+Space) | 영문 ↔ 마지막에 쓴 한글 입력기 |
| Ctrl+Shift+Space | 한글 입력기끼리 순환 (예: 두벌식 ↔ 세모이) |

한글 입력기가 세모이 하나뿐이면 Fcitx5 전환 키만으로 충분합니다.

### 지우기

```sh
scripts/enable-fcitx5.sh --remove
sudo pacman -R fcitx5-semoi                      # Arch
sudo xargs rm < build/install_manifest.txt       # CMake 로 설치했을 때
```

## 자판

배열과 결합 규칙은 신세기 님의 [Semo-e_keyboard](https://github.com/Sinseiki/Semo-e_keyboard)를 [OHI의 세모이 구현](https://github.com/Sinseiki/ohi) 차례대로 옮긴 [신세벌 타자](https://github.com/Kaorw/sebeol-type)의 규칙을 따릅니다. 그 연습기([kaorw.github.io/sebeol-type](https://kaorw.github.io/sebeol-type/?scheme=semoe))에서 익힌 조합이 여기서도 똑같이 들어갑니다.

- 오른손 `Y U I O · H J K L · N M` = 첫소리 ㅁ ㄴ ㄷ ㅂ · ㅎ ㅇ ㄱ ㅈ · ㅅ ㄹ
- 왼손 `R T · D F G · C V B` = 홀소리 ㅓ ㅕ · ㅣ ㅏ ㅡ · ㅔ ㅗ ㅜ
- 왼손 `Q W E · A S · Z X` = 받침 ㅅ ㅂ ㄹ · ㅇ ㄴ · ㅁ ㄱ
- `;` = 받침 ㅆ. 다른 받침과 모아 ㅊ ㅍ ㅈ ㅀ ㅎ ㄷ ㅋ을 만듭니다.
- `.` = 겹홀소리용 ㅗ. 다른 홀소리와 모아 ㅝ ㅒ ㅑ ㅖ ㅛ ㅠ ㅘ ㅚ를 만듭니다.
- 거센소리·된소리 첫소리는 ㅎ(`H`)이나 ㅇ(`J`)과 함께 누릅니다. 타 = `I H F`, 까 = `J K F`
- `P` = `.`, `,` = `,`, 숫자는 그대로입니다. Shift+`/` `1` `;` = `?` `!` `:`
- 글자 키와 스페이스를 함께 누르면 띄어쓰기까지 한 번에 들어갑니다. 따로 누른 스페이스는 그냥 띄어쓰기입니다.
- 키를 누르고 있는 동안에는 지금까지 모은 글자를 밑줄로 보여 줍니다.
- Ctrl·Alt·Super와 함께 누른 키와 배열에 없는 키는 입력기가 건드리지 않습니다.

키는 물리 위치(evdev 키코드)로 판정합니다. 시스템 키보드 배치를 Colemak 등으로 바꿔 두어도 QWERTY 자리 기준으로 같은 글자가 나옵니다.

다음 글자의 키를 누를 때 앞 글자의 키가 하나라도 눌려 있으면 두 글자가 한 묶음으로 합쳐집니다. 앞 글자의 키를 모두 뗀 다음에 다음 글자를 누르세요.

아직 없는 것: 약어, 옛한글, 기호 확장, 이어치기 전용 모드.

### 동작 환경

입력기가 키를 뗀 이벤트까지 받아야 합니다. Wayland(`zwp_input_method_v2`를 쓰는 Hyprland, Sway, KDE 등), GTK·Qt 입력기 모듈, XIM은 모두 뗌 이벤트를 넘겨 줍니다. Omarchy(Hyprland)에서 시험했습니다.

## 개발

```sh
cmake -B build -S . && cmake --build build && ctest --test-dir build --output-on-failure
```

| 경로 | 내용 |
|---|---|
| `src/semoi.cpp` | Fcitx5와 무관한 모아치기 엔진: 키 묶음 → 글자 |
| `src/rules.inc` | 결합 규칙 표 (생성 파일) |
| `src/engine.cpp` | Fcitx5 애드온: 누름/뗌을 모아 두었다가 모두 떼면 확정 |
| `data/` | Fcitx5 애드온·입력기 등록 파일 |
| `packaging/PKGBUILD` | 이 작업 트리를 빌드하는 Arch 패키지 |
| `scripts/enable-fcitx5.sh` | 입력기 목록·단축키 설정 |

테스트는 둘입니다.

- `chords`: 낱자 키 5개까지의 모든 묶음과 문장부호·스페이스 조합 101,603개를 신세벌 타자(TypeScript) 엔진의 결과(`test/chords.tsv`)와 대조합니다.
- `engine`: 실제 Fcitx5 `Instance`에 애드온을 올리고, 누름/뗌 이벤트로 글자 확정, 자동 반복, 스페이스, Shift, 단축키 통과를 확인합니다. Fcitx5의 testfrontend 모듈이 있을 때만 빌드됩니다. 빼려면 `-DBUILD_TESTING=OFF`를 줍니다.

신세벌 타자의 결합 규칙이 바뀌면 규칙 표와 대조 자료를 다시 만듭니다. Node 22.6 이상이 필요합니다.

```sh
scripts/gen-chords.sh /path/to/sebeol-type
```

## 라이선스

[CC BY-SA 4.0](LICENSE). 세모이 배열과 결합 규칙은 신세기 님의 [Semo-e_keyboard](https://github.com/Sinseiki/Semo-e_keyboard)(CC BY-SA 4.0)를 바탕으로 했으므로 같은 조건으로 공개합니다.
