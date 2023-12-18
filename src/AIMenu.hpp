#pragma once
#include <cocos2d.h>

class AIMenu : public FLAlertLayer {
public:
    static AIMenu* create(float w, float h, const char* spr = "GJ_square01.png");

protected:
    cocos2d::CCSize m_layerSize;
    bool init(float w, float h, const char* spr = "GJ_square01.png");

    void setup();

    void onClose(cocos2d::CCObject*);
};