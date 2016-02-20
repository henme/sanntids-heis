#pragma once

#include <string>

class tcpCom {
  private:
  	void init();
  	void broadcast();
   	void recieve();

  public:
  	tcpCom();
  	void send(std::string msg);

};	
