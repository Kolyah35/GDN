#include "GDNLayer.hpp"

#include <Geode/utils/web.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

using namespace geode::prelude;

bool GDNLayer::init() {
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

    web::AsyncWebRequest()
        .fetch("https://pastebin.com/raw/vNi1WHNF")
        .text()
        .then([this](std::string const& data) {
            close();
            onSuccess(data);
        })
        .expect([this](std::string const& error) {
            close();
            onError(error);
        });

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

    removeMeAndCleanup();
}
void GDNLayer::onSuccess(std::string response) {
    onLoginFailure();
}

void GDNLayer::onLoginSuccess() {
    if (_callback != nullptr) {
        _callback(this);
    }

    if (_closeOnFullSuccess) return;
}

void GDNLayer::loginFailureMessage() {
    FLAlertLayer *l = FLAlertLayer::create(new GDNLayerProtocol(), "GDN", "You are not <cp>registered in GD Neural.</c> Do you want to <cy>join</c> GDN's <cj>Discord server</c>?", "No", "Yes");
    l->show();
}
void GDNLayer::onLoginFailure() {
    loginFailureMessage();

    removeMeAndCleanup();
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