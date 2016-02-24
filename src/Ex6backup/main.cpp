#include <iostream>
#include <string>
#include <queue>
#include <boost/asio.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/thread.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

typedef boost::shared_ptr<tcp::socket> socket_ptr;
typedef boost::shared_ptr<string> string_ptr;
typedef boost::shared_ptr< queue<string_ptr> > messageQueue_ptr;

int main () {
    //Backup kicks inn if no ack from main
    io_service service;
    tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), 8001));
    messageQueue_ptr messageQueue(new queue<string_ptr>);

    cout << "Waiting for main to connect..." << endl;
    socket_ptr clientSock(new tcp::socket(service));
    acceptor.accept(*clientSock); 
    cout << "Main connected! " << endl;

    int bytesRead = 0;
    char readBuf[1024] = {0};
    int i = 1;
    while(1){
        try
        {
            bytesRead = clientSock->read_some(buffer(readBuf, 256));
            string_ptr msg(new string(readBuf, bytesRead));

            messageQueue->push(msg);
            if(!messageQueue->empty())
            {
                i = stoi(*(messageQueue->front()));
            }
            messageQueue->pop();
        }
        catch(std::exception& e)
        {
            //cerr << "\n" << e.what() << endl;
            break;
        }
    }
    i++;
    for(i=i; i < 5; i++){
        cout << i << endl;
        boost::this_thread::sleep( boost::posix_time::millisec(1000));
    }
    return 0;
}

