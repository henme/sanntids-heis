#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "network.hpp"

int main () {
    network com;
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    std::string msg = "Test\n";
    com.send(msg);
    puts("Press any key to exit...");
    getc(stdin);
    return 0;
}
