// fcitx5 세모이 입력기: 한꺼번에 누른 키를 모아 두었다가 모두 떼는 순간 한 글자로 넣는다.
#include "semoi.h"

#include <algorithm>
#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <fcitx/text.h>
#include <fcitx/userinterface.h>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/textformatflags.h>

namespace fcitx {
namespace {

// xkb 키코드 = evdev 키코드 + 8
constexpr int XKB_EVDEV_OFFSET = 8;

// 이 수식키가 눌려 있으면 단축키로 보고 입력기가 손대지 않는다
constexpr KeyStates SHORTCUT_STATES{KeyState::Ctrl,   KeyState::Alt,
                                    KeyState::Super,  KeyState::Super2,
                                    KeyState::Hyper,  KeyState::Hyper2,
                                    KeyState::Meta};

bool contains(const std::vector<int> &v, int x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}

class SemoiState : public InputContextProperty {
public:
    std::vector<int> held;  // 지금 눌려 있는 묶음 키
    std::vector<int> chord; // 이번 묶음에서 누른 키 전부

    void clear() {
        held.clear();
        chord.clear();
    }
};

class SemoiEngine : public InputMethodEngineV2 {
public:
    explicit SemoiEngine(Instance *instance)
        : instance_(instance),
          factory_([](InputContext &) { return new SemoiState; }) {
        instance_->inputContextManager().registerProperty("semoiState",
                                                          &factory_);
    }

    void keyEvent(const InputMethodEntry &, KeyEvent &event) override {
        auto *ic = event.inputContext();
        auto *st = ic->propertyFor(&factory_);
        const Key &key = event.rawKey();
        const int evdev = key.code() - XKB_EVDEV_OFFSET;

        if (event.isRelease()) {
            auto it = std::find(st->held.begin(), st->held.end(), evdev);
            if (it == st->held.end()) {
                return; // 묶음에 들지 않은 키의 뗌은 그대로 보낸다
            }
            st->held.erase(it);
            event.filterAndAccept();
            if (st->held.empty()) {
                commit(ic, st);
            }
            return;
        }

        if (contains(st->held, evdev)) {
            event.filterAndAccept(); // 누르고 있는 키의 자동 반복
            return;
        }

        const auto states = key.states();
        if (states.testAny(SHORTCUT_STATES)) {
            commit(ic, st);
            return;
        }

        if (states.test(KeyState::Shift)) {
            commit(ic, st);
            auto text = semoi::shiftText(evdev);
            if (!text.empty()) {
                ic->commitString(text);
                event.filterAndAccept();
            }
            return;
        }

        if (!semoi::isChordKey(evdev)) {
            commit(ic, st);
            return;
        }

        // 묶음 밖에서 누른 스페이스는 그냥 띄어쓰기
        if (evdev == semoi::KEY_SPACE && st->held.empty()) {
            return;
        }

        st->held.push_back(evdev);
        st->chord.push_back(evdev);
        event.filterAndAccept();
        showPreedit(ic, st);
    }

    void reset(const InputMethodEntry &, InputContextEvent &event) override {
        auto *ic = event.inputContext();
        ic->propertyFor(&factory_)->clear();
        clearPreedit(ic);
    }

    void deactivate(const InputMethodEntry &entry,
                    InputContextEvent &event) override {
        reset(entry, event);
    }

private:
    void commit(InputContext *ic, SemoiState *st) {
        if (st->chord.empty()) {
            return;
        }
        auto text = semoi::chordText(st->chord);
        st->clear();
        clearPreedit(ic);
        if (!text.empty()) {
            ic->commitString(text);
        }
    }

    void showPreedit(InputContext *ic, SemoiState *st) {
        Text text(semoi::chordText(st->chord), TextFormatFlag::Underline);
        text.setCursor(text.textLength());
        setPreedit(ic, std::move(text));
    }

    void clearPreedit(InputContext *ic) { setPreedit(ic, Text()); }

    void setPreedit(InputContext *ic, Text text) {
        auto &panel = ic->inputPanel();
        if (ic->capabilityFlags().test(CapabilityFlag::Preedit)) {
            panel.setClientPreedit(std::move(text));
            ic->updatePreedit();
        } else {
            panel.setPreedit(std::move(text));
        }
        ic->updateUserInterface(UserInterfaceComponent::InputPanel);
    }

    Instance *instance_;
    FactoryFor<SemoiState> factory_;
};

class SemoiEngineFactory : public AddonFactory {
public:
    AddonInstance *create(AddonManager *manager) override {
        return new SemoiEngine(manager->instance());
    }
};

} // namespace
} // namespace fcitx

FCITX_ADDON_FACTORY_V2(semoi, fcitx::SemoiEngineFactory);
