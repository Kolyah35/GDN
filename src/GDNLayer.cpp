#include "GDNLayer.hpp"

#include <Geode/utils/web.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GJAccountManager.hpp>

using namespace geode::prelude;

cocos2d::CCScene *GDNLayer::_workingScene = nullptr;

/**
 * static std::string _failure;
    static std::string _returnedFailure;
    static std::string _success;
    static bool _failed;
*/

std::string GDNLayer::_failure = "You are not <cp>registered in GD Neural.</c> Do you want to <cy>join</c> GDN's <cj>Discord server</c>?";
std::string GDNLayer::_returnedFailure = "";
std::string GDNLayer::_success = "Success! <cg>Access granted.</c>";
bool GDNLayer::_failed = false;

bool GDNLayer::init() {
    _protocol = this;
    _failure = "You are not <cp>registered in GD Neural.</c> Do you want to <cy>join</c> GDN's <cj>Discord server</c>?";
    _success = "Success! <cg>Access granted.</c>";

    CCLayer *l = cocos2d::CCLayer::create();

    l->setID("main-layer");

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

    _workingScene = cocos2d::CCScene::get();

    return true;
}

void GDNLayer::close() {
    auto scene = cocos2d::CCScene::get();

    if (_workingScene != scene) return;

    _circle->removeMeAndCleanup();
    _bg->runAction(cocos2d::CCFadeTo::create(0.15f, 0));
    
    void *nptr = this->getChildByID("info-label");
    if (nptr != nullptr) {
        CCNode *n = typeinfo_cast<CCNode *>(this->getChildByID("info-label"));   
    
        n->removeMeAndCleanup();
    }
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
    setupNetworking();

    if (_requireGDA) {
        beginGD();

        return;
    }

    beginN();
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

void GDNLayer::withGDAuthentication() {
    _requireGDA = true;

    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

    auto bmf = cocos2d::CCLabelBMFont::create("Verifying GD Account Identity...", "goldFont.fnt");
	bmf->setScale(0.5f);
	bmf->setPositionX(winSize.width / 2);
	bmf->setPositionY((winSize.height / 2) + _circle->m_pCircle->getContentSize().height - 10.f);
    bmf->setID("info-label");

    bmf->setOpacity(0);
    bmf->runAction(cocos2d::CCFadeTo::create(0.5f, 255));

    addChild(bmf);
}

void GDNLayer::beginGD() {
    // beginN();

    // return;

    _currentService = GeometryDash;

    GJAccountManager *manager = GJAccountManager::get();

    std::string gjp = manager->m_GJP2;
    std::string username = manager->m_username;
    // int aid = manager->m_accountID;

    web::WebRequest req = web::WebRequest();

    req.userAgent("");
    req.bodyString(fmt::format("udid=S93972361&userName={}&gjp2={}&secret=Wmfv3899gc9", username, gjp));
    req.timeout(std::chrono::seconds(10));

    auto task = req.post("https://www.boomlings.com/database/accounts/loginGJAccount.php");

    _listener.setFilter(task);
}
void GDNLayer::beginN() {
    _currentService = GDNetwork;

    void *nptr = this->getChildByID("info-label");
    if (nptr != nullptr) {
        cocos2d::CCLabelBMFont *n = typeinfo_cast<cocos2d::CCLabelBMFont *>(this->getChildByID("info-label"));   
    
        n->setString("Verifying GDN Account...");
    }

    web::WebRequest req = web::WebRequest();
    req.timeout(std::chrono::seconds(10));

    _listener.setFilter(req.get(_url));
}

void GDNLayer::setupNetworking() {
    if (_networkInstalled) return;

    _networkInstalled = true;

    _listener.bind([this](web::WebTask::Event *e) {
        if (web::WebResponse *value = e->getValue()) {
            std::string default_value;
            if (this->_currentService == GDNLayer::GeometryDash) {
                default_value = "-1";
            } else {
                default_value = "403";
            }

            std::string res = value->string().unwrapOr(default_value);

            if (this->_currentService == GDNLayer::GDNetwork) {
                if (!value->ok()) {
                    this->close();
                    this->onError(res);

                    return;
                }

                this->close();
                this->onSuccess(res);

                return;
            }

            if (!value->ok()) {
                this->close();
                this->onError(res);

                return;
            }

            if (res.at(0) == '-') {
                close();

                std::string msg = fmt::format("<cr>Cannot verify</c> GD account: <cy>{}</c>", res);
                _returnedFailure = msg;
                _failed = true;

                if (this->_callback != nullptr) {
                    this->_callback(this);
                }

                FLAlertLayer *l = FLAlertLayer::create("GDN", msg, "OK");
                l->show();

                return;
            }

            this->beginN();
        }

        else if (e->isCancelled()) {
            this->close();
            this->onError("Authorization has been canceled");

            return;
        }
    });
}