#pragma once

#include <string>
#include <vector>

class network {

  std::vector<std::string> InnboundMessages;

  private:
  	void init();
  	void connectionHandler();
  	void udpBroadcaster();
  	void respond();
   	void recieve();

  public:
  	network();
  	void send(std::string msg);
  	std::vector<std::string> get_messages(){return InnboundMessages;};

};	
