#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/FLAlertLayerProtocol.hpp>

#include "AIMenu.hpp"
#include "GDNLayer.hpp"

/*
	ОСТОРОЖНО! ЩИТКОД
	Многие вещи в этом моде были написаны на скорую руку
	для теста и в планах былодорботать все на релизе, но 
	я устал работать над этим модом. Удачи.
*/

/*
    я исправил немного щиткода

	почему   прошлый   программист   решил  использовать
	rapidjson?? есть же более удобная альтернатива этому

	в моей  среде  rapidjson  не  хочет  компилироваться
	в никакую
*/

#define ALIGN(var) ((int)var % 30)

using namespace geode::prelude;

class REMessage {
public:
	void run() {
		printf("hello reverse engineers welcome to the game\n");
	}
};

namespace AISettings {
	REMessage reMessage;
	bool loginSuccessful = false;
	bool attemptedToLogin = false;
}

class $modify(AIEditor, EditorUI) {
	bool init(LevelEditorLayer* editor) {
		if(!EditorUI::init(editor)) return false;

		// auto menu = (CCMenu*)this->getChildren()->objectAtIndex(24);

		auto btn_spr = EditorButtonSprite::createWithSprite(
			"gdn2.png"_spr,
			0.5f,
			EditorBaseColor::Gray,
			EditorBaseSize::Normal
		);

		if (!btn_spr) {
			log::info("failed to create EditorButtonSprite\n");

			AIMenu::m_invisibleArray = cocos2d::CCArray::create();
    		AIMenu::m_invisibleArray->retain();

			return true;
		}

		auto menu = this->getChildByID("editor-buttons-menu");
		
		auto aibtn = CCMenuItemSpriteExtra::create(btn_spr, this, SEL_MenuHandler(&AIEditor::onAI));
		
		aibtn->setPosition({-110, 150});
		aibtn->setID("gdn-create");

		menu->addChild(aibtn);
		menu->updateLayout();

		aibtn->setAnchorPoint({0.7f, 0.65f});

		AIMenu::m_invisibleArray = cocos2d::CCArray::create();
    	AIMenu::m_invisibleArray->retain();

		return true;
	}

	void onAI(CCObject* obj) {
		if (AISettings::attemptedToLogin) {
			if (!AISettings::loginSuccessful) {
				GDNLayer::loginFailureMessage();

				return;
			} else {
				auto aiMenu = AIMenu::create(300, 200);
				this->getParent()->addChild(aiMenu);
			}
		} else {
			AISettings::attemptedToLogin = true;

			auto l = GDNLayer::create();
			l->setCloseOnFullSuccess(true);
			l->setCallback([this, obj] (GDNLayer *) {
				AISettings::loginSuccessful = true;
				onAI(obj);
			});

			addChild(l, 100);
		}
	}

	void onPause(CCObject* sender) {
		if(!AIMenu::m_aiMode)
			return EditorUI::onPause(sender);
	}
};

class $modify(AMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		auto bottomMenu = static_cast<CCMenu*>(this->getChildByID("bottom-menu"));

		auto winSize = CCDirector::sharedDirector()->getWinSize();

		auto btn_spr = CircleButtonSprite::createWithSprite(
			"gdn2.png"_spr,
			1.0f,
			CircleBaseColor::Green,
			CircleBaseSize::MediumAlt
		);

		if (!btn_spr) {
			log::info("failed to create CircleButtonSprite\n");
			return true;
		}

		auto btn = CCMenuItemSpriteExtra::create(
        	btn_spr, this,
            static_cast<SEL_MenuHandler>(&AMenuLayer::onGDN)
        );

		if (!btn) {
			log::info("failed to create CCMenuItemSpriteExtra\n");
			return true;
		}

        btn->setID("gdn-button"_spr);
        bottomMenu->addChild(btn);
        bottomMenu->setContentSize({ winSize.width / 2, bottomMenu->getScaledContentSize().height });

        bottomMenu->updateLayout();

		return true;
	}

	void onGDN(CCObject *obj) {
		AISettings::attemptedToLogin = true;

		auto l = GDNLayer::create();

		l->setCallback([this, obj] (GDNLayer *) {
			AISettings::loginSuccessful = true;
		});

		addChild(l, 100);
    }
};

/**
 * void onPausePlaytest() = win 0x246c70;
	void onPlaytest() = win 0x2463a0;
	void onResumePlaytest() = win 0x246d6
*/
class $modify(ALevelEditorLayer, LevelEditorLayer){
	bool init(GJGameLevel* level, bool p1) {
		if(!LevelEditorLayer::init(level, p1)) return false;

		AIMenu::m_drawbox = CCDrawNode::create();
		AIMenu::m_drawbox->setZOrder(1000);
		this->m_objectLayer->addChild(AIMenu::m_drawbox);

		return true;
	}

	void hideEditorButtons() {
		auto menu = this->m_editorUI->getChildByID("editor-buttons-menu");

		menu->setVisible(false);
	}
	void showEditorButtons() {
		auto menu = this->m_editorUI->getChildByID("editor-buttons-menu");

		menu->setVisible(true);
	}

	void onPausePlaytest() {
		LevelEditorLayer::onPausePlaytest();
		showEditorButtons();
	}
	void onPlaytest() {
		LevelEditorLayer::onPlaytest();
		hideEditorButtons();
	}
	void onResumePlaytest() {
		LevelEditorLayer::onResumePlaytest();
		hideEditorButtons();
	}

	CCArray* objectsInRect(cocos2d::CCRect rect, bool ignoreLayer) {
		if(AIMenu::m_aiMode && !AIMenu::m_aiSelectObjects) {
			AIMenu::m_drawbox->clear();

			rect.origin.x = floorf(rect.origin.x - ALIGN(rect.origin.x));
			rect.origin.y = floorf(rect.origin.y - ALIGN(rect.origin.y));
			rect.size.width += (30 - ALIGN(rect.size.width));
			rect.size.height += (30 - ALIGN(rect.size.height));

			CCPoint rectangle[4] = {
				{rect.origin.x, rect.origin.y},
				{rect.origin.x + rect.size.width, rect.origin.y},
				{rect.origin.x + rect.size.width, rect.origin.y + rect.size.height},
				{rect.origin.x, rect.origin.y + rect.size.height}
			};

			AIMenu::m_drawbox->drawPolygon(rectangle, 4, {0.0f, 0.0f, 0.0f, 0.0f}, 1.0f, {1.0f, 1.0f, 0.0f, 1.0f});
			AIMenu::m_selectedRect = rect;
			AIMenu::m_ignoreLayer = ignoreLayer;
			
			return CCArray::create();
		} else {
			AIMenu::m_aiSelectObjects = false;
			return LevelEditorLayer::objectsInRect(rect, ignoreLayer);
		}
	}
};

// class MySettingValue : StringSettingValue {
// public:
	
// };

// $execute {
// 	auto urlSetting = Mod::get()->getSettingDefinition("url");
	
// }
