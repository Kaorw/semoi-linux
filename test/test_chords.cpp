// sebeol-type(TS) 세모이 엔진이 낸 결과(chords.tsv)와 C++ 엔진을 전수 대조한다.
// chords.tsv: 배열 차례로 정렬한 evdev 키코드 묶음 \t 들어갈 글 (scripts/gen-chords.ts 로 생성)
#include "semoi.h"

#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char **argv) {
    std::ifstream in(argc > 1 ? argv[1] : "chords.tsv");
    if (!in) {
        std::cerr << "chords.tsv 를 열 수 없음\n";
        return 2;
    }
    std::string line;
    int total = 0, fail = 0;
    while (std::getline(in, line)) {
        auto tab = line.find('\t');
        std::vector<int> codes;
        std::stringstream ss(line.substr(0, tab));
        for (std::string c; std::getline(ss, c, ',');) {
            codes.push_back(std::stoi(c));
        }
        auto want = line.substr(tab + 1);
        auto got = semoi::chordText(codes);
        total++;
        if (got != want) {
            if (++fail <= 20) {
                std::cerr << "FAIL " << line.substr(0, tab) << ": want '" << want
                          << "' got '" << got << "'\n";
            }
        }
    }
    std::cout << total - fail << "/" << total << " ok\n";
    return fail ? 1 : 0;
}
