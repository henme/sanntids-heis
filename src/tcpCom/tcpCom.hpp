#pragma once

#include <string>

class tcpCom {
  private:
  	void init();
  	void connectionHandler();
  	void udpBroadcaster();
  	void respond();
   	void recieve();

  public:
  	tcpCom();
  	void send(std::string msg);

};	
