#include "AIMenu.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <rapidjson/document.h>
#include "rapidjson/stringbuffer.h"
#include <rapidjson/writer.h>

#undef GetObject

using namespace geode::prelude;
using namespace rapidjson;

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

	// auto textInputBG = cocos2d::extension::CCScale9Sprite::create("square02_001.png");
	// textInputBG->setContentSize({260, 100});
	// textInputBG->setPosition(winSize / 2);
	// textInputBG->setOpacity(125);
	// this->m_mainLayer->addChild(textInputBG);

    // auto textArea = TextArea::create("", "chatFont.fnt", 1.0f, 230, {0.5f, 0.5f}, 10.0f, true);
    // textArea->setID("kolyah35.gdn/textArea");
    // textArea->setPosition(this->m_buttonMenu->convertToNodeSpace(winSize / 2));
	// this->m_buttonMenu->addChild(textArea);

	// auto textInput = CCTextInputNode::create(260, 100, "Your prompt...", "chatFont.fnt");
    // textInput->setAllowedChars(" abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,-!?:;)(/\\\"'`*=+-_%[]<>|@&^#{}%$~");
    // textInput->addTextArea(textArea);
	// textInput->setPosition(this->m_buttonMenu->convertToNodeSpace(winSize / 2));
    // textInput->setLabelPlaceholderColor({128, 128, 128});
    // textInput->setDelegate(this);
	// this->m_buttonMenu->addChild(textInput);

    auto textInput = geode::InputNode::create(260, "Your prompt...", "chatFont.fnt", " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,-!?:;)(/\\\"'`*=+-_%[]<>|@&^#{}%$~", 200);
    textInput->setID("kolyah35.gdn/textArea");
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
}

void AIMenu::onSendBtn(CCObject*) {
    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto editorUI = (EditorUI*)levelEditorLayer->getChildren()->objectAtIndex(7);
    auto am = GJAccountManager::sharedState();
    auto inputNode = (InputNode*)this->m_buttonMenu->getChildByID("kolyah35.gdn/textArea");

    m_aiSelectObjects = true;
    auto objectsInAIRect = levelEditorLayer->objectsInRect(m_selectedRect, m_ignoreLayer);

    Document data;
    data.SetObject();

    auto allocator = data.GetAllocator();

    Value userobj;
    userobj.SetObject();

    Value username;
    username.SetString(am->m_username.c_str(), am->m_username.size());
    userobj.AddMember("UserName", username, allocator);

    Value prompt;
    prompt.SetString(inputNode->getString().c_str(), inputNode->getString().length());
    userobj.AddMember("Prompt", prompt, allocator);

    data.AddMember("User", userobj, allocator);


    Value blocks;
    blocks.SetArray();

    for(int i = 0; i < objectsInAIRect->count(); i++) {
        auto object = dynamic_cast<GameObject*>(objectsInAIRect->objectAtIndex(i));
        Value obj;
        obj.SetObject();

        Value id;
        id.SetInt(object->m_objectID);
        obj.AddMember("ID", id, allocator);

        Value x;
        x.SetInt(object->getPositionX() - m_selectedRect.origin.x);
        obj.AddMember("X", x, allocator);

        Value y;
        y.SetInt(object->getPositionY() - m_selectedRect.origin.y);
        obj.AddMember("Y", y, allocator);

        Value z;
        z.SetInt(object->m_zOrder);
        obj.AddMember("Z", z, allocator);

        Value l;
        l.SetInt(object->m_editorLayer);
        obj.AddMember("L", l, allocator);
        
        Value scaleX;
        scaleX.SetFloat(object->m_scaleX);
        obj.AddMember("ScaleX", scaleX, allocator);
        
        Value scaleY;
        scaleY.SetFloat(object->m_scaleY);
        obj.AddMember("ScaleY", scaleY, allocator);
        
        Value color;
        color.SetArray();

        for(int i = 0; i < object->m_colorGroupCount; i++) {
            Value id;
            id.SetInt(object->m_colorGroups->at(i));
            color.PushBack(id, data.GetAllocator());
        }

        obj.AddMember("Color", color, allocator);

        Value groups;
        groups.SetArray();

        for(int i = 0; i < object->m_groupCount; i++) {
            Value id;
            id.SetInt(object->m_groups->at(i));
            groups.PushBack(id, data.GetAllocator());
        }

        obj.AddMember("Groups", groups, allocator);

        blocks.PushBack(obj, data.GetAllocator());
    }

    data.AddMember("Blocks", blocks, allocator);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    data.Accept(writer);

    log::info("{} | {} | {}", buffer.GetString(), buffer.GetLength(), buffer.GetSize());

    auto client = CCHttpClient::getInstance();
    auto request = new CCHttpRequest();
    request->setUrl("http://127.0.0.1:8000/api");

    auto str = fmt::format("json={}", buffer.GetString());
    request->setRequestData(str.c_str(), str.length());
    request->setRequestType(CCHttpRequest::kHttpPost);
    request->setResponseCallback(this, SEL_HttpResponse(&AIMenu::onHttpCallback));
    request->setUserData(objectsInAIRect);

    client->send(request);

    auto notification = Notification::create("Sending request...", NotificationIcon::Loading, 0);
    notification->setID("kolyah35.gdn/notification");
    notification->show();
}

