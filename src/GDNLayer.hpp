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

    std::string _returnedData;
    std::string _url;

    FLAlertLayerProtocol *_protocol;

    bool _requireGDA = false;

    void beginGD();
    void beginN();
public:
    static cocos2d::CCScene *_workingScene;
    static std::string _failure;
    static std::string _returnedFailure;
    static std::string _success;
    static bool _failed;

    CREATE_FUNC(GDNLayer);

    void setURL(std::string url);
    void begin();
    
    void withGDAuthentication();

    // void setButtonProvider(FLAlertLayerProtocol *protocol);

    void setFailureMessage(std::string str);
    void setSuccessMessage(std::string str);

    void setCloseOnFullSuccess(bool state);
    void setCallback(std::function<void(GDNLayer*)> callback);

    bool init() override;
    void close();

    void onError(std::string response);
    void onSuccess(std::string response);

    void onLoginSuccess();
    void onLoginFailure();

    void loginFailureMessage();
    static void sendAlert(std::string alert);
    static void sendAlertWithOk(std::string alert);
    std::string getReturnedFailureMessage();

    std::string getReturnedData();

    bool isFailed();
};