#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/FLAlertLayerProtocol.hpp>

#include "AIMenu.hpp"
#include "GDNLayer.hpp"
#include "GDNGlobal.hpp"

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

CCArray *GDNGlobal::selectedObjects = nullptr;
GJBaseGameLayer *GDNGlobal::baseGameLayer = nullptr;
std::array<gd::string, 595> GDNGlobal::tempArray1;
std::array<void *, 595> GDNGlobal::tempArray2;

void GDNGlobal::accessSelectedObjects() {
	if (selectedObjects == nullptr) {
		selectedObjects = CCArray::create();
		selectedObjects->retain();
	}
}
void GDNGlobal::clearArrayWithoutCleanup(CCArray *array) {
	while (array->count() != 0) {
		array->removeObjectAtIndex(0, false);
	}
}
cocos2d::CCRect GDNGlobal::createOriginRect(const std::vector<GameObject *> &objects) {
	float min_x = 0, min_y = 0;
	float max_x = 0, max_y = 0;

	std::vector<float> x_vec;
	std::vector<float> y_vec;

	CCRect rect = {};

	if (objects.size() == 0) return rect;

	for (GameObject *game_object : objects) {
		x_vec.push_back(game_object->getPositionX());
		y_vec.push_back(game_object->getPositionY());
	}

	min_x = *(std::min_element(x_vec.begin(), x_vec.end()));
	min_y = *(std::min_element(y_vec.begin(), y_vec.end()));

	max_x = *(std::max_element(x_vec.begin(), x_vec.end()));
	max_y = *(std::max_element(y_vec.begin(), y_vec.end()));

	log::info("min_x={}; min_y={}; max_x={}; max_y={};", min_x, min_y, max_x, max_y);

	rect.origin.x = min_x - 30;
	rect.origin.y = min_y - 30;
	rect.size.width = max_x - min_x + 60;
	rect.size.height = max_y - min_y + 60;

	return rect;
}
std::vector<GameObject *> GDNGlobal::copyObjectsWithRelativePos() {
	std::vector<GameObject *> result = {};

	float min_x;
	float min_y;

	std::vector<float> x_vec;
	std::vector<float> y_vec;

	accessSelectedObjects();

	for (int i = 0; i < selectedObjects->count(); i++) {
		CCObject *base_object = selectedObjects->objectAtIndex(i);
		GameObject *game_object = typeinfo_cast<GameObject *>(base_object);

		if (game_object == nullptr) continue;

		GameObject *new_object = copyGameObject(game_object);
		new_object->retain();

		result.push_back(new_object);

		x_vec.push_back(game_object->getPositionX());
		y_vec.push_back(game_object->getPositionY());
	}

	if (result.size() == 0) return result;

	min_x = *(std::min_element(x_vec.begin(), x_vec.end()));
	min_y = *(std::min_element(y_vec.begin(), y_vec.end()));

	log::info("min_x={}; min_y={}", min_x, min_y);

	for (GameObject *ref : result) {
		auto pos = ref->getPosition();

		pos.x -= min_x;
		pos.y -= min_y - 90.f;

		ref->setPosition(pos);
	}

	return result;
}
GameObject *GDNGlobal::copyGameObject(GameObject *_obj) {
	if (!_obj) return nullptr;

	tempArray1.fill("");
	tempArray2.fill(nullptr);

	std::string object_string = _obj->getSaveString(baseGameLayer);

	std::map<int, std::string> object_map = parseObjectData(object_string);

	for (auto [k, v] : object_map) {
		tempArray1[k] = v;
		tempArray2[k] = baseGameLayer;
	}

	std::vector v1(tempArray1.begin(), tempArray1.end());
	std::vector v2(tempArray2.begin(), tempArray2.end());

	GameObject *new_object = GameObject::objectFromVector(
		v1, v2, baseGameLayer, false
	);

	return new_object;
}
std::vector<std::string> GDNGlobal::splitString(const char *str, char d) {
	std::vector<std::string> result;

	do {
		const char *begin = str;

		while(*str != d && *str) str++;

		result.push_back(std::string(begin, str));
	} while (0 != *str++);

	return result;
}

std::map<int, std::string> GDNGlobal::parseObjectData(const std::string &object_string) {
	std::vector<std::string> data = splitString(object_string.data(), ',');

	std::map<int, std::string> object_map;

	bool _key = true;

	int key;
	std::string value;

	for (std::string el : data) {
		if (_key) {
			key = std::stoi(el);
		} else {
			value = el;

			object_map[key] = value;
		}

		_key = !_key;
	}

	return object_map;
}
void GDNGlobal::deleteObjectVector(const std::vector<GameObject *> &objects) {
	for (GameObject *obj : objects) {
		obj->release();
	}
}

namespace AISettings {
	bool loginSuccessful = false;
	bool attemptedToLogin = false;
	std::string failureReason;
	bool loginRequested = false;
}

extern bool aimenu_exists;

class $modify(AIEditor, EditorUI) {
	bool init(LevelEditorLayer* editor) {
		if(!EditorUI::init(editor)) return false;

		// auto menu = (CCMenu*)this->getChildren()->objectAtIndex(24);

		auto btn_spr = EditorButtonSprite::createWithSprite(
			"gdn2.png"_spr,
			1.f,
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

	void reloadDrawbox() {
		AIMenu::m_drawbox->clear();

		auto objects = GDNGlobal::convertArrayIntoVector<GameObject>(GDNGlobal::selectedObjects);
		auto rect = GDNGlobal::createOriginRect(objects);

		rect.origin.x = (rect.origin.x - ALIGN(rect.origin.x));
		rect.origin.y = (rect.origin.y - ALIGN(rect.origin.y));
		rect.size.width += (30 - ALIGN(rect.size.width));
		rect.size.height += (30 - ALIGN(rect.size.height));

		CCPoint rectangle[4] = {
			{rect.origin.x, rect.origin.y},
			{rect.origin.x + rect.size.width, rect.origin.y},
			{rect.origin.x + rect.size.width, rect.origin.y + rect.size.height},
			{rect.origin.x, rect.origin.y + rect.size.height}
		};

		AIMenu::m_drawbox->drawPolygon(rectangle, 4, {0.0f, 0.0f, 0.0f, 0.0f}, 1.0f, {1.0f, 1.0f, 0.0f, 1.0f});
	}

	void selectObject(GameObject *obj, bool p1) {
		log::info("hook! p1={}", p1);

		EditorUI::selectObject(obj, p1);

		GDNGlobal::accessSelectedObjects();
		GDNGlobal::clearArrayWithoutCleanup(GDNGlobal::selectedObjects);

		if (m_selectedObjects->count() == 0) {
			GDNGlobal::selectedObjects->addObject(obj);
		}

		// if(AIMenu::m_aiMode && !AIMenu::m_aiSelectObjects) {
			reloadDrawbox();
		// }
	}

	void selectObjects(CCArray *p0, bool p1) {
		log::info("hook! p0={}; p1={}", p0->count(), p1);

		EditorUI::selectObjects(p0, p1);

		GDNGlobal::accessSelectedObjects();
		GDNGlobal::clearArrayWithoutCleanup(GDNGlobal::selectedObjects);

		GDNGlobal::selectedObjects->addObjectsFromArray(m_selectedObjects);

		// if(AIMenu::m_aiMode && !AIMenu::m_aiSelectObjects) {
			reloadDrawbox();
		// }
	}

	void deselectAll() {
		EditorUI::deselectAll();

		GDNGlobal::accessSelectedObjects();
		GDNGlobal::clearArrayWithoutCleanup(GDNGlobal::selectedObjects);

		log::info("EditorUI::deselectAll();");

		// if(AIMenu::m_aiMode && !AIMenu::m_aiSelectObjects) {
			reloadDrawbox();
		// }
	}

	void deselectObject(GameObject *p0) {
		EditorUI::deselectObject(p0);

		GDNGlobal::accessSelectedObjects();

		if (GDNGlobal::selectedObjects->containsObject(p0)) {
			GDNGlobal::selectedObjects->removeObject(p0, false);
		}

		// auto position = p0->getRealPosition();
		// auto structures = GDNGlobal::getStructuresOnPosition(position);

		// log::info("found {} structures", structures.size());

		// if (structures.size() != 0) {
		// 	for (struct GDNGlobal::CollectionStructure &structure : structures) {
		// 		GDNGlobal::removeStructureFromList(structure);
		// 	}
		// }
		// log::info("EditorUI::deselectObject({}); | Pos={}", p0, p0->getRealPosition());
	}

	void onAI(CCObject* obj) {
		if (aimenu_exists) return;
		// if (AISettings::loginRequested) return;

		if (AISettings::attemptedToLogin) {
			if (!AISettings::loginSuccessful) {
				// GDNLayer::loginFailureMessage();
				GDNLayer::sendAlert(AISettings::failureReason);

				return;
			} else {
				auto aiMenu = AIMenu::create(300, 130);
				this->getParent()->addChild(aiMenu);
			}
		} else {
			AISettings::loginRequested = true;

			auto l = GDNLayer::create();

			l->setCloseOnFullSuccess(true);
			l->setURL("https://example.com");
			// l->withGDAuthentication();
			l->begin();

			l->setCallback([this, obj] (GDNLayer *l2) {
				AISettings::loginRequested = false;
				AISettings::loginSuccessful = !l2->isFailed();
				AISettings::attemptedToLogin = true;
				if (!AISettings::loginSuccessful) {
					AISettings::failureReason = l2->getReturnedFailureMessage();
				}
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
			0.8f,
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
		// if (AISettings::loginRequested) return;

		AISettings::loginRequested = true;

		auto l = GDNLayer::create();

		l->setURL("https://example.com");
		l->withGDAuthentication();
		l->begin();

		l->setCallback([this](GDNLayer* l2) {
			AISettings::attemptedToLogin = true;
			AISettings::loginSuccessful = !l2->isFailed();
			if (!AISettings::loginSuccessful) {
				AISettings::failureReason = l2->getReturnedFailureMessage();
			}
			AISettings::loginRequested = false;
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

		GDNGlobal::baseGameLayer = this;

		return true;
	}

#ifndef _WIN32
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
			// AIMenu::m_selectedRect = rect;
			AIMenu::m_ignoreLayer = ignoreLayer;

			return CCArray::create();
		} else {
			AIMenu::m_aiSelectObjects = false;
			return LevelEditorLayer::objectsInRect(rect, ignoreLayer);
		}
	}
#endif
};

// class MySettingValue : StringSettingValue {
// public:

// };

// $execute {
// 	auto urlSetting = Mod::get()->getSettingDefinition("url");

// }
