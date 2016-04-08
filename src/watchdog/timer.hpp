#pragma once
#include <ctime>
class Timer {
    private:
        std::time_t endTime;
    public:
        Timer();
        void set(int seconds);
        int check();
};
