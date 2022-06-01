
#include <esp32-hal.h>
#include "NukiTimeout.h"

namespace Nuki
{
    void NukiTimeout::update()
    {
        if(isTimedOut())
        {
            for(auto subscriber : subsciptions)
            {
                subscriber->onTimeout();
            }

            reset();
        }
    }

    void NukiTimeout::reset()
    {
        lastStartTimeout = 0;
    }

    void NukiTimeout::extend()
    {
        lastStartTimeout = millis();
    }

    bool NukiTimeout::isTimedOut()
    {
        return lastStartTimeout != 0 && (millis() - lastStartTimeout > duration);
    }

    void NukiTimeout::subscribe(TimeoutSubscriber *subscriber)
    {
        subsciptions.push_back(subscriber);
    }

    void NukiTimeout::setDuration(uint32_t value)
    {
        duration = value;
    }
}