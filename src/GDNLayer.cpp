#include "GDNLayer.hpp"

#include <Geode/utils/web.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

using namespace geode::prelude;

bool GDNLayer::init() {
    _protocol = this;
    _failure = "You are not <cp>registered in GD Neural.</c> Do you want to <cy>join</c> GDN's <cj>Discord server</c>?";
    _success = "Success! <cg>Access granted.</c>";

    CCLayer *l = cocos2d::CCLayer::create();

    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
    auto base = cocos2d::CCSprite::create("square.png");

    base->setPosition({ 0, 0 });
    base->setScale(500.f);
    base->setColor({0, 0, 0});
    base->setOpacity(0);
    base->runAction(cocos2d::CCFadeTo::create(0.5f, 125));

    auto loadingCircle = LoadingCircleLayer::create();

    loadingCircle->m_pCircle->setOpacity(0);
    loadingCircle->m_pCircle->runAction(cocos2d::CCFadeTo::create(0.5f, 255));
    loadingCircle->setPosition(winSize.width / 2, winSize.height / 2 );
    addChild(base, -1);

    l->addChild(loadingCircle, 1);

    _circle = loadingCircle;
    _bg = base;

    addChild(l, 1024);

    return true;
}

void GDNLayer::close() {
    _circle->removeMeAndCleanup();
    _bg->runAction(cocos2d::CCFadeTo::create(0.15f, 0));
    // _bg->removeMeAndCleanup();
}

void GDNLayer::onError(std::string response) {
    std::string msg = fmt::format("<cr>HTTP Error: {}</c>", response);

    FLAlertLayer *l = FLAlertLayer::create("Error", msg, "OK");
    l->show();
}
void GDNLayer::onSuccess(std::string response) {
    onLoginSuccess();
}

void GDNLayer::onLoginSuccess() {
    if (_callback != nullptr) {
        _callback(this);
    }

    if (_closeOnFullSuccess) return;

    FLAlertLayer *l = FLAlertLayer::create("GDN", _success, "OK");
    l->show();
}

void GDNLayer::loginFailureMessage() {
    sendAlert(_failure);

    _returnedFailure = _failure;
}
std::string GDNLayer::getReturnedFailureMessage() {
    return _returnedFailure;
}
void GDNLayer::onLoginFailure() {
    _failed = true;

    if (_callback != nullptr) {
        _returnedFailure = _failure;
        _callback(this);
    }

    loginFailureMessage();

    // removeMeAndCleanup();
}

void GDNLayerProtocol::FLAlert_Clicked(FLAlertLayer *l, bool idk) {
	if (idk != 1) return;

	ShellExecuteA(NULL, "open", "https://discord.gg/gdn-neiroset-dlia-geometry-dash-1115715187484414044", NULL, NULL, SW_SHOWNORMAL);
}

void GDNLayer::setCloseOnFullSuccess(bool state) {
    _closeOnFullSuccess = state;
}
void GDNLayer::setCallback(std::function<void(GDNLayer*)> callback) {
    _callback = callback;
}

void GDNLayer::setURL(std::string url) {
    _url = url;
}
void GDNLayer::begin() {
    web::AsyncWebRequest()
        .fetch(_url)
        .text()
        .then([this](std::string const& data) {
            close();
            onSuccess(data);
        })
        .expect([this](std::string const& error) {
            close();
            onError(error);
        });
}

void GDNLayer::setFailureMessage(std::string str) {
    _success = str;
}
void GDNLayer::setSuccessMessage(std::string str) {
    _failure = str;
}

// void GDNLayer::setButtonProvider(FLAlertLayerProtocol *protocol) {
//     _protocol = protocol;
// }

bool GDNLayer::isFailed() {
    return _failed;
}

void GDNLayer::sendAlert(std::string alert) {
    FLAlertLayer *l = FLAlertLayer::create(new GDNLayerProtocol(), "GDN", alert, "No", "Yes");
    l->show();
}