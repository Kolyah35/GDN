#include "AIMenu.hpp"
#include <Geode/Geode.hpp>
#include <rapidjson/document.h>

#undef GetObject

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

    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);

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

    this->m_mainLayer->setScale(0.1f);
    this->m_mainLayer->runAction(cocos2d::CCEaseElasticOut::create(cocos2d::CCScaleTo::create(0.5f, 1.0), 0.6f));

    return true;
}

void AIMenu::onClose(cocos2d::CCObject*) {
    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto editorUI = (EditorUI*)levelEditorLayer->getChildren()->objectAtIndex(7);
    auto okBtn = editorUI->getChildByIDRecursive("kolyah35.gdn/okBtn");
    auto gm = GameManager::sharedState();

    CCObject* obj;
    CCARRAY_FOREACH(m_invisibleArray, obj) {
        CCNode* node = dynamic_cast<CCNode*>(obj);
        node->setVisible(true);
    }

    // gm->setGameVariable("0003", m_swipeEnabled);
    if(!gm->getGameVariable("0003")) {
        auto menu = (CCMenu*)editorUI->getChildren()->objectAtIndex(5);
        auto swipeBtn = menu->getChildren()->objectAtIndex(3);
        editorUI->toggleSwipe(swipeBtn);
    }

    // editorUI->m_selectedMode = m_currentMode;
    m_aiSelectObjects = false;
    m_ignoreLayer = false;
    m_selectedRect = {0, 0, 0, 0};
    m_aiMode = false;

    // editorUI->m_editButtonBar->setPositionY(0);

    if(okBtn) okBtn->removeFromParent();

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
    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();	
	
    // auto menu = CCMenu::create();
    
    auto selectBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Select area"), this, cocos2d::SEL_MenuHandler(&AIMenu::selectAreaClicked));
    selectBtn->setPosition(this->m_buttonMenu->convertToNodeSpace({winSize.width / 2, 235}));
	this->m_buttonMenu->addChild(selectBtn);

	auto textInputBG = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
	textInputBG->setContentSize({260, 100});
	textInputBG->setPosition(winSize / 2);
	textInputBG->setOpacity(125);
	this->m_mainLayer->addChild(textInputBG);

    auto textArea = TextArea::create("", "chatFont.fnt", 1.0f, 230, {0.5f, 0.5f}, 10.0f, true);
    textArea->setID("kolyah35.gdn/textArea");
    textArea->setPosition(this->m_buttonMenu->convertToNodeSpace(winSize / 2));
	this->m_buttonMenu->addChild(textArea);

	auto textInput = CCTextInputNode::create(260, 100, "Your prompt...", "chatFont.fnt");
    textInput->setAllowedChars(" abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,-!?:;)(/\\\"'`*=+-_%[]<>|@&^#{}%$~");
    textInput->addTextArea(textArea);
	textInput->setPosition(this->m_buttonMenu->convertToNodeSpace(winSize / 2));
    textInput->setLabelPlaceholderColor({128, 128, 128});
    textInput->setDelegate(this);
	this->m_buttonMenu->addChild(textInput);

	auto sendBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Send"), this, SEL_MenuHandler(&AIMenu::onSendBtn));
	sendBtn->setPosition(this->m_buttonMenu->convertToNodeSpace({winSize.width / 2, 85}));
	this->m_buttonMenu->addChild(sendBtn);

	// menu->setPosition(winSize / 2);
	// this->m_mainLayer->addChild(menu);
    // menu->setID("kolyah35.gdn/menu");
}

