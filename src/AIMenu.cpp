#include "AIMenu.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

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
    auto editorUI = (EditorUI*)this->getParent()->getChildByID("EditorUI");
    auto okBtn = editorUI->getChildByIDRecursive("kolyah35.gdn/okBtn");

    CCObject* obj;
    CCARRAY_FOREACH(m_invisibleArray, obj) {
        CCNode* node = dynamic_cast<CCNode*>(obj);
        node->setVisible(true);
    }

    editorUI->m_swipeEnabled = m_swipeEnabled;
    editorUI->m_selectedMode = m_currentMode;
    m_aiMode = false;

    editorUI->m_editButtonBar->setPositionY(0);

    if(okBtn != nullptr)
        okBtn->removeFromParent();

    m_drawbox->clear();

    this->setKeyboardEnabled(false);
    this->removeFromParentAndCleanup(true);
}

void AIMenu::textChanged(CCTextInputNode* p0) {
    auto textArea = (TextArea*)this->getChildByIDRecursive("kolyah35.gdn/textArea");
    std::string str = p0->getString();

    if(str.size() != 0){
        textArea->setString(gd::string(str));
        textArea->m_label->setColor({255, 255, 255});
    }else{
        textArea->setString("Your prompt...");
        textArea->m_label->setColor({150, 150, 150});
    }
}

void AIMenu::setup() {
	this->setZOrder(101);

	auto menu = cocos2d::CCMenu::create();
    menu->setID("kolyah35.gdn/menu");
    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();	

    auto selectBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Select area"), this, cocos2d::SEL_MenuHandler(&AIMenu::selectAreaClicked));
    selectBtn->setPosition(menu->convertToNodeSpace({winSize.width / 2, 235}));
	menu->addChild(selectBtn);

	auto textInputBG = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
	textInputBG->setContentSize({260, 100});
	textInputBG->setPosition(menu->convertToNodeSpace(winSize / 2));
	textInputBG->setOpacity(125);
	menu->addChild(textInputBG);

    auto textArea = TextArea::create("Your prompt...", "chatFont.fnt", 1.0f, 230, {0.5f, 0.5f}, 10.0f, true);
    textArea->setID("kolyah35.gdn/textArea");
    textArea->setPosition(menu->convertToNodeSpace(winSize / 2));
	menu->addChild(textArea);

	auto textInput = CCTextInputNode::create(260, 100, "Your prompt...", "chatFont.fnt");
	textInput->setPosition(menu->convertToNodeSpace(winSize / 2));
    textInput->getPlaceholderLabel()->setVisible(false);
    textInput->setDelegate(this);
	menu->addChild(textInput);

	auto sendBtn = ButtonSprite::create("Send");
	sendBtn->setPosition(menu->convertToNodeSpace({winSize.width / 2, 85}));
	menu->addChild(sendBtn);

	menu->setPosition(winSize / 2);
	this->m_mainLayer->addChild(menu);
}

void AIMenu::selectAreaClicked(cocos2d::CCObject*) {
    auto menu = (cocos2d::CCMenu*)this->getChildByIDRecursive("kolyah35.gdn/menu");
    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
    auto editorUI = (EditorUI*)CCScene::get()->getChildByIDRecursive("EditorUI");
    

    if(!m_aiMode){
        m_invisibleArray->removeAllObjects();

        for(int i = 0; i < editorUI->getChildrenCount(); i++) {
            CCNode* node = dynamic_cast<CCNode*>(editorUI->getChildren()->objectAtIndex(i));

            if(node != nullptr && node->isVisible()){
                m_invisibleArray->addObject(dynamic_cast<CCObject*>(node));
                node->setVisible(false);
            }
        }

        auto slider = (Slider*)editorUI->getChildByID("position-slider");
        slider->setVisible(true);

        auto zoomMenu = (cocos2d::CCMenu*)editorUI->getChildByID("zoom-menu");
        zoomMenu->setVisible(true);

        auto layerMenu = (cocos2d::CCMenu*)editorUI->getChildByID("layer-menu");
        layerMenu->setVisible(true);

        editorUI->m_editButtonBar->setPositionY(-200);

        m_swipeEnabled = editorUI->m_swipeEnabled;
        m_currentMode = editorUI->m_selectedMode;
        m_aiMode = true;

        editorUI->m_swipeEnabled = true;
        editorUI->m_selectedMode = 3;

        auto okBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("OK"), this, cocos2d::SEL_MenuHandler(&AIMenu::okButtonClicked));
        okBtn->setID("kolyah35.gdn/okBtn");
        okBtn->setPosition(zoomMenu->convertToNodeSpace({winSize.width / 2, 20}));
        zoomMenu->addChild(okBtn);
    }

    this->setKeyboardEnabled(false);
    this->removeFromParentAndCleanup(true);
}

void AIMenu::okButtonClicked(CCObject*) {
    auto editorUI = (EditorUI*)CCScene::get()->getChildByIDRecursive("EditorUI");
    auto aiMenu = AIMenu::create(300, 200);

    aiMenu->setID("kolyah35.gdn/AIMenu");
    editorUI->getParent()->addChild(aiMenu);
}