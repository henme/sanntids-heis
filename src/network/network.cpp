#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <cstdlib>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "network.hpp"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

typedef boost::shared_ptr<tcp::socket> socket_ptr;
typedef boost::shared_ptr<string> string_ptr;
typedef map<socket_ptr, string_ptr> clientMap;
typedef boost::shared_ptr<clientMap> clientMap_ptr;
typedef boost::shared_ptr< list<socket_ptr> > clientList_ptr;
typedef boost::shared_ptr< queue<clientMap_ptr> > messageQueue_ptr;

const short port = 8001;
const int bufSize = 1024;

io_service service;
tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), port));
boost::mutex mtx;
clientList_ptr clientList(new list<socket_ptr>);
messageQueue_ptr InnMessageQueue(new queue<clientMap_ptr>) ;
messageQueue_ptr OutMessageQueue(new queue<clientMap_ptr>) ;

network::network(){
    network::init();
}

void network::init(){
    //Start approriate threads
    new boost::thread(bind(&network::connectionHandler, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));
    new boost::thread(bind(&network::recieve, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));
    new boost::thread(bind(&network::respond, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));        
    new boost::thread(bind(&network::udpBroadcaster, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));        
}

void network::connectionHandler(){
    cout << "Waiting for peers..." << endl;

    for(;;)
    {
        socket_ptr clientSock(new tcp::socket(service));

        acceptor.accept(*clientSock);

        cout << "New peer joined! " << endl;

        mtx.lock();
        clientList->emplace_back(clientSock);
        mtx.unlock();
        std::string s = clientSock->remote_endpoint().address().to_string();
        cout << s << " connected sucsessfully!" << endl;
    }
}

void network::send(string msg){
    string_ptr msgptr(new string(msg));
    clientMap_ptr cm(new clientMap);
    cm->insert(pair<socket_ptr, string_ptr>(NULL, msgptr));
    OutMessageQueue->push(cm);
}

void network::respond(){
    for(;;)
    {
        if(!OutMessageQueue->empty())
        {
            auto message = OutMessageQueue->front();

            mtx.lock();
            for(auto& clientSock : *clientList)
            {
                clientSock->write_some(buffer(*(message->begin()->second), bufSize));
            }
            mtx.unlock();

            mtx.lock();
            OutMessageQueue->pop();
            mtx.unlock();
        }

        boost::this_thread::sleep( boost::posix_time::millisec(200));
    }
}

void network::recieve(){
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
                    string ip = clientSock->remote_endpoint().address().to_string();
                    string payload = ip + " " + *msg;
                    mtx.lock();
                    InnboundMessages.push_back(payload);
                    mtx.unlock();
                    //cout << payload << endl;
                }
            }
            mtx.unlock();
        }

        boost::this_thread::sleep( boost::posix_time::millisec(200));
    }
}

void network::udpBroadcaster(){
/*
    //Loopbakc for test
    tcp::endpoint ep(ip::address::from_string("127.0.0.1"), port);
    try
    {
        socket_ptr sock(new tcp::socket(service));
        sock->connect(ep);
        // loop msg!
        clientList->emplace_back(sock);
        //cout << "Connected!" << endl;
    }
    catch(std::exception& e)
    {
        cerr << e.what() << endl;
        cout << "Not Connect!" << endl;

    }
*/

    //UDP broadcast, "Connect to me!"
    io_service udpService;
    udp::socket socket(udpService);
    socket_base::broadcast option(true);
    udp::endpoint remote_endpoint(address_v4::any(), 13);
    
    boost::system::error_code ignored_error;
    int msgsize = 9;
    socket.send_to(buffer("127.0.0.1", msgsize), remote_endpoint, 0, ignored_error);
    for (;;){
        //Listen for incomming
        if(socket.available())
        {
            char data[9] ={0};
            size_t length = socket.receive_from(buffer(data, msgsize), remote_endpoint,0, ignored_error);
            string_ptr msg(new string(data, length));
            try
            {
                tcp::endpoint ep(ip::address::from_string(*msg), port);
                cout << *msg << endl;
                socket_ptr sock(new tcp::socket(service));
                sock->connect(ep);
                // loop msg!
                clientList->emplace_back(sock);
                cout << "Connected!" << endl;
            }
            catch(std::exception& e)
            {
                cerr << e.what() << endl;
                cout << "Not Connect!" << endl;

            }
        }
    }
}