void AIMenu::selectAreaClicked(cocos2d::CCObject*) {
    auto menu = (cocos2d::CCMenu*)this->getChildByIDRecursive("kolyah35.gdn/menu");
    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto editorUI = (EditorUI*)levelEditorLayer->getChildren()->objectAtIndex(7);

    if(!m_aiMode){
        m_invisibleArray->removeAllObjects();

        for(int i = 0; i < editorUI->getChildrenCount(); i++) {
            CCNode* node = dynamic_cast<CCNode*>(editorUI->getChildren()->objectAtIndex(i));

            if(node != nullptr && node->isVisible()){
                m_invisibleArray->addObject(dynamic_cast<CCObject*>(node));
                node->setVisible(false);
            }
        }

        auto layerNum = (CCMenu*)editorUI->getChildren()->objectAtIndex(0);
        layerNum->setVisible(true);

        auto slider = (Slider*)editorUI->getChildren()->objectAtIndex(4);
        slider->setVisible(true);

        auto zoomMenu = (CCMenu*)editorUI->getChildren()->objectAtIndex(5);
        zoomMenu->setVisible(true);

        for(int i = 0; i < zoomMenu->getChildrenCount(); i++) {
            if(i == 13 || i == 14) continue;

            auto obj = (CCNode*)zoomMenu->getChildren()->objectAtIndex(i);
            m_invisibleArray->addObject(obj);
            obj->setVisible(false);
        }

        auto btnsMenu = (CCMenu*)editorUI->getChildren()->objectAtIndex(25);
        btnsMenu->setVisible(true);

        for(int i = 0; i < btnsMenu->getChildrenCount(); i++) {
            if(i >= 14 && i <= 16) continue;

            auto obj = (CCNode*)btnsMenu->getChildren()->objectAtIndex(i);
            m_invisibleArray->addObject(obj);
            obj->setVisible(false);
        }

        // editorUI->m_editButtonBar->setPositionY(-200);

        auto gm = GameManager::sharedState();

        m_swipeEnabled = gm->getGameVariable("0003");
        m_currentMode = editorUI->m_selectedMode;
        m_aiMode = true;

        // gm->setGameVariable("0003", true);
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
    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto editorUI = (EditorUI*)levelEditorLayer->getChildren()->objectAtIndex(7);
    auto aiMenu = AIMenu::create(300, 200);

    aiMenu->setID("kolyah35.gdn/AIMenu");
    editorUI->getParent()->addChild(aiMenu);

    m_aiSelectObjects = true;
    auto objectsInAIRect = levelEditorLayer->objectsInRect(m_selectedRect, m_ignoreLayer);

    CCObject* obj;
    CCARRAY_FOREACH(objectsInAIRect, obj) {
        GameObject* object = dynamic_cast<GameObject*>(obj);
        levelEditorLayer->removeObject(object, false);
        log::info("delete");
    }
}

void AIMenu::onSendBtn(CCObject*) {
    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto editorUI = (EditorUI*)levelEditorLayer->getChildren()->objectAtIndex(7);
    
    std::ifstream t(Mod::get()->getResourcesDir() / "testjson.json");
    std::stringstream buffer;
    buffer << t.rdbuf();

    rapidjson::Document blocks;
    blocks.Parse(buffer.str().c_str());

    for(auto& block : blocks["blocks"].GetArray()) {
        log::info("AAA");
        auto obj = block.GetObject();
        
        auto ID = obj["ID"].GetInt();
        
        auto pos = obj["pos"].GetArray();
        auto x = pos[0].GetFloat();
        auto y = pos[1].GetFloat();
        
        auto color = obj["color"].GetArray();
        unsigned char r = color[0].GetUint();
        unsigned char g = color[1].GetUint();
        unsigned char b = color[2].GetUint();

        auto scaleX = obj["scaleX"].GetFloat();
        auto scaleY = obj["scaleY"].GetFloat();

        auto rotation = obj["rotation"].GetInt();

        // auto gameObj = editorUI->createObject(ID, cocos2d::CCPoint {m_selectedRect.origin.x + x, m_selectedRect.origin.y + y});
        auto gameObj = levelEditorLayer->createObject(ID, cocos2d::CCPoint {m_selectedRect.origin.x + x, m_selectedRect.origin.y + y}, true);
        gameObj->setObjectColor({r, g, b});
        gameObj->updateCustomScaleX(scaleX);
        gameObj->updateCustomScaleX(scaleY);
        gameObj->setRotation(rotation);

        log::info("place");

        // levelEditorLayer->addSpecial(gameObj);
    }
}