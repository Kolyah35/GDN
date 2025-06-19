#pragma once

#include <Geode/Geode.hpp>
#include <array>

namespace GDNGlobal {
	template<typename T>
	std::vector<T *> findInstancesOfObj(cocos2d::CCNode *node) {
		cocos2d::CCArray *children = node->getChildren();

		std::vector<T *> objs = {};

		for (int i = 0; i < children->count(); i++) {
			cocos2d::CCObject *_obj = children->objectAtIndex(i);

			T *dyn = geode::cast::typeinfo_cast<T *>(_obj);

			if (dyn != nullptr) {
				objs.push_back(dyn);
			}
		}

		return objs;
	}

	template<typename T>
	std::vector<T *> convertArrayIntoVector(cocos2d::CCArray *array) {
		std::vector<T *> vec = {};

		for (int i = 0; i < array->count(); i++) {
			cocos2d::CCObject *_obj = array->objectAtIndex(i);

			T *dyn = geode::cast::typeinfo_cast<T *>(_obj);

			if (dyn != nullptr){
				vec.push_back(dyn);
			}
		}

		return vec;
	}

	extern cocos2d::CCArray *selectedObjects;
	extern GJBaseGameLayer *baseGameLayer;
	extern std::array<gd::string, 595> tempArray1;
	extern std::array<void *, 595> tempArray2;

	void accessSelectedObjects();
	void clearArrayWithoutCleanup(cocos2d::CCArray *array);
	cocos2d::CCRect createOriginRect(const std::vector<GameObject *> &objects);
	std::vector<GameObject *> copyObjectsWithRelativePos();
	GameObject *copyGameObject(GameObject *_obj);
	std::vector<std::string> splitString(const char *str, char d);
	std::map<int, std::string> parseObjectData(const std::string &object_string);
	void deleteObjectVector(const std::vector<GameObject *> &objects);
}
