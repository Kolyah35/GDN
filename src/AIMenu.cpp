#include <Geode/Geode.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include "AIMenu.hpp"
#include <Geode/utils/web.hpp>
#include <nlohmann/json.hpp>
#include "GDNLayer.hpp"
#include "GDNGlobal.hpp"
#include "Geode/binding/GJSpriteColor.hpp"
#include "Geode/ui/Layout.hpp"

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
    ColumnLayout *layout = ColumnLayout::create();
    this->m_buttonMenu->setLayout(layout);
    this->m_mainLayer->addChild(this->m_buttonMenu);

    setup();
    this->m_buttonMenu->updateLayout();

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

#include "GDNGlobal.hpp"

void AIMenu::onClose(cocos2d::CCObject*) {
    if (m_aiSelectObjects2 == true) return;

    aimenu_exists = false;

    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto editorUI = (EditorUI*)levelEditorLayer->getChildByID("EditorUI");
    auto okBtn = editorUI->getChildByIDRecursive("kolyah35.gdn/okBtn");
    auto gm = GameManager::sharedState();

    // CCMenuItemSpriteExtra *swipeBtn = typeinfo_cast<CCMenuItemSpriteExtra *>(editorUI->getChildByIDRecursive("swipe-button"));

    // if (swipeBtn != nullptr) {
    //     swipeBtn->activate();
    // }

    CCObject* obj;
    CCARRAY_FOREACH(m_invisibleArray, obj) {
        CCNode* node = typeinfo_cast<CCNode*>(obj);
        if (node) {
            node->setVisible(true);
        }
    }

    // gm->setGameVariable("0003", m_swipeEnabled);
    // if(!gm->getGameVariable("0003")) {
    //     auto menu = typeinfo_cast<CCMenu *>(editorUI->getChildren()->objectAtIndex(5));
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

    auto textInput = geode::TextInput::create(260, "Enter prompt...", "chatFont.fnt");
    auto idInput = geode::TextInput::create(260, "Enter color IDs to be used (i.e. 1-5)...", "chatFont.fnt");
    idInput->setFilter("0123456789-");

    CCMenu *btnMenu = CCMenu::create();
    RowLayout *layout = RowLayout::create();
    btnMenu->setLayout(layout);

    auto sendBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Send"), this, menu_selector(AIMenu::onSendBtn));
	btnMenu->addChild(sendBtn);

	if (Mod::get()->getSettingValue<bool>("experimental-features") == true) {
        auto feedbackBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Feedback"), this, menu_selector(AIMenu::onSendBtn));
    	btnMenu->addChild(feedbackBtn);

    	auto selectAreaBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Select Area"), this, menu_selector(AIMenu::onSendBtn));
    	btnMenu->addChild(selectAreaBtn);
	}

	btnMenu->setContentWidth(m_layerSize.width);
	btnMenu->setTouchPriority(-700);
	btnMenu->updateLayout();
    this->m_buttonMenu->addChild(btnMenu);

    idInput->setID("kolyah35.gdn/idInput");
    this->m_buttonMenu->addChild(idInput);

    textInput->setID("kolyah35.gdn/textArea");
    this->m_buttonMenu->addChild(textInput);

    this->m_buttonMenu->updateLayout();
}

void AIMenu::keyBackClicked() {
    if (m_aiSelectObjects2) return;

    aimenu_exists = false;

    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    auto editorUI = (EditorUI*)levelEditorLayer->getChildByID("EditorUI");

    CCMenuItemSpriteExtra *swipeBtn = typeinfo_cast<CCMenuItemSpriteExtra *>(editorUI->getChildByIDRecursive("swipe-button"));
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

    // auto color = levelEditorLayer->m_levelSettings->m_effectManager->getColorAction(3);
    // color->m_color = { 145, 255, 0 };

    if(!m_aiMode){
        m_aiSelectObjects = true;
        m_aiSelectObjects2 = true;

        m_invisibleArray->removeAllObjects();

        editorUI->setVisible(false);

        // for(int i = 0; i < editorUI->getChildrenCount(); i++) {
        //     CCNode* node = typeinfo_cast<CCNode*>(editorUI->getChildren()->objectAtIndex(i));

        //     if(node != nullptr && node->isVisible()){
        //         m_invisibleArray->addObject(typeinfo_cast<CCObject*>(node));
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

        CCMenuItemSpriteExtra *swipeBtn = typeinfo_cast<CCMenuItemSpriteExtra *>(editorUI->getChildByIDRecursive("swipe-button"));

        if (swipeBtn != nullptr) {
            auto btnsprv = GDNGlobal::findInstancesOfObj<ButtonSprite>(swipeBtn);

            if (btnsprv.size() == 0) {
                log::info("ButtonSprite cannot be found inside swipe button");

                swipeBtn->activate();
            } else {
                ButtonSprite *btnspr = btnsprv[0];

                CCSprite *bg = btnspr->m_subBGSprite;

                // definetely not stolen from devtools
                if (auto textureProtocol = typeinfo_cast<CCTextureProtocol*>(bg)) {
                    if (auto texture = textureProtocol->getTexture()) {
                        auto* cachedTextures = CCTextureCache::sharedTextureCache()->m_pTextures;
                        for (auto [key, obj] : CCDictionaryExt<std::string, CCTexture2D*>(cachedTextures)) {
                            if (obj == texture) {

                                if (key.find("GJ_button_01") != std::string::npos) {
                                    swipeBtn->activate();
                                    log::info("activating swipe");
                                } else {
                                    log::info("swipe is already has been activated");
                                }

                                break;
                            }
                        }
                    }
                } else {
                    log::info("bg is not texture protocol");

                    swipeBtn->activate();
                }
            }
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
    auto inputNode = (TextInput*)this->m_buttonMenu->getChildByID("kolyah35.gdn/textArea");
    auto idInputNode = (TextInput*)this->m_buttonMenu->getChildByID("kolyah35.gdn/idInput");

    std::string id_area = idInputNode->getString();
    std::string val_a, val_b;
    bool val_switch = false;

    for (const char c : id_area) {
        if (c != '-') {
            if (!val_switch) val_a += c;
            else val_b += c;
        } else {
            if (!val_a.empty()) val_switch = true;
        }
    }

    log::info("val_a={}; val_b={}", val_a, val_b);

    int ival_a = 1;
    int ival_b = 5;
    if (!val_a.empty()) {
        ival_a = std::stoi(val_a);
    }
    if (!val_b.empty()) {
        ival_b = std::stoi(val_b);
    }

    if (ival_b - ival_a < 4) return;

    std::vector<int> color_ids;
    for (int i = ival_a; i <= ival_b; i++) {
        color_ids.push_back(i);
    }

    m_aiSelectObjects = true;
    GDNGlobal::accessSelectedObjects();
    auto objectsInRect = GDNGlobal::selectedObjects;

    log::info("amount of GameObjects: {}", objectsInRect->count());

    nlohmann::json data;

    data["Prompt"] = inputNode->getString();
    data["ListColor"] = color_ids;

    nlohmann::json blocks = nlohmann::json::array();
    m_selectedRect = GDNGlobal::createOriginRect(GDNGlobal::convertArrayIntoVector<GameObject>(GDNGlobal::selectedObjects));

    nlohmann::json colors;

    for(int i = 0; i < objectsInRect->count(); i++) {
        nlohmann::json bdata;

        auto object = typeinfo_cast<GameObject*>(objectsInRect->objectAtIndex(i));

        bdata["ID"] = object->m_objectID;
        bdata["PosX"] = (int)(object->getPositionX() - m_selectedRect.origin.x);
        bdata["PosY"] = (int)(object->getPositionY() - m_selectedRect.origin.y);
        bdata["ScaleX"] = object->m_scaleX;
        bdata["ScaleY"] = object->m_scaleY;
        bdata["Rotate"] = object->getRotation();
        bdata["Z_order"] = object->getObjectZOrder();
        bdata["Z_layer"] = object->getObjectZLayer();
        bdata["BaseColId"] = (object->m_baseColor != nullptr) ? object->m_baseColor->m_colorID : -1;
        bdata["DetColId"] = (object->m_detailColor != nullptr) ? object->m_detailColor->m_colorID : -1;

        std::vector<GJSpriteColor*> c = {
            object->m_baseColor,
            object->m_detailColor
        };

        for (GJSpriteColor *col : c) {
            if (!col) continue;

            auto action = levelEditorLayer->m_levelSettings->m_effectManager->getColorAction(col->m_colorID);
            nlohmann::json color_obj = nlohmann::json::array();
            color_obj.push_back(col->m_colorID);
            if (!action) {
                color_obj.push_back(255);
                color_obj.push_back(255);
                color_obj.push_back(255);
                color_obj.push_back(0);
                color_obj.push_back(-1);
            } else {
                color_obj.push_back((int)action->m_fromColor.r);
                color_obj.push_back((int)action->m_fromColor.g);
                color_obj.push_back((int)action->m_fromColor.b);
                color_obj.push_back((int)action->m_blending);
            }
            colors[std::to_string(col->m_colorID)] = color_obj;
        }

        blocks.push_back(bdata);
    }

    data["Data"] = blocks;
    data["Colors"] = colors;

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

std::string AIMenu::createColorTrigger(int colId, ccColor3B col, float dur, bool blending) {
    // we would setup a 2.0 color trigger with customizable rgb and col id values
    std::string color_trigger_str = fmt::format("1,899,2,-100,3,100,7,{},8,{},9,{},10,{},17,{},23,{}", col.r, col.g, col.b, dur, (int)blending, colId);

    // return color trigger data
	return color_trigger_str;
}

std::string AIMenu::createStandardObject(cocos2d::CCPoint pos, int id, int l, float scaleX, float scaleY, int baseCol, float rotation, std::vector<int> groups, int zlayer, int zorder) {
    std::string group_string;

    for (int g : groups) {
        group_string += fmt::format("{}.", g);
    }
    if (groups.size() != 0 && !group_string.empty()) {
        group_string.pop_back();
    }

    std::string object_str = fmt::format("1,{},2,{},3,{},20,{},21,{},128,{},129,{},6,{},57,{},24,{},25,{}", id, pos.x, pos.y, l, baseCol, scaleX, scaleY, rotation, group_string, zlayer, zorder);

	return object_str;
}

void AIMenu::onHttpCallback(CCHttpClient* client, CCHttpResponse* response) {
    auto levelEditorLayer = (LevelEditorLayer*)CCScene::get()->getChildren()->objectAtIndex(0);
    // auto notification = (Notification*)this->getChildByIDRecursive("kolyah35.gdn/notification");
    m_aiSelectObjects = true;
    GDNGlobal::accessSelectedObjects();
    auto objectsInRect = GDNGlobal::selectedObjects;

    log::info("HTTP CALLBACK");
    log::info("{}", objectsInRect == nullptr);

    if(response->isSucceed()) {
        log::info("Request succeeded");
        // bool cleanupObjects = Mod::get()->getSettingValue<bool>("cleanup-objects");

        // if (cleanupObjects) {
        //     log::info("deleting objects");

        //     for(int i = 0; i < objectsInRect->count(); i++){
        //         GameObject* object = typeinfo_cast<GameObject*>(objectsInRect->objectAtIndex(i));

        //         if(object != nullptr && !object->isTrigger()) {
        //             levelEditorLayer->removeObject(object, true);
        //             objectsInRect->removeObject(objectsInRect->objectAtIndex(i), false);
        //             i--;
        //         }
        //     }
        // }
    } else {
        // log::info("HTTP ERROR");
        int responseCode = response->getResponseCode();

        notification->setIcon(NotificationIcon::Error);
        notification->setTime(1.0f);

        if (responseCode == -1) {
            notification->setString(fmt::format("Error: Server is unreachable. ({})", response->getErrorBuffer()));
        }
        else {
            notification->setString(fmt::format("Error {}: {}", responseCode, response->getErrorBuffer()));
        }

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

    std::map<int, ccColor4B> colors;

    for (int i = 0; i < responsejson["Data"].size(); i++) {
        nlohmann::json obj = responsejson["Data"].at(i);

        auto ID = obj["ID"].get<int>();

        log::info("parcing block ID:{}", ID);

        auto x = obj["PosX"].get<float>();
        auto y = obj["PosY"].get<float>();

        auto rot = obj["Rotate"].get<float>();

        x += m_selectedRect.origin.x + 30;
        y += m_selectedRect.origin.y + 30;

        auto scaleX = obj["ScaleX"].get<float>();
        auto scaleY = obj["ScaleY"].get<float>();

        auto color = obj["Color"];

        ccColor4B colobj = {};
        colobj.a = (color["Blending"].get<bool>()) ? 0 : 255;
        colobj.r = color["R"].get<int>();
        colobj.g = color["G"].get<int>();
        colobj.b = color["B"].get<int>();
        int colid = color["id"].get<int>();

        int layer = 0, zlayer = 0, zorder = 0;
        if (obj.contains("L")) layer = obj["L"].get<int>();
        if (obj.contains("Z_layer")) zlayer = obj["Z_layer"].get<int>();
        if (obj.contains("Z_order")) zorder = obj["Z_order"].get<int>();

        colors[colid] = colobj;

        std::vector<int> groups = {};
        /*for (nlohmann::json::iterator it = obj["Groups"].begin(); it != obj["Groups"].end(); ++it) {
            auto key = *it;
            groups.push_back(key);
        }*/

        log::info("generating object string of {} (zlayer={}, zorder={})", ID, zlayer, zorder);

        std::string objdata = createStandardObject({x,y}, ID, layer, scaleX, scaleY, colid, rot, groups, zlayer, zorder);
        _gameObjects.push_back(objdata);
    }

    for (const auto &[k, v] : colors) {
        std::string coldata = createColorTrigger(k, {v.r, v.g, v.b}, 0.f, v.a == 0);
        log::info("placing color trigger with this data: {}", coldata);
        levelEditorLayer->createObjectsFromString(coldata, false, false);
    }

    notification->setIcon(NotificationIcon::Success);
    notification->setString(fmt::format("Success! Created {} objects.", responsejson["D ata"].size() + colors.size()));
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
            GameObject *_obj = typeinfo_cast<GameObject *>(objs->objectAtIndex(i));

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
    } else {
        // if (_createdObjects.size() > 0) {
        //     processObjectsGlobally();

        //     _createdObjects.clear();
        // }
    }
}

void AIMenu::processCreatedObject(GameObject *obj) {
    // log::info("processing obj");

    // _createdObjects.push_back(obj);

    // auto lel = LevelEditorLayer::get();

    // UndoObject *undo = UndoObject::create(obj, UndoCommand::New);
    // lel->m_undoObjects->addObject(undo);
}

void AIMenu::removeTouchDispatcher() {
    auto dispatcher = cocos2d::CCDirector::sharedDirector()->getTouchDispatcher();

#ifdef _WIN32
    // 2.204
        // pFVar2 = this + 0x108;
    // 2.206
        // puVar1 = &this[-1].field_0x120;

    CCObject *obj = (CCObject *)(((unsigned char *)this) + 0x120); // MAYBE

    dispatcher->unregisterForcePrio(obj);
#else
    dispatcher->unregisterForcePrio(this);
#endif
}
void AIMenu::addTouchDispatcher() {
    registerWithTouchDispatcher();
}

void AIMenu::processObjectsGlobally() {
    log::info("applying undo action to {} objects", _createdObjects.size());

    CCArray *array = CCArray::create();

    for (GameObject *obj : _createdObjects) {
        array->addObject(obj);
    }

    auto lel = LevelEditorLayer::get();

    UndoObject *undo = UndoObject::createWithArray(array, UndoCommand::New);
    lel->m_undoObjects->addObject(undo);
}
