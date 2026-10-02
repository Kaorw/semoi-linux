// 세모이 모아치기 엔진 — sebeol-type src/engine/semoe.ts (chordText, resolveTokens)를 옮긴 것
#include "semoi.h"

#include <algorithm>
#include <array>
#include <optional>

namespace semoi {
namespace {

// 낱자 표기: 앞 글자로 자리를 나눈다. c=초성, v=중성, j=종성. "v." 는 . 키의 가상 ㅗ
using Tok = std::string;

struct KeyDef {
    int evdev;
    const char *tok;   // 한글 낱자 (없으면 nullptr)
    const char *punct; // 한글 밖의 글자 (없으면 nullptr)
};

// 배열 차례 = 계산 차례. sebeol-type 의 SEMOE 표와 같은 차례로 둔다.
constexpr KeyDef KEYS[] = {
    // 오른손: 첫소리
    {21, "cㅁ", nullptr}, // Y
    {22, "cㄴ", nullptr}, // U
    {23, "cㄷ", nullptr}, // I
    {24, "cㅂ", nullptr}, // O
    {35, "cㅎ", nullptr}, // H
    {36, "cㅇ", nullptr}, // J
    {37, "cㄱ", nullptr}, // K
    {38, "cㅈ", nullptr}, // L
    {49, "cㅅ", nullptr}, // N
    {50, "cㄹ", nullptr}, // M
    {39, "jㅆ", nullptr}, // ;  받침 ㅆ (다른 받침과 모아 ㅊ ㅍ ㅈ ㅀ ㅎ ㄷ ㅋ)
    {52, "v.", nullptr},  // .  겹홀소리용 ㅗ
    // 왼손: 받침
    {16, "jㅅ", nullptr}, // Q
    {17, "jㅂ", nullptr}, // W
    {18, "jㄹ", nullptr}, // E
    {30, "jㅇ", nullptr}, // A
    {31, "jㄴ", nullptr}, // S
    {44, "jㅁ", nullptr}, // Z
    {45, "jㄱ", nullptr}, // X
    // 왼손: 홀소리
    {19, "vㅓ", nullptr}, // R
    {20, "vㅕ", nullptr}, // T
    {32, "vㅣ", nullptr}, // D
    {33, "vㅏ", nullptr}, // F
    {34, "vㅡ", nullptr}, // G
    {46, "vㅔ", nullptr}, // C
    {47, "vㅗ", nullptr}, // V
    {48, "vㅜ", nullptr}, // B
    // 한글 밖의 글자 (OHI 세모이, 기호 확장 끔)
    {25, nullptr, "."}, // P
    {51, nullptr, ","}, // ,
    {2, nullptr, "1"},  {3, nullptr, "2"},  {4, nullptr, "3"}, {5, nullptr, "4"},
    {6, nullptr, "5"},  {7, nullptr, "6"},  {8, nullptr, "7"}, {9, nullptr, "8"},
    {10, nullptr, "9"}, {11, nullptr, "0"},
};

struct ShiftDef {
    int evdev;
    const char *text;
};
constexpr ShiftDef SHIFT_KEYS[] = {{53, "?"}, {2, "!"}, {39, ":"}};

struct Rule {
    const char *a, *b, *r;
};
// 모아치기 결합 규칙: 낱자 차례를 따지지 않고 위에서부터 먼저 맞는 규칙을 쓴다
const Rule RULES[] = {
#include "rules.inc"
};

const KeyDef *find(int evdev) {
    for (const auto &k : KEYS) {
        if (k.evdev == evdev) {
            return &k;
        }
    }
    return nullptr;
}

int order(int evdev) {
    for (size_t i = 0; i < std::size(KEYS); i++) {
        if (KEYS[i].evdev == evdev) {
            return static_cast<int>(i);
        }
    }
    return static_cast<int>(std::size(KEYS));
}

char cat(const Tok &t) { return t[0]; }
std::string jamo(const Tok &t) { return t.substr(1); }

/** 규칙 찾기(차례를 따짐, 표 순서대로) */
std::optional<Tok> ruleFor(const Tok &a, const Tok &b) {
    for (const auto &r : RULES) {
        if (a == r.a && b == r.b) {
            return Tok(r.r);
        }
    }
    return std::nullopt;
}

struct Resolved {
    std::vector<Tok> c, v, j;
    bool bad = false;
};

/**
 * 낱자 묶음 → 자리별로 모은 낱자
 * OHI 세모이 처리 차례를 따른다: 결합 규칙 표를 위에서부터 훑어 누른 낱자에서 짝을 찾아 합치고,
 * 남은 낱자는 뒤에 붙인 다음, 자리(초·중·종)별로 앞에서부터 이어 합친다.
 */
Resolved resolveTokens(std::vector<Tok> pressed) {
    std::vector<Tok> out;
    for (const auto &r : RULES) {
        auto i = std::find(pressed.begin(), pressed.end(), r.a);
        if (i == pressed.end()) {
            continue;
        }
        std::vector<Tok> rest(pressed.begin(), i);
        rest.insert(rest.end(), i + 1, pressed.end());
        auto k = std::find(rest.begin(), rest.end(), r.b);
        if (k == rest.end()) {
            continue;
        }
        rest.erase(k);
        pressed = std::move(rest);
        out.emplace_back(r.r);
    }
    out.insert(out.end(), pressed.begin(), pressed.end());

    Resolved res;
    for (const auto &t : out) {
        if (t == "x") {
            res.bad = true;
            continue;
        }
        auto &list = cat(t) == 'c' ? res.c : cat(t) == 'v' ? res.v : res.j;
        std::optional<Tok> comb;
        if (!list.empty()) {
            comb = ruleFor(list.back(), t);
        }
        if (comb && *comb != "x") {
            list.back() = *comb;
        } else {
            list.push_back(t);
        }
    }
    // . 키의 가상 ㅗ 가 홀로 남으면 보통 ㅗ 로 들어간다
    for (auto &t : res.v) {
        if (t == "v.") {
            t = "vㅗ";
        }
    }
    return res;
}

const std::array<const char *, 19> CHO = {"ㄱ", "ㄲ", "ㄴ", "ㄷ", "ㄸ", "ㄹ", "ㅁ",
                                          "ㅂ", "ㅃ", "ㅅ", "ㅆ", "ㅇ", "ㅈ", "ㅉ",
                                          "ㅊ", "ㅋ", "ㅌ", "ㅍ", "ㅎ"};
const std::array<const char *, 21> JUNG = {"ㅏ", "ㅐ", "ㅑ", "ㅒ", "ㅓ", "ㅔ", "ㅕ",
                                           "ㅖ", "ㅗ", "ㅘ", "ㅙ", "ㅚ", "ㅛ", "ㅜ",
                                           "ㅝ", "ㅞ", "ㅟ", "ㅠ", "ㅡ", "ㅢ", "ㅣ"};
const std::array<const char *, 28> JONG = {
    "",   "ㄱ", "ㄲ", "ㄳ", "ㄴ", "ㄵ", "ㄶ", "ㄷ", "ㄹ", "ㄺ",
    "ㄻ", "ㄼ", "ㄽ", "ㄾ", "ㄿ", "ㅀ", "ㅁ", "ㅂ", "ㅄ", "ㅅ",
    "ㅆ", "ㅇ", "ㅈ", "ㅊ", "ㅋ", "ㅌ", "ㅍ", "ㅎ"};

template <size_t N>
int indexOf(const std::array<const char *, N> &table, const std::string &s) {
    for (size_t i = 0; i < N; i++) {
        if (s == table[i]) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::string utf8(char32_t cp) {
    std::string s;
    s += static_cast<char>(0xE0 | (cp >> 12));
    s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    s += static_cast<char>(0x80 | (cp & 0x3F));
    return s;
}

std::optional<std::string> compose(const std::string &cho,
                                   const std::string &jung,
                                   const std::string &jong) {
    int a = indexOf(CHO, cho), b = indexOf(JUNG, jung), c = indexOf(JONG, jong);
    if (a < 0 || b < 0 || c < 0) {
        return std::nullopt;
    }
    return utf8(0xAC00 + (a * 21 + b) * 28 + c);
}

} // namespace

bool isChordKey(int evdev) { return evdev == KEY_SPACE || find(evdev); }

bool isJamoKey(int evdev) {
    const auto *k = find(evdev);
    return k && k->tok;
}

std::string shiftText(int evdev) {
    for (const auto &k : SHIFT_KEYS) {
        if (k.evdev == evdev) {
            return k.text;
        }
    }
    return {};
}

std::string chordText(const std::vector<int> &evdevCodes) {
    std::vector<int> codes = evdevCodes;
    std::stable_sort(codes.begin(), codes.end(),
                     [](int a, int b) { return order(a) < order(b); });

    std::vector<Tok> toks;
    std::string punct, space;
    for (int code : codes) {
        if (code == KEY_SPACE) {
            space = " ";
        } else if (const auto *k = find(code)) {
            if (k->tok) {
                toks.emplace_back(k->tok);
            } else {
                punct += k->punct;
            }
        }
    }
    if (toks.empty()) {
        return punct + space;
    }

    auto r = resolveTokens(std::move(toks));
    if (!r.bad && r.c.size() == 1 && r.v.size() == 1 && r.j.size() <= 1) {
        if (auto s = compose(jamo(r.c[0]), jamo(r.v[0]),
                             r.j.empty() ? "" : jamo(r.j[0]))) {
            return *s + punct + space;
        }
    }
    // 한 글자가 안 되면 낱자를 늘어놓는다
    std::string text;
    for (const auto *list : {&r.c, &r.v, &r.j}) {
        for (const auto &t : *list) {
            text += jamo(t);
        }
    }
    return text + punct + space;
}

} // namespace semoi
