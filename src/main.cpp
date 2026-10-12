#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include "Brain.hpp"

#include <algorithm>
#include <cstdint>
#include <string>

using namespace geode::prelude;

namespace {
constexpr float SlotSize = 10.f; // Horizontal coordinate bucket size.
Brain g_brain;
std::string g_levelKey;
bool g_trainRequested = false;
bool g_demoRequested = false;

std::string saveKey(const char* field) {
    return "level-" + g_levelKey + "-" + field;
}

void loadLevelData(GJGameLevel* level) {
    if (!level) return;
    // Keep the same key when restarting a given level, distinct between levels.
    g_levelKey = std::to_string(static_cast<std::uint32_t>(level->m_levelID));
    auto mod = Mod::get();
    g_brain.load(
        mod->getSavedValue<std::string>(saveKey("policy"), ""),
        static_cast<int>(mod->getSavedValue<std::int64_t>(saveKey("best"), 0)),
        mod->getSavedValue<bool>(saveKey("solved"), false)
    );
}

void saveLevelData() {
    if (g_levelKey.empty()) return;
    auto mod = Mod::get();
    mod->setSavedValue<std::string>(saveKey("policy"), g_brain.serializeBest());
    mod->setSavedValue<std::int64_t>(saveKey("best"), g_brain.bestSlot());
    mod->setSavedValue<bool>(saveKey("solved"), g_brain.solved());
}

void clearLevelData() {
    if (g_levelKey.empty()) return;
    g_brain.clear();
    saveLevelData();
}

CCMenuItemSpriteExtra* textButton(
    CCObject* owner,
    SEL_MenuHandler callback,
    const char* title,
    const char* bg,
    float scale = 0.52f
) {
    auto sprite = ButtonSprite::create(title, "bigFont.fnt", bg, 0.64f);
    sprite->setScale(scale);
    return CCMenuItemSpriteExtra::create(sprite, owner, callback);
}
} // namespace

class $modify(NeuralPlayLayer, PlayLayer) {
    struct Fields {
        bool ready = false;
        bool deathHandled = false;
        bool aiHolding = false;
        bool runTraining = false;
        bool runDemo = false;
        int maxSlot = 0;
        float spawnX = 0.f;
        unsigned frames = 0;
        CCLabelBMFont* hud = nullptr;
    };

    int positionSlot(float x) const {
        return std::clamp(
            static_cast<int>((x - m_fields->spawnX) / SlotSize),
            0, Brain::MaxSlots - 1
        );
    }

    bool supportedLevel() const {
        // Position-only search is not safe for checkpoints or dual input.
        return m_level && !m_isPracticeMode && !m_level->m_twoPlayerMode;
    }

    void releaseAI() {
        if (m_fields->aiHolding && m_player1) {
            m_player1->releaseButton(PlayerButton::Jump);
        }
        m_fields->aiHolding = false;
    }

    void beginNewRun() {
        m_fields->deathHandled = false;
        m_fields->aiHolding = false;
        m_fields->maxSlot = 0;
        m_fields->frames = 0;
        m_fields->spawnX = m_player1 ? m_player1->getPositionX() : 0.f;
        m_fields->runTraining = g_trainRequested && supportedLevel();
        m_fields->runDemo = g_demoRequested && !m_fields->runTraining && supportedLevel();
        g_brain.beginAttempt(m_fields->runTraining, m_fields->runDemo);
    }

    void updateHUD() {
        auto label = m_fields->hud;
        if (!label) return;
        const char* mode = m_fields->runTraining ? "TRAIN" :
                           m_fields->runDemo ? "DEMO" : "OFF";
        std::string status = std::string("NEURAL AUTO [") + mode + "]  Trials: " +
                             std::to_string(g_brain.attempts()) +
                             "  Best: " + std::to_string(g_brain.bestSlot() * 10) + "u";
        if (g_brain.solved()) status += "  SOLVED";
        if (!supportedLevel()) status = "NEURAL AUTO: no practice / dual support";
        label->setString(status.c_str());
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        g_trainRequested = false; // Safety: never auto-start in a new level.
        g_demoRequested = false;
        loadLevelData(level);
        auto label = CCLabelBMFont::create("NEURAL AUTO [OFF]", "bigFont.fnt");
        if (label) {
            label->setScale(0.36f);
            label->setAnchorPoint({0.f, 1.f});
            label->setPosition({12.f, CCDirector::sharedDirector()->getWinSize().height - 12.f});
            this->addChild(label, 100);
            m_fields->hud = label;
        }
        m_fields->ready = true;
        beginNewRun();
        updateHUD();
        return true;
    }

