#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include "network.hpp"

int main () {
    network com;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::string sendmsg = "Test";
    com.send(sendmsg);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    /*std::vector<std::string> msg = com.get_messages();
	for (auto i = msg.begin(); i != msg.end(); ++i){
    	std::cout << *i << "\n";
	}*/
    puts("Press any key to exit...");
    getc(stdin);
    return 0;
}
