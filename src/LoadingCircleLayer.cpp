#include "LoadingCircleLayer.hpp"

bool LoadingCircleLayer::init() {
    m_pCircle = cocos2d::CCSprite::create("loadingCircle.png");
    this->addChild(m_pCircle);

    m_pCircle->setBlendFunc({GL_SRC_ALPHA, GL_ONE});

    scheduleUpdate();

    return true;
}

void LoadingCircleLayer::update(float delta) {
    if(m_pCircle) {
        float rot = m_pCircle->getRotation();
        rot += 3;
        m_pCircle->setRotation(rot);
    }
}