#pragma once
#include <cocos2d.h>

class AIMenu : public FLAlertLayer, TextInputDelegate {
public:
    static inline bool m_aiMode;
    static inline bool m_aiSelectObjects;
    static inline bool m_swipeEnabled;
    static inline bool m_ignoreLayer;
    static inline int m_currentMode;
    static inline cocos2d::CCArray* m_invisibleArray;
    static inline cocos2d::CCDrawNode* m_drawbox;
    static inline cocos2d::CCRect m_selectedRect;

    static AIMenu* create(float w, float h, const char* spr = "GJ_square01.png");

protected:
    cocos2d::CCSize m_layerSize;

    bool init(float w, float h, const char* spr = "GJ_square01.png");

    void setup();

    void onClose(cocos2d::CCObject*);

    void textChanged(CCTextInputNode* p0) override;

    void selectAreaClicked(cocos2d::CCObject*);
    void okButtonClicked(CCObject*); 
    void onSendBtn(CCObject*);
};