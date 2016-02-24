#pragma once

#include <string>

class network {
  private:
  	void init();
  	void connectionHandler();
  	void udpBroadcaster();
  	void respond();
   	void recieve();

  public:
  	network();
  	void send(std::string msg);

};	
