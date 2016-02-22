#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "tcpCom.hpp"

int main () {
    tcpCom com;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::string msg = "Test";
    com.send(msg);
    puts("Press any key to exit...");
    getc(stdin);
    return 0;
}
