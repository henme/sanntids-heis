#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include "network.hpp"

int main () {
    network com(8002);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::string sendmsg = "Test";
    while(1){
	    com.send(sendmsg);
	    std::vector<std::string> msg = com.get_messages();
		for (auto i = msg.begin(); i != msg.end(); ++i){
	    	std::cout << *i << "\n";
		}
	    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    puts("Press any key to exit...");
    getc(stdin);
    return 0;
}
