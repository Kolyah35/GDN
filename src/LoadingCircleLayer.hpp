#pragma once

#include <Geode/Geode.hpp>
#include <string>

class LoadingCircleLayer : public cocos2d::CCLayer {
public:
    cocos2d::CCSprite *m_pCircle;

    bool init();
    CREATE_FUNC(LoadingCircleLayer);

    void update(float delta);
};