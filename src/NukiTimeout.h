#pragma once

#include <stdint.h>
#include <vector>

namespace Nuki
{
    class TimeoutSubscriber
    {
    public:
        virtual void onTimeout() = 0;
    };

    class NukiTimeout
    {
    public:
        void update();

        void reset();
        void extend();
        bool isTimedOut();

        void subscribe(TimeoutSubscriber* subscriber);
        void setDuration(uint32_t value); // ms

    private:

        uint32_t lastStartTimeout = 0;
        uint16_t duration = 1000;

        std::vector<TimeoutSubscriber*> subsciptions;
    };
}