#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

#include "AIMenu.hpp"

#define ALIGN(var) ((int)##var % 30)

using namespace geode::prelude;

class $modify(AIEditor, EditorUI) {
	bool init(LevelEditorLayer* editor) {
		if(!EditorUI::init(editor)) return false;

		// auto menu = (CCMenu*)this->getChildren()->objectAtIndex(24);
		auto menu = CCMenu::create();
		auto aibtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_deSelBtn2_001.png"), this, SEL_MenuHandler(&AIEditor::onAI));
		aibtn->setPosition({-110, 150});

		menu->addChild(aibtn);
		this->addChild(menu);
		// menu->alignItemsHorizontallyWithPadding(48);

		AIMenu::m_invisibleArray = cocos2d::CCArray::create();
    	AIMenu::m_invisibleArray->retain();

		return true;
	}

	void onAI(CCObject* obj) {
		auto aiMenu = AIMenu::create(300, 200);
		this->getParent()->addChild(aiMenu);
	}

	void onPause(CCObject* sender) {
		if(!AIMenu::m_aiMode)
			return EditorUI::onPause(sender);
	}
};

class $modify(LevelEditorLayer){
	bool init(GJGameLevel* level, bool p1) {
		if(!LevelEditorLayer::init(level, p1)) return false;

		AIMenu::m_drawbox = CCDrawNode::create();
		AIMenu::m_drawbox->setZOrder(1000);
		this->m_objectLayer->addChild(AIMenu::m_drawbox);

		return true;
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