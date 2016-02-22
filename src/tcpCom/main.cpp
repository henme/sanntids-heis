#include <iostream>
#include <string>
#include "tcpCom.hpp"

int main () {
    tcpCom com;
    std::string msg = "Test";
    com.send(msg);
    puts("Press any key to exit...");
    getc(stdin);
    return 0;
}
