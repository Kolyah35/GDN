#pragma once
#include <Geode/Geode.hpp>
// #include <cocos-ext.h>
#include <Geode/ui/Notification.hpp>
#include <string>

class AIMenu : public FLAlertLayer {
public:
    static inline bool m_aiMode;
    static inline bool m_aiSelectObjects;
    static inline bool m_swipeEnabled;
    static inline bool m_ignoreLayer;
    static inline int m_currentMode;
    static inline cocos2d::CCArray* m_invisibleArray;
    static inline cocos2d::CCDrawNode* m_drawbox;
    static inline cocos2d::CCRect m_selectedRect;
    static inline geode::Notification* notification;

    static AIMenu* create(float w, float h, const char* spr = "GJ_square01.png");

protected:
    cocos2d::CCSize m_layerSize;
    
    bool init(float w, float h, const char* spr = "GJ_square01.png");

    void setup();

    void onClose(cocos2d::CCObject*);

    void selectAreaClicked(cocos2d::CCObject*);
    void okButtonClicked(CCObject*); 
    void onSendBtn(CCObject*);

    void onHttpCallback(cocos2d::extension::CCHttpClient* client, cocos2d::extension::CCHttpResponse* response);
    
    std::string createColorTrigger(int colId, cocos2d::ccColor3B col, float dur);
    std::string createStandardObject(cocos2d::CCPoint pos, int id, int z, int l, float scaleX, float scaleY, int baseCol, int detailCol);
};
