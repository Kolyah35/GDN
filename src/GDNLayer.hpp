#pragma once

#include <Geode/Geode.hpp>
#include <string>
#include <functional>
#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include "LoadingCircleLayer.hpp"

class GDNLayerProtocol : public FLAlertLayerProtocol {
    void FLAlert_Clicked(FLAlertLayer *l, bool idk) override;
};

class GDNLayer : public cocos2d::CCLayer, public GDNLayerProtocol {
protected:
    LoadingCircleLayer *_circle;
    cocos2d::CCSprite *_bg;

    bool _closeOnFullSuccess = false;

    std::function<void(GDNLayer*)> _callback = nullptr;
public:
    CREATE_FUNC(GDNLayer);

    void setCloseOnFullSuccess(bool state);
    void setCallback(std::function<void(GDNLayer*)> callback);

    bool init() override;
    void close();

    void onError(std::string response);
    void onSuccess(std::string response);

    void onLoginSuccess();
    void onLoginFailure();

    static void loginFailureMessage();
};