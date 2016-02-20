#include <iostream>
#include <string>
#include "tcpCom.hpp"

int main () {
  tcpCom com;
  std::string msg = "Test";
  com.send(msg);
  return 0;
}
