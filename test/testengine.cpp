// 실제 fcitx5 Instance 에 세모이 애드온을 올리고, 누름/뗌 이벤트를 보내 끝까지 확인한다.
#include <fcitx-utils/eventdispatcher.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/testing.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputmethodgroup.h>
#include <fcitx/inputmethodmanager.h>
#include <fcitx/instance.h>
#include "testfrontend_public.h"

using namespace fcitx;

namespace {

// evdev 키코드 (xkb 키코드는 +8)
enum : int {
    Q = 16, E = 18, D = 32, F = 33, G = 34, H = 35, J = 36, K = 37,
    S = 31, V = 47, X = 45, L = 38, SEMI = 39, DOT = 52, SLASH = 53, SPACE = 57,
};

Key key(int evdev, KeyStates states = KeyStates()) {
    // 키심은 판정에 쓰지 않으므로 아무거나 둔다. 엔진은 물리 키코드만 본다.
    return Key(FcitxKey_a, states, evdev + 8);
}

struct Driver {
    AddonInstance *fe;
    ICUUID uuid;

    bool press(int k, KeyStates s = KeyStates()) {
        return fe->call<ITestFrontend::sendKeyEvent>(uuid, key(k, s), false);
    }
    bool release(int k, KeyStates s = KeyStates()) {
        return fe->call<ITestFrontend::sendKeyEvent>(uuid, key(k, s), true);
    }
    /** 키를 차례로 모두 누른 뒤, 주어진 차례로 모두 뗀다 */
    void chord(std::initializer_list<int> down, std::initializer_list<int> up) {
        for (int k : down) FCITX_ASSERT(press(k));
        for (int k : up) FCITX_ASSERT(release(k));
    }
    void expect(const char *s) {
        fe->call<ITestFrontend::pushCommitExpectation>(s);
    }
};

void scheduleEvent(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto group = instance->inputMethodManager().currentGroup();
        group.inputMethodList().clear();
        group.inputMethodList().push_back(InputMethodGroupItem("keyboard-us"));
        group.inputMethodList().push_back(InputMethodGroupItem("semoi"));
        group.setDefaultInputMethod("");
        instance->inputMethodManager().setGroup(group);

        auto *fe = instance->addonManager().addon("testfrontend");
        auto uuid = fe->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(fe->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("Control+space"), false));
        FCITX_ASSERT(instance->inputMethod(ic) == "semoi");

        Driver d{fe, uuid};

        // 가 = K F : 모두 떼는 순간 들어간다
        d.expect("가");
        d.chord({K, F}, {K, F});

        // 과 = K . F : 누르는 차례·떼는 차례와 무관
        d.expect("과");
        d.chord({F, DOT, K}, {DOT, F, K});

        // 키를 누르고 있는 동안의 자동 반복은 무시
        d.expect("각");
        FCITX_ASSERT(d.press(K));
        FCITX_ASSERT(d.press(F));
        FCITX_ASSERT(d.press(F));
        FCITX_ASSERT(d.press(X));
        FCITX_ASSERT(d.release(F));
        FCITX_ASSERT(d.release(K));
        FCITX_ASSERT(d.release(X));

        // 있 + 띄어쓰기 = J D ; Space
        d.expect("있 ");
        d.chord({J, D, SEMI, SPACE}, {SPACE, J, D, SEMI});

        // 묶음 밖의 스페이스는 입력기가 잡지 않는다
        FCITX_ASSERT(!d.press(SPACE));
        FCITX_ASSERT(!d.release(SPACE));

        // 좋 = L V ; S,  한 = H F S,  글 = K G E
        d.expect("좋");
        d.chord({L, V, SEMI, S}, {L, V, SEMI, S});
        d.expect("한");
        d.chord({H, F, S}, {H, F, S});
        d.expect("글");
        d.chord({K, G, E}, {E, G, K});

        // Shift+/ = ?
        d.expect("?");
        FCITX_ASSERT(d.press(SLASH, KeyState::Shift));

        // 단축키(Ctrl+Q)는 그대로 통과
        FCITX_ASSERT(!d.press(Q, KeyState::Ctrl));
        FCITX_ASSERT(!d.release(Q, KeyState::Ctrl));

        instance->deactivate();
    });
    instance->eventDispatcher().schedule([instance]() { instance->exit(); });
}

} // namespace

int main() {
    setupTestingEnvironmentPath(TESTING_BINARY_DIR, {"."}, {"testdata"});
    char arg0[] = "testengine";
    char arg1[] = "--disable=all";
    char arg2[] = "--enable=testim,testfrontend,semoi";
    char *argv[] = {arg0, arg1, arg2};
    fcitx::Log::setLogRule("default=5");
    Instance instance(FCITX_ARRAY_SIZE(argv), argv);
    instance.addonManager().registerDefaultLoader(nullptr);
    scheduleEvent(&instance);
    instance.exec();
    return 0;
}
