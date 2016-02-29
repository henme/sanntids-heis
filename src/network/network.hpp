#pragma once

#include <string>
#include <vector>

class network {

  std::vector<std::string> InnboundMessages;
  int port;
  std::string ip; 

  private:
  	void init();
  	void connectionHandler();
  	void udpBroadcaster();
  	void respond();
   	void recieve();

  public:
  	network(int port, std::string ip);
  	void send(std::string msg);
    std::vector<std::string> get_messages(){
      std::vector<std::string> messages = InnboundMessages;
      InnboundMessages = {};
      return messages;
    };

};	
