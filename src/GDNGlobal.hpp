#pragma once

#include <Geode/Geode.hpp>

namespace GDNGlobal {
	template<typename T>
	std::vector<T *> findInstancesOfObj(cocos2d::CCNode *node) {
		CCArray *children = node->getChildren();

		std::vector<T *> objs = {};

		for (int i = 0; i < children->count(); i++) {
			cocos2d::CCObject *_obj = children->objectAtIndex(i);

			T *dyn = dynamic_cast<T *>(_obj);

			if (dyn != nullptr) {
				objs.push_back(dyn);
			}
		}

		return objs;
	}
}