#include <iostream>
#include <string>
#include <queue>
#include <boost/asio.hpp>
#include <boost/asio/ip/udp.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

typedef boost::shared_ptr<tcp::socket> socket_ptr;
typedef boost::shared_ptr<string> string_ptr;
typedef boost::shared_ptr< queue<string_ptr> > messageQueue_ptr;

int main () {
    io_service service;
    tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), 8001));
    messageQueue_ptr messageQueue(new queue<string_ptr>);

    cout << "Waiting for main to connect..." << endl;
    socket_ptr clientSock(new tcp::socket(service));
    acceptor.accept(*clientSock); 
    cout << "Main connected! ";

    int bytesRead = 0;
    char readBuf[1024] = {0};
/*
    for(;;)
    {
        if(clientSock->available())
        {
            bytesRead = clientSock->read_some(buffer(readBuf, 256));
            string_ptr msg(new string(readBuf, bytesRead));

            messageQueue->push(msg);
        }
        if(!messageQueue->empty())
        {
            cout << "\n" + *(messageQueue->front());
        }
        messageQueue->pop();
    }
*/
    if(0){
      for(int i = 1; i < 5; i++){
      std::cout << i << std::endl;
      }
    }
    puts("Press any key to exit...");
    getc(stdin);
    return 0;
}

