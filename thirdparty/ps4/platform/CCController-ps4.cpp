#include "base/CCController.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_PS4)

NS_CC_BEGIN

void Controller::startDiscoveryController()
{
}

void Controller::stopDiscoveryController()
{
}

void Controller::registerListeners()
{
}

bool Controller::isConnected() const
{
    return false;
}

Controller::Controller()
    : _controllerTag(TAG_UNSET)
    , _impl(nullptr)
    , _connectEvent(nullptr)
    , _keyEvent(nullptr)
    , _axisEvent(nullptr)
{
    init();
}

Controller::~Controller()
{
    delete _connectEvent;
    delete _keyEvent;
    delete _axisEvent;
}

NS_CC_END

#endif
