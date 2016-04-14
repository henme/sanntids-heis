#pragma once

#include <string>
#include <vector>
#include <map>
#include <utility> 
#include <tuple>
#include <ctime>

#include <boost/asio.hpp>
//#include <boost/asio/ip/address_v4.hpp>

using namespace boost::asio::ip;
using boost::asio::ip::address_v4;

typedef boost::shared_ptr<tcp::socket> socket_ptr;
typedef boost::shared_ptr<std::string> string_ptr;

class network {

  std::vector<std::pair<address_v4, std::string>> InnboundMessages;
  std::vector<std::pair<address_v4, bool>> connectedPeers;
  int port;
  address_v4 myIP; 

  private:
  	void connectionHandler();
    void heartbeat();
    void sendtoSocket(socket_ptr clientSock, std::string msg);
  	void tcpMessageBroadcaster();
   	void recieve();
    void messageParser(std::tuple<socket_ptr, time_t, address_v4> &clientSock, string_ptr msg);
    void udpBroadcaster();
    void udpListener();

  public:
  	network(int port, address_v4 myIP);
  	void send(std::string msg);
    std::vector<std::pair<address_v4, std::string>> get_messages();
    std::vector<std::pair<address_v4, bool>> get_listofPeers();
};	
