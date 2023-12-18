#include "AIMenu.hpp"

AIMenu* AIMenu::create(float w, float h, const char* spr) {
    auto pRet = new(std::nothrow) AIMenu();
    if(pRet && pRet->init(w, h, spr)) {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
}

bool AIMenu::init(float w, float h, const char* spr) {
    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
    this->m_layerSize = cocos2d::CCSize {w, h};

    if (!this->initWithColor({0, 0, 0, 125}))
        return false;
    this->m_mainLayer = cocos2d::CCLayer::create();
    this->addChild(this->m_mainLayer);

    auto bg = cocos2d::extension::CCScale9Sprite::create(spr, {0.0f, 0.0f, 80.0f, 80.0f});
    bg->setContentSize(this->m_layerSize);
    bg->setPosition(winSize.width / 2, winSize.height / 2);
    this->m_mainLayer->addChild(bg);

    this->m_buttonMenu = cocos2d::CCMenu::create();
    this->m_mainLayer->addChild(this->m_buttonMenu);

    setup();

    auto closeSpr = cocos2d::CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    closeSpr->setScale(.8f);

     auto closeBtn = CCMenuItemSpriteExtra::create(
         closeSpr,
         this,
         (cocos2d::SEL_MenuHandler)&AIMenu::onClose
     );
     closeBtn->setUserData(reinterpret_cast<void*>(this));

    this->m_buttonMenu->addChild(closeBtn);

    closeBtn->setPosition(-w / 2, h / 2);

    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);

    this->m_mainLayer->setScale(0.1f);
    this->m_mainLayer->runAction(cocos2d::CCEaseElasticOut::create(cocos2d::CCScaleTo::create(0.5f, 1.0), 0.6f));

    return true;
}

void AIMenu::onClose(cocos2d::CCObject*) {
    this->setKeyboardEnabled(false);
    this->removeFromParentAndCleanup(true);
}