// 세모이(세벌식 모아치기 e) 모아치기 엔진 — fcitx 와 무관한 순수 로직
//
// 배열과 결합 규칙은 ~/Work/sebeol-type/src/layout/semoe.ts 를 그대로 옮겼다.
// 원 배열: https://github.com/Sinseiki/Semo-e_keyboard (CC BY-SA 4.0)
#pragma once

#include <string>
#include <vector>

namespace semoi {

// 물리 키는 evdev 키코드(linux/input-event-codes.h 의 KEY_*)로 다룬다.
constexpr int KEY_SPACE = 57;

/** 이 키가 모아치기 묶음에 들어가는 키인가 (낱자·문장부호·숫자·스페이스) */
bool isChordKey(int evdev);

/** 이 키가 한글 낱자 키인가 */
bool isJamoKey(int evdev);

/** Shift 와 함께 눌렀을 때 들어갈 글 (없으면 빈 문자열) */
std::string shiftText(int evdev);

/**
 * 한꺼번에 누른 키 묶음 → 들어갈 글.
 * 한 글자가 되면 그 글자, 안 되면 낱자를 늘어놓는다. 문장부호·스페이스는 뒤에 붙는다.
 * 결과는 키 차례와 무관하도록 배열 차례로 정렬해서 계산한다.
 */
std::string chordText(const std::vector<int> &evdevCodes);

} // namespace semoi