void AIMenu::onHttpCallback(CCHttpClient* client, CCHttpResponse* response) {
    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto objectsInAIRect = (CCArray*)response->getHttpRequest()->getUserData();
    auto notification = (Notification*)this->getChildByIDRecursive("kolyah35.gdn/notification");
    
    if(response->isSucceed()) {
        CCObject* obj;
        CCARRAY_FOREACH(objectsInAIRect, obj) {
            GameObject* object = dynamic_cast<GameObject*>(obj);
            if(!object->isTrigger())
                levelEditorLayer->removeObject(object, true);
        }

        notification->setIcon(NotificationIcon::Error);
        notification->setString(fmt::format("Error {}", response->getResponseCode()));
        notification->setTime(1.0f);
    } else {
        return;
    }

    auto resp = response->getResponseData();
    // auto resp = "{\"Blocks\": [{\"ID\": 1,\"X\": 0,\"Y\": 0,\"Z\": 8,\"L\": 1,\"ScaleX\": 2,\"ScaleY\": 4,\"Color\": [],\"Groups\": [1, 3]}],\"Colors\": [{\"ID\": 3,\"R\": 255,\"G\": 0,\"B\": 0}] }";

    log::info("RETURNED JSON {}", resp->data());

    Document responsejson;
    responsejson.Parse(resp->data());

    for(auto& block : responsejson["Blocks"].GetArray()) {
        auto obj = block.GetObject();
        
        auto ID = obj["ID"].GetInt();
    
        auto x = obj["X"].GetFloat();
        auto y = obj["Y"].GetFloat();
        auto z = obj["Z"].GetInt();
        auto l = obj["L"].GetInt();

        auto scaleX = obj["ScaleX"].GetFloat();
        auto scaleY = obj["ScaleY"].GetFloat();

        auto color = obj["Color"].GetArray();
        auto groups = obj["Groups"].GetArray();

        // auto gameObj = editorUI->createObject(ID, cocos2d::CCPoint {m_selectedRect.origin.x + x, m_selectedRect.origin.y + y});
        auto gameObj = levelEditorLayer->createObject(ID, cocos2d::CCPoint {m_selectedRect.origin.x + x, m_selectedRect.origin.y + y}, true);
        // gameObj->setCustomZLayer(z);
        gameObj->updateCustomScaleX(scaleX);
        gameObj->updateCustomScaleY(scaleY);

        log::info("place");

        levelEditorLayer->addSpecial(gameObj);
    }
}