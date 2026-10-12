#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

// Stage 1: demonstrate that the Geode hook and custom button work.
// Stage 2 will add recording attempts and training, not included yet.
class $modify(NeuralAutoMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto sprite = CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png");
        if (!sprite) {
            return true;
        }

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(NeuralAutoMenuLayer::onNeuralAuto)
        );

        auto menu = CCMenu::create();
        menu->addChild(button);

        const auto screen = CCDirector::sharedDirector()->getWinSize();
        menu->setPosition({screen.width - 45.f, screen.height - 45.f});
        this->addChild(menu, 20);
        return true;
    }

    void onNeuralAuto(CCObject*) {
        FLAlertLayer::create(
            "GD Neural Auto",
            "The mod works! Learning AI is the next development stage.",
            "OK"
        )->show();
    }
};
