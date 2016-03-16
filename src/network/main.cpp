#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include "network.hpp"
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

int main () {
    network com(8001, "127.0.0.1");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    ptree pt;
    pt.put ("foo", "bar");
    ostringstream buf; 
    write_json (buf, pt, false);
    string json = buf.str(); // {"foo":"bar"}

    while(1){
	    com.send(json);
	    std::vector<std::string> msg = com.get_messages();
		for (auto i = msg.begin(); i != msg.end(); ++i){
	    	std::cout << *i << "\n";
		}
/*
        // Read json.
        ptree pt2;
        istringstream is (json);
        read_json (is, pt2);
        string foo = pt2.get<string> ("foo");
        cout << foo << std::endl;
*/
        /*
        std::vector<std::string> peers = com.get_listofPeers();
        for (auto i = peers.begin(); i != peers.end(); ++i){
            std::cout << *i << "\n";
        }
        */
	    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    puts("Press any key to exit...");
    getc(stdin);
    return 0;
}
