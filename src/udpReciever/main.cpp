#include <iostream>

#include <boost/asio.hpp>

int main()
{
    namespace ip = boost::asio::ip;
    boost::asio::io_service io_service;
    
    // Client binds to any address on port 8888 (the same port on which
    // broadcast data is sent from server).
    ip::udp::socket socket(io_service,
                           ip::udp::endpoint(ip::udp::v4(), 8888 ));
    
    ip::udp::endpoint sender_endpoint;
    
    // Receive data.
    char data[9] ={0};
    std::size_t bytes_transferred =
    socket.receive_from(boost::asio::buffer(data), sender_endpoint);
    
    std::cout << "got " << bytes_transferred << " bytes." << std::endl;
}