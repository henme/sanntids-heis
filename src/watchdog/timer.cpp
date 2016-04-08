#include "timer.hpp"
Timer::Timer() {
    endTime = 0;
}
void Timer::set(int seconds) {
    endTime = time(0)+seconds;
}
int Timer::check(){
    if(endTime==0) {
        return 0;
    } else if(time(0) >= endTime) {
        endTime=0;
        return 1;
    }
    return 0;
}
