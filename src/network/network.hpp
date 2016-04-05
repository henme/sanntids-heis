#pragma once

#include <string>
#include <vector>
#include <map>

class network {

  std::vector<std::string> InnboundMessages;
  std::map<std::string, bool> connectedPeers;
  int port;
  std::string ip; 

  private:
  	void connectionHandler();
    void heartbeat();
  	void respond();
   	void recieve();
    void udpBroadcaster();

  public:
  	network(int port, std::string ip);
  	void send(std::string msg);
    std::vector<std::string> get_messages();
    std::map<std::string, bool> get_listofPeers();
};	
