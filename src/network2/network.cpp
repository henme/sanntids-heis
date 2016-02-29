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

const int bufSize = 1024;

io_service service;
boost::mutex mtx;
clientList_ptr clientList(new list<socket_ptr>);
messageQueue_ptr OutMessageQueue(new queue<clientMap_ptr>);

network::network(int port) : port(port)
{
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
    tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), port));
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
                try
                {
                    clientSock->write_some(buffer(*(message->begin()->second), bufSize));
                }
                catch(std::exception& e)
                {
                    clientList->remove(clientSock);
                    std::string s = clientSock->remote_endpoint().address().to_string();
                    cout << s << " disconnected!" << endl;
                }
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
                    InnboundMessages.push_back(payload);
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
    tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8002);
    try
    {
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
*/
    //UDP broadcast onces; "Connect to me!"
    io_service io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), 0));
    socket.set_option(socket_base::broadcast(true));
    ip::udp::endpoint broadcast_endpoint(address_v4::broadcast(), 8888);
    char data[1024];
    string tmp = "127.0.0.1";
    strcpy(data, tmp.c_str());
    socket.send_to(buffer(data), broadcast_endpoint);
    //Recieve loop

    udp::socket recieveSocket(io_service, udp::endpoint(udp::v4(), 8888 ));
    udp::endpoint sender_endpoint;
    for (;;){
        //Listen for incomming
        char data[9] ={0};
        std::size_t bytes_transferred = recieveSocket.receive_from(buffer(data), sender_endpoint);
        string_ptr msg(new string(data, bytes_transferred));
        cout << *msg << endl;
        if(!msg->empty()){
            try
            {
                tcp::endpoint ep(address::from_string(*msg), 8001);
                cout << *msg << endl;
                socket_ptr sock(new tcp::socket(service));
                sock->connect(ep);
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