    void resetLevel() {
        if (m_fields->ready) releaseAI();
        PlayLayer::resetLevel();
        if (m_fields->ready) {
            beginNewRun();
            updateHUD();
        }
    }

    void update(float dt) {
        if (m_fields->ready && m_player1 && !m_fields->deathHandled && supportedLevel()) {
            const int slot = positionSlot(m_player1->getPositionX());
            m_fields->maxSlot = std::max(m_fields->maxSlot, slot);

            // The choice is made before the game's physics tick.
            if (m_fields->runTraining && g_trainRequested) {
                bool press = g_brain.desiredHold(slot);
                if (press != m_fields->aiHolding) {
                    if (press) m_player1->pushButton(PlayerButton::Jump);
                    else m_player1->releaseButton(PlayerButton::Jump);
                    m_fields->aiHolding = press;
                }
            } else {
                releaseAI();
                if (m_fields->runDemo && g_demoRequested) {
                    g_brain.observeDemo(slot, m_player1->buttonDown(PlayerButton::Jump));
                }
            }
        }
        PlayLayer::update(dt);
        if (m_fields->ready && ++m_fields->frames % 12 == 0) updateHUD();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (m_fields->ready && player == m_player1 && !m_fields->deathHandled) {
            m_fields->deathHandled = true;
            m_fields->maxSlot = std::max(m_fields->maxSlot, positionSlot(player->getPositionX()));
            if (g_brain.finishAttempt(m_fields->maxSlot, false)) saveLevelData();
        }
        PlayLayer::destroyPlayer(player, object);
    }

    void levelComplete() {
        if (m_fields->ready && !m_fields->deathHandled) {
            m_fields->deathHandled = true;
            if (g_brain.finishAttempt(m_fields->maxSlot, true)) saveLevelData();
            updateHUD();
        }
        PlayLayer::levelComplete();
    }

    void onQuit() {
        if (m_fields->ready) {
            releaseAI();
            m_fields->ready = false;
            g_trainRequested = false;
            g_demoRequested = false;
        }
        PlayLayer::onQuit();
    }
};

class $modify(NeuralPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto menu = CCMenu::create();
        auto size = CCDirector::sharedDirector()->getWinSize();
        menu->setPosition({size.width - 89.f, size.height - 75.f});
        this->addChild(menu, 100);

        auto train = textButton(this, menu_selector(NeuralPauseLayer::onTrain),
            g_trainRequested ? "Stop AI" : "Train AI", "GJ_button_01.png");
        auto demo = textButton(this, menu_selector(NeuralPauseLayer::onDemo),
            g_demoRequested ? "Stop Demo" : "Record Demo", "GJ_button_02.png");
        auto reset = textButton(this, menu_selector(NeuralPauseLayer::onForget),
            "Forget AI", "GJ_button_06.png");
        menu->addChild(train);
        menu->addChild(demo);
        menu->addChild(reset);
        train->setPosition({0.f, 0.f});
        demo->setPosition({0.f, -40.f});
        reset->setPosition({0.f, -80.f});
    }

    void onTrain(CCObject*) {
        g_trainRequested = !g_trainRequested;
        if (g_trainRequested) g_demoRequested = false;
        FLAlertLayer::create("Neural Auto",
            g_trainRequested ? "Training enabled. Restart the attempt to begin." :
                               "Training disabled. Restart the attempt to apply.",
            "OK")->show();
    }

    void onDemo(CCObject*) {
        g_demoRequested = !g_demoRequested;
        if (g_demoRequested) g_trainRequested = false;
        FLAlertLayer::create("Neural Auto",
            g_demoRequested ? "Demo recording enabled. Restart and play manually." :
                              "Demo recording disabled. Restart to apply.",
            "OK")->show();
    }

    void onForget(CCObject*) {
        clearLevelData();
        FLAlertLayer::create("Neural Auto",
            "This level's learned inputs were erased. Restart to apply.",
            "OK")->show();
    }
};

class $modify(NeuralMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        auto menu = CCMenu::create();
        auto size = CCDirector::sharedDirector()->getWinSize();
        menu->setPosition({size.width - 37.f, size.height - 37.f});
        auto button = textButton(this, menu_selector(NeuralMenuLayer::onHelp),
                                 "AI", "GJ_button_04.png", 0.65f);
        menu->addChild(button);
        this->addChild(menu, 20);
        return true;
    }

    void onHelp(CCObject*) {
        FLAlertLayer::create("GD Neural Auto v0.3",
            "Open a classic level, press Pause, then choose Train AI or Record Demo. "
            "Restart the attempt. The search remembers the best distance for each level. "
            "This is an experimental evolutionary bot, not a neural network.",
            "OK")->show();
    }
};
