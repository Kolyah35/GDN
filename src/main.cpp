#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

#define ALIGN(var) ((int)##var % 30)

using namespace geode::prelude;

CCDrawNode* drawbox;
bool aimode = false;

class $modify(AIEditor, EditorUI) {
	bool init(LevelEditorLayer* editor) {
		if(!EditorUI::init(editor)) return false;

		auto menu = (CCMenu*)this->getChildren()->objectAtIndex(9);
		auto aibtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_deSelBtn2_001.png"), this, SEL_MenuHandler(&AIEditor::onAI));

		menu->addChild(aibtn);
		menu->alignItemsHorizontallyWithPadding(48);

		return true;
	}

	void onAI(CCObject* obj) {
		return;
	}
};

class $modify(LevelEditorLayer){
	bool init(GJGameLevel* level) {
		if(!LevelEditorLayer::init(level)) return false;

		drawbox = CCDrawNode::create();
		this->m_drawGridLayer->addChild(drawbox);

		return true;
	}

	CCArray* objectsInRect(cocos2d::CCRect rect, bool ignoreLayer) {
		if(aimode) {
			drawbox->clear();

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

			drawbox->drawPolygon(rectangle, 4, {1.0f, 1.0f, 1.0f, 0.0f}, 1.0f, {1.0f, 1.0f, 0.0f, 1.0f});
			
			return CCArray::create();
		} else {
			return LevelEditorLayer::objectsInRect(rect, ignoreLayer);
		}
	}
};