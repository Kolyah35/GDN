#include <Geode/Geode.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include "AIMenu.hpp"
#include <Geode/utils/web.hpp>
#include <nlohmann/json.hpp>
#include "GDNLayer.hpp"

#undef GetObject

using namespace geode::prelude;
// using namespace rapidjson;

bool aimenu_exists = false;

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
    aimenu_exists = true;

    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
    this->m_layerSize = cocos2d::CCSize {w, h};

    if (!FLAlertLayer::init(0))
        return false;
    // this->m_mainLayer = cocos2d::CCLayer::create();
    this->addChild(this->m_mainLayer);

    auto base = cocos2d::CCSprite::create("square.png");

    base->setPosition({ 0, 0 });
    base->setScale(500.f);
    base->setColor({0, 0, 0});
    base->setOpacity(0);
    base->runAction(cocos2d::CCFadeTo::create(0.15f, 125));

    this->m_mainLayer->addChild(base);

    auto bg = cocos2d::extension::CCScale9Sprite::create(spr, {0.0f, 0.0f, 80.0f, 80.0f});
    bg->setContentSize(this->m_layerSize);
    bg->setPosition(winSize.width / 2, winSize.height / 2);
    this->m_mainLayer->addChild(bg);

    this->m_buttonMenu = cocos2d::CCMenu::create();
    this->m_mainLayer->addChild(this->m_buttonMenu);

    // this->setKeypadEnabled(true);
    // this->setTouchEnabled(true);
    // this->setKeyboardEnabled(false);

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

    scheduleUpdate();

    m_aiSelectObjects = false;
    m_aiSelectObjects2 = false;
    m_aiMode = false;

    // show();

    return true;
}

void AIMenu::onClose(cocos2d::CCObject*) {
    if (m_aiSelectObjects2 == true) return;

    aimenu_exists = false;

    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto editorUI = (EditorUI*)levelEditorLayer->getChildByID("EditorUI");
    auto okBtn = editorUI->getChildByIDRecursive("kolyah35.gdn/okBtn");
    auto gm = GameManager::sharedState();

    CCMenuItemSpriteExtra *swipeBtn = dynamic_cast<CCMenuItemSpriteExtra *>(editorUI->getChildByIDRecursive("swipe-button"));
    if (swipeBtn != nullptr) {
        swipeBtn->activate();
    }

    CCObject* obj;
    CCARRAY_FOREACH(m_invisibleArray, obj) {
        CCNode* node = dynamic_cast<CCNode*>(obj);
        if (node) {
            node->setVisible(true);
        }
    }

    // gm->setGameVariable("0003", m_swipeEnabled);
    // if(!gm->getGameVariable("0003")) {
    //     auto menu = dynamic_cast<CCMenu *>(editorUI->getChildren()->objectAtIndex(5));
    //     if (menu) {
    //         auto swipeBtn = menu->getChildren()->objectAtIndex(3);
    //         if (swipeBtn) {
    //             editorUI->toggleSwipe(swipeBtn);
    //         }
    //     }
    // }

    // editorUI->m_selectedMode = m_currentMode;
    m_aiSelectObjects = false;
    m_ignoreLayer = false;
    // m_selectedRect = {0, 0, 0, 0};
    m_aiMode = false;

    // editorUI->m_editButtonBar->setPositionY(0);

    if(okBtn) okBtn->removeFromParent();

    // m_drawbox->clear();

    // this->setKeyboardEnabled(true);
    if (_closeWithCleanup) this->removeFromParentAndCleanup(true);

    _closed = true;
}

void AIMenu::setup() {
	this->setZOrder(101);
    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();	
    
    auto selectBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Select Area"), this, cocos2d::SEL_MenuHandler(&AIMenu::selectAreaClicked));
    selectBtn->setPosition(this->m_buttonMenu->convertToNodeSpace({winSize.width / 2, 235}));
	this->m_buttonMenu->addChild(selectBtn);

    auto textInput = geode::InputNode::create(260, "Enter prompt...", "chatFont.fnt", " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,-!?:;)(/\\\"'`*=+-_%[]<>|@&^#{}%$~", 200);
    textInput->setID("kolyah35.gdn/textArea");
    this->m_buttonMenu->addChild(textInput);

	auto sendBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Send"), this, SEL_MenuHandler(&AIMenu::onSendBtn));
	sendBtn->setPosition(this->m_buttonMenu->convertToNodeSpace({winSize.width / 2, 85}));
	this->m_buttonMenu->addChild(sendBtn);
}

void AIMenu::keyBackClicked() {
    if (m_aiSelectObjects2) return;

    aimenu_exists = false;

    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto editorUI = (EditorUI*)levelEditorLayer->getChildByID("EditorUI");

    CCMenuItemSpriteExtra *swipeBtn = dynamic_cast<CCMenuItemSpriteExtra *>(editorUI->getChildByIDRecursive("swipe-button"));
    if (swipeBtn != nullptr) {
        swipeBtn->activate();
    }

    // m_selectedRect = {0, 0, 0, 0};
    m_aiMode = false;

    // m_drawbox->clear();

    FLAlertLayer::keyBackClicked();
}

void AIMenu::selectAreaClicked(cocos2d::CCObject*) {
    auto menu = (cocos2d::CCMenu*)this->getChildByIDRecursive("kolyah35.gdn/menu");
    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto editorUI = (EditorUI*)levelEditorLayer->getChildByID("EditorUI");

    auto color = levelEditorLayer->m_levelSettings->m_effectManager->getColorAction(3);
    color->m_color = { 145, 255, 0 };


    if(!m_aiMode){
        m_aiSelectObjects = true;
        m_aiSelectObjects2 = true;

        m_invisibleArray->removeAllObjects();

        editorUI->setVisible(false);

        // for(int i = 0; i < editorUI->getChildrenCount(); i++) {
        //     CCNode* node = dynamic_cast<CCNode*>(editorUI->getChildren()->objectAtIndex(i));

        //     if(node != nullptr && node->isVisible()){
        //         m_invisibleArray->addObject(dynamic_cast<CCObject*>(node));
        //         node->setVisible(false);
        //     }
        // }

        // auto layerNum = (CCMenu*)editorUI->getChildren()->objectAtIndex(0);
        // layerNum->setVisible(true);

        // auto slider = (Slider*)editorUI->getChildren()->objectAtIndex(4);
        // slider->setVisible(true);

        // auto zoomMenu = (CCMenu*)editorUI->getChildren()->objectAtIndex(5);
        // zoomMenu->setVisible(true);

        // for(int i = 0; i < zoomMenu->getChildrenCount(); i++) {
        //     if(i == 13 || i == 14) continue;

        //     auto obj = (CCNode*)zoomMenu->getChildren()->objectAtIndex(i);
        //     m_invisibleArray->addObject(obj);
        //     obj->setVisible(false);
        // }

        // auto btnsMenu = (CCMenu*)editorUI->getChildren()->objectAtIndex(25);
        // btnsMenu->setVisible(true);

        // for(int i = 0; i < btnsMenu->getChildrenCount(); i++) {
        //     if(i >= 14 && i <= 16) continue;

        //     auto obj = (CCNode*)btnsMenu->getChildren()->objectAtIndex(i);
        //     m_invisibleArray->addObject(obj);
        //     obj->setVisible(false);
        // }

        // editorUI->m_editButtonBar->setPositionY(-200);

        auto gm = GameManager::sharedState();

        m_swipeEnabled = gm->getGameVariable("0003");
        m_currentMode = editorUI->m_selectedMode;
        m_aiMode = true;

        // gm->setGameVariable("0003", true);
        editorUI->m_selectedMode = 3;

        CCMenu *_menu = CCMenu::create();

        auto okBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("OK"), this, cocos2d::SEL_MenuHandler(&AIMenu::okButtonClicked));
        
        okBtn->setID("kolyah35.gdn/okBtn");
        _menu->addChild(okBtn);

        _menu->setPosition({winSize.width / 2, winSize.height - okBtn->getContentSize().height - 10});

        addChild(_menu);

        this->m_mainLayer->setVisible(false);
        this->m_buttonMenu->setVisible(false);

        removeTouchDispatcher();

        CCMenuItemSpriteExtra *swipeBtn = dynamic_cast<CCMenuItemSpriteExtra *>(editorUI->getChildByIDRecursive("swipe-button"));
        if (swipeBtn != nullptr) {
            swipeBtn->activate();
        }

        // FLAlertLayer::keyBackClicked();
    }

    // this->setKeyboardEnabled(false);
    // this->removeFromParentAndCleanup(true);
}

void AIMenu::okButtonClicked(CCObject*) {
    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto editorUI = (EditorUI*)levelEditorLayer->getChildByID("EditorUI");
    auto aiMenu = AIMenu::create(300, 200);

    m_aiSelectObjects = false;
    m_aiSelectObjects2 = false;

    removeMeAndCleanup();

    aiMenu->setID("kolyah35.gdn/AIMenu");
    editorUI->getParent()->addChild(aiMenu);
    editorUI->setVisible(true);

    // this->m_mainLayer->setVisible(true);
    // this->m_buttonMenu->setVisible(true);

    // addTouchDispatcher();
}

void AIMenu::onSendBtn(CCObject*) {
    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto editorUI = (EditorUI*)levelEditorLayer->getChildByID("EditorUI");
    auto am = GJAccountManager::sharedState();
    auto inputNode = (InputNode*)this->m_buttonMenu->getChildByID("kolyah35.gdn/textArea");

    m_aiSelectObjects = true;
    auto objectsInRect = levelEditorLayer->objectsInRect(m_selectedRect, m_ignoreLayer);

    nlohmann::json data;

    nlohmann::json userobj;

    // Document data;
    // data.SetObject();

    // auto allocator = data.GetAllocator();

    // Value userobj;
    // userobj.SetObject();

    // Value username;
    // username.SetString(am->m_username.c_str(), am->m_username.size());
    // userobj.AddMember("UserName", username, allocator);

    // Value prompt;
    // prompt.SetString(inputNode->getString().c_str(), inputNode->getString().length());
    // userobj.AddMember("Prompt", prompt, allocator);

    // data.AddMember("User", userobj, allocator);

    userobj["UserName"] = am->m_username;
    userobj["Prompt"] = inputNode->getString();

    data["User"] = userobj;

    // Value blocks;
    // blocks.SetArray();

    nlohmann::json blocks = nlohmann::json::array();

    for(int i = 0; i < objectsInRect->count(); i++) {
        nlohmann::json bdata;

        auto object = dynamic_cast<GameObject*>(objectsInRect->objectAtIndex(i));

        bdata["ID"] = object->m_objectID;
        bdata["X"] = (int)(object->getPositionX() - m_selectedRect.origin.x);
        bdata["Y"] = (int)(object->getPositionY() - m_selectedRect.origin.y);
        bdata["Z"] = (int)(object->m_zOrder);
        bdata["L"] = (int)(object->m_editorLayer);
        bdata["ScaleX"] = object->m_scaleX;
        bdata["ScaleY"] = object->m_scaleY;
        bdata["ScaleX"] = object->m_scaleX;
        bdata["Rotation"] = object->getRotation();

        // nlohmann::json color;

        // for(int i = 0; i < object->m_colorGroupCount; i++) {
        //     color.push_back(object->m_colorGroups->at(i))
        // }

        if(object->m_baseColor) {;
            bdata["BaseColor"] = (int)object->m_baseColor->m_colorID;
        }

        if(object->m_detailColor) {
            bdata["DetailColor"] = (int)object->m_detailColor->m_colorID;
        }

        nlohmann::json groups = nlohmann::json::array();

        for(int i = 0; i < object->m_groupCount; i++) {
            // Value id;
            // id.SetInt(object->m_groups->at(i));
            // groups.PushBack(id, data.GetAllocator());
            groups.push_back(object->m_groups->at(i));
        }

        bdata["Groups"] = groups;

        blocks.push_back(bdata);
    }

    data["Blocks"] = blocks;

    std::string buffer = data.dump();

    log::info("{} | {} | {}", buffer, buffer.length(), buffer.size());

    auto client = CCHttpClient::getInstance();
    auto request = new CCHttpRequest();
    request->setUrl(Mod::get()->getSettingValue<std::string>("url").c_str());
    request->setRequestData(buffer.c_str(), buffer.size());
    request->setRequestType(CCHttpRequest::kHttpPost);
    request->setResponseCallback(this, SEL_HttpResponse(&AIMenu::onHttpCallback));

    log::info("APIURL {}", request->getUrl());

    client->send(request);

    notification = Notification::create("Sending request...", NotificationIcon::Loading, 0);
    // notification->setID("kolyah35.gdn/notification");
    notification->show();

    GDNLayer *gdnl = GDNLayer::create();
    gdnl->setID("gdnlayer");
    addChild(gdnl);
}

std::string AIMenu::createColorTrigger(int colId, ccColor3B col, float dur) {
    // we would setup a 2.0 color trigger with customizable rgb and col id values
    std::string color_trigger_str = fmt::format("1,899,2,-100,3,100,7,{},8,{},9,{},10,{},23,{}", col.r, col.g, col.b, dur, colId);
	
    // return color trigger data
	return color_trigger_str;
}

std::string AIMenu::createStandardObject(cocos2d::CCPoint pos, int id, int z, int l, float scaleX, float scaleY, int baseCol, int detailCol, float rotation, std::vector<int> groups) {
    std::string group_string;

    for (int g : groups) {
        group_string += fmt::format("{}.", g);
    }

    group_string.pop_back();

    std::string object_str = fmt::format("1,{},2,{},3,{},20,{},25,{},21,{},22,{},128,{},129,{},6,{},57,{}", id, pos.x, pos.y, l, z, baseCol, detailCol, scaleX, scaleY, rotation, group_string);
	
	return object_str;
}

void AIMenu::onHttpCallback(CCHttpClient* client, CCHttpResponse* response) {
    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    // auto notification = (Notification*)this->getChildByIDRecursive("kolyah35.gdn/notification");
    m_aiSelectObjects = true;
    auto objectsInRect = levelEditorLayer->objectsInRect(m_selectedRect, m_ignoreLayer);
    
    // log::info("HTTP CALLBACK");
    // log::info("{}", objectsInRect == nullptr);

    if(response->isSucceed()) {
        // log::info("deleting objects");
        for(int i = 0; i < objectsInRect->count(); i++){
            GameObject* object = dynamic_cast<GameObject*>(objectsInRect->objectAtIndex(i));
            if(object != nullptr && !object->isTrigger()) {
                levelEditorLayer->removeObject(object, true);
                objectsInRect->removeObject(objectsInRect->objectAtIndex(i), false);
                i--;
            }
        }
    } else {
        // log::info("HTTP ERROR");
        notification->setIcon(NotificationIcon::Error);
        notification->setString(fmt::format("Error {}: {}", response->getResponseCode(), response->getErrorBuffer()));
        notification->setTime(1.0f);

        _closeWithCleanup = true;
        onClose(this);
        removeAllChildrenWithCleanup(true);

        removeTouchDispatcher();

        return;
    }

    auto resp = response->getResponseData();

    std::string returned_json(resp->data(), resp->size());

    // log::info("RETURNED JSON {}", returned_json);

    nlohmann::json responsejson = nlohmann::json::parse(returned_json, nullptr, false);

    // std::string object_array = "";
    _readyToPlace = false;

    for (int i = 0; i < responsejson["Blocks"].size(); i++) {
        nlohmann::json obj = responsejson["Blocks"].at(i);

        auto ID = obj["ID"].get<int>();
    
        auto x = obj["X"].get<float>();
        auto y = obj["Y"].get<float>();
        auto z = obj["Z"].get<int>();
        auto l = obj["L"].get<int>();

        auto rot = obj["Rotation"].get<float>();

        x += m_selectedRect.origin.x;
        y += m_selectedRect.origin.y;

        auto scaleX = obj["ScaleX"].get<float>();
        auto scaleY = obj["ScaleY"].get<float>();

        auto baseColor = obj["BaseColor"].get<int>();
        auto detailColor = obj["DetailColor"].get<int>();
        
        std::vector<int> groups;
        for (nlohmann::json::iterator it = obj["Groups"].begin(); it != obj["Groups"].end(); ++it) {
            auto key = *it;
            groups.push_back(key);
        }

        std::string objdata = createStandardObject({x,y}, ID, z, l, scaleX, scaleY, baseColor, detailColor, rot, groups);
        _gameObjects.push_back(objdata);
    }

    for (int i = 0; i < responsejson["Colors"].size(); i++) {
        nlohmann::json color = responsejson["Colors"].at(i);

        auto ID = color["ID"].get<int>();
    
        uint8_t r = (unsigned int)color["R"].get<int>();
        uint8_t g = (unsigned int)color["G"].get<int>();
        uint8_t b = (unsigned int)color["B"].get<int>();
       
        // log::info("creating color trigger with this info: id={}, r={}, g={}, b={}, dur={}", ID, r, g, b, 0.f);
        
        std::string coldata = createColorTrigger(ID, {r, g, b}, 0.f);
        // log::info("placing color trigger with this data: {}", coldata);
        levelEditorLayer->createObjectsFromString(coldata, false, false);
    }

    notification->setIcon(NotificationIcon::Success);
    notification->setString(fmt::format("Success! Created {} objects.", responsejson["Blocks"].size() + responsejson["Colors"].size()));
    notification->setTime(1.0f);

    // delete resp;

    _closeWithCleanup = false;
    onClose(this);
    removeAllChildrenWithCleanup(true);

    _readyToPlace = true;

    removeTouchDispatcher();
}

void AIMenu::update(float delta) {
    if (_gameObjects.size() > 0 && _readyToPlace) {
        _closeWithCleanup = false;
        if (!_closed) onClose(this);
        setVisible(false); 

        std::string gameObj = _gameObjects.at(_gameObjects.size() - 1);

        auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);

        cocos2d::CCArray * objs = levelEditorLayer->createObjectsFromString(gameObj, false, false);

        for (int i = 0; i < objs->count(); i++) {
            GameObject *_obj = dynamic_cast<GameObject *>(objs->objectAtIndex(i));

            if (_obj) {
                processCreatedObject(_obj);
            }
        }

        while (objs->count() != 0) {
            objs->removeObjectAtIndex(0, false);
        }

        objs->release();

        _gameObjects.pop_back();
        if (_gameObjects.size() == 0) {
            this->removeFromParentAndCleanup(true);
        }
    }
}

void AIMenu::processCreatedObject(GameObject *obj) {
    // log::info("processing obj");
}

void AIMenu::removeTouchDispatcher() {
    auto dispatcher = cocos2d::CCDirector::sharedDirector()->getTouchDispatcher();
    // pFVar2 = this + 0x108;

    // windows
    auto delegate = (cocos2d::CCTouchDelegate *)(((char *)this) + 0x108);

    dispatcher->removeDelegate(delegate);
}
void AIMenu::addTouchDispatcher() {
    registerWithTouchDispatcher();

}