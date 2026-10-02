# fcitx5-semoi

세모이(세벌식 모아치기 e) 자판을 Omarchy의 Fcitx5에서 쓰기 위한 애드온입니다. 한 글자의 초성·중성·종성 키를 한꺼번에 눌렀다가 모두 떼는 순간 그 글자가 들어갑니다.

같은 컴퓨터의 두벌식 순아래(`~/Work/dubeolsik-sunarae`, `fcitx5-hangul`)와 같은 입력기 그룹에 두고 단축키로 바꿔 씁니다.

| 단축키 | 동작 |
|---|---|
| Shift+Space | 영문 ↔ 마지막에 쓴 한글 입력기 |
| Ctrl+Shift+Space | 두벌식 순아래 ↔ 세모이 (영문은 건너뜀) |

## 자판

배열과 결합 규칙은 `~/Work/sebeol-type`(신세벌 타자)의 `src/layout/semoe.ts`를 그대로 옮겼습니다. 그쪽 연습기에서 익힌 조합이 여기서도 똑같이 들어갑니다. 원 배열은 [Sinseiki/Semo-e_keyboard](https://github.com/Sinseiki/Semo-e_keyboard)(CC BY-SA 4.0)입니다.

- 오른손 `Y U I O · H J K L · N M` = 첫소리 ㅁ ㄴ ㄷ ㅂ · ㅎ ㅇ ㄱ ㅈ · ㅅ ㄹ
- 왼손 `R T · D F G · C V B` = 홀소리 ㅓ ㅕ · ㅣ ㅏ ㅡ · ㅔ ㅗ ㅜ
- 왼손 `Q W E · A S · Z X` = 받침 ㅅ ㅂ ㄹ · ㅇ ㄴ · ㅁ ㄱ
- `;` = 받침 ㅆ(다른 받침과 모아 ㅊ ㅍ ㅈ ㅀ ㅎ ㄷ ㅋ), `.` = 겹홀소리용 ㅗ
- `P` = `.`, `,` = `,`, 숫자는 그대로. Shift+`/` `1` `;` = `?` `!` `:`
- 글자 키와 스페이스를 함께 누르면 띄어쓰기까지 한 번에 들어갑니다. 따로 누른 스페이스는 그냥 띄어쓰기입니다.
- Ctrl·Alt·Super와 함께 누른 키, 배열에 없는 키는 입력기가 건드리지 않습니다.

키는 물리 키코드(evdev)로 판정하므로 시스템 키보드 배치와 상관없이 같은 자리에서 같은 글자가 나옵니다.

빠진 것: 약어, 옛한글, 기호 확장, 이어치기 전용 모드.

## 빌드·설치

```sh
cd packaging && makepkg -si      # 빌드 + 테스트 + 설치
../scripts/enable-fcitx5.sh      # 입력기 그룹에 세모이 추가, Ctrl+Shift+Space 단축키
```

`enable-fcitx5.sh`는 `omarchy-fcitx5.service`를 잠깐 멈추고 `~/.config/fcitx5/profile`, `config`를 고친 뒤 다시 띄웁니다. 고치기 전 파일은 `*.bak-semoi`로 남깁니다.

## 테스트

```sh
cmake -B build -S . && cmake --build build && ctest --test-dir build --output-on-failure
```

- `chords`: 낱자 키 5개까지의 모든 묶음과 문장부호·스페이스 조합 101,603개를 sebeol-type 엔진 결과(`test/chords.tsv`)와 대조
- `engine`: 실제 fcitx5 `Instance`에 애드온을 올리고 누름/뗌 이벤트로 가·과·각·있·좋·한·글, 자동 반복, 스페이스, Shift, 단축키 통과를 확인

sebeol-type의 규칙이 바뀌면 다시 만듭니다.

```sh
python3 scripts/gen-rules.py ~/Work/sebeol-type/src/layout/semoe.ts > src/rules.inc
# test/chords.tsv: scripts/gen-chords.ts 를 sebeol-type src 복사본(상대 import 에 .ts 붙임)과 함께 node 로 실행
```

## 라이선스

[CC BY-SA 4.0](LICENSE). 세모이 배열과 결합 규칙은 신세기 님의 [Semo-e_keyboard](https://github.com/Sinseiki/Semo-e_keyboard)(CC BY-SA 4.0)를 바탕으로 했으므로 같은 조건으로 공개합니다.
