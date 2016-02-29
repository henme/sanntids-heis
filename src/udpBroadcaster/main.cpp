#include <boost/asio.hpp>

int main()
{
    namespace ip = boost::asio::ip;
    boost::asio::io_service io_service;
    
    // Server binds to any address and any port.
    ip::udp::socket socket(io_service,
                           ip::udp::endpoint(ip::udp::v4(), 0));
    socket.set_option(boost::asio::socket_base::broadcast(true));
    
    // Broadcast will go to port 8888.
    ip::udp::endpoint broadcast_endpoint(ip::address_v4::broadcast(), 8888);
    
    // Broadcast data.
    char data[9] ={0};
    socket.send_to(boost::asio::buffer(data), broadcast_endpoint);
}
