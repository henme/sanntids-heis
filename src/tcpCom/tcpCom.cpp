#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <cstdlib>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "tcpCom.hpp"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

typedef boost::shared_ptr<tcp::socket> socket_ptr;
typedef boost::shared_ptr<string> string_ptr;
typedef map<socket_ptr, string_ptr> clientMap;
typedef boost::shared_ptr<clientMap> clientMap_ptr;
typedef boost::shared_ptr< list<socket_ptr> > clientList_ptr;
typedef boost::shared_ptr< queue<clientMap_ptr> > messageQueue_ptr;

io_service service;
tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), 8001));
boost::mutex mtx;
clientList_ptr clientList(new list<socket_ptr>);
messageQueue_ptr messageQueue(new queue<clientMap_ptr>) ;

const int bufSize = 1024;

tcpCom::tcpCom(){
	tcpCom::init();
}

void tcpCom::init(){
    //Start approriate threads
    new boost::thread(bind(&tcpCom::connectionHandler, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));
    new boost::thread(bind(&tcpCom::recieve, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));
    new boost::thread(bind(&tcpCom::respond, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));        
    new boost::thread(bind(&tcpCom::udpBroadcaster, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));        
}

void tcpCom::connectionHandler(){
    cout << "Waiting for peers..." << endl;

    for(;;)
    {
        socket_ptr clientSock(new tcp::socket(service));

        acceptor.accept(*clientSock);

        cout << "New peer joined! ";

        mtx.lock();
        clientList->emplace_back(clientSock);
        mtx.unlock();

        cout << clientList->size() << " total peers" << endl;
    }
}

void tcpCom::send(string msg){
    string_ptr msgptr(new string(msg));
    clientMap_ptr cm(new clientMap);
    cm->insert(pair<socket_ptr, string_ptr>(NULL, msgptr));
    messageQueue->push(cm);
}

void tcpCom::respond(){
    for(;;)
    {
        if(!messageQueue->empty())
        {
            auto message = messageQueue->front();

            mtx.lock();
            for(auto& clientSock : *clientList)
            {
                clientSock->write_some(buffer(*(message->begin()->second), bufSize));
            }
            mtx.unlock();

            mtx.lock();
            messageQueue->pop();
            mtx.unlock();
        }

        boost::this_thread::sleep( boost::posix_time::millisec(200));
    }
}

void tcpCom::recieve(){
    for(;;)
    {
        if(!clientList->empty())
        {
            mtx.lock();
            for(auto& clientSock : *clientList)
            {
                if(clientSock->available())
                {
                    char readBuf[bufSize] = {0};

                    int bytesRead = clientSock->read_some(buffer(readBuf, bufSize));

                    string_ptr msg(new string(readBuf, bytesRead));

                    clientMap_ptr cm(new clientMap);
                    cm->insert(pair<socket_ptr, string_ptr>(clientSock, msg));

                    messageQueue->push(cm);

                    cout << "LOG: " << *msg << endl;
                }
            }
            mtx.unlock();
        }

        boost::this_thread::sleep( boost::posix_time::millisec(200));
    }
}

void tcpCom::udpBroadcaster(){
	//UDP broadcast, "Connect to me!"
    io_service udpService;
    udp::socket socket(udpService);
    udp::endpoint remote_endpoint;
    socket_base::broadcast option(true);
    //socket.set_option(option);
    //udp::socket udpsock(udpService, udp::endpoint(udp::v4(), 13));
    //remote_endpoint = udp::endpoint(udp::v4(), 13);
    
    //boost::system::error_code ignored_error;

    //socket.send_to(buffer("message", 7), remote_endpoint, 0); //ignored_error);
    for (;;){
        //char data[1024];
        //udp::endpoint sender_endpoint;
        //size_t length = udpsock.receive_from(buffer(data, 1024), sender_endpoint);
        //udpsock.send_to(buffer(data, length), sender_endpoint);
    }
}
