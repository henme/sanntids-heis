#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <cstdlib>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

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
const int heartbeat_time_ms = 1000;

io_service service;
boost::mutex mtx;
clientList_ptr clientList(new list<socket_ptr>);
messageQueue_ptr OutMessageQueue(new queue<clientMap_ptr>);

network::network(int port, string ip) : port(port), ip(ip)
{
    new boost::thread(bind(&network::connectionHandler, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));
    new boost::thread(bind(&network::recieve, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));
    new boost::thread(bind(&network::respond, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));        
    new boost::thread(bind(&network::udpBroadcaster, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));
    new boost::thread(bind(&network::heartbeat, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));      
}

void network::connectionHandler(){
    cout << "Waiting for peers..." << endl;
    tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), port));
    while(true)
    {
        socket_ptr clientSock(new tcp::socket(service));
        acceptor.accept(*clientSock);
        mtx.lock();
        clientList->emplace_back(clientSock);
        mtx.unlock();

        std::string s = clientSock->remote_endpoint().address().to_string();
        cout << s << " connected sucsessfully!" << endl;
    }
}

void network::heartbeat(){
    while(true){
        send("syn");
        for(auto& clientSock : *clientList)
        {
            //if timestape outdated, close socket
        }
        boost::this_thread::sleep(boost::posix_time::millisec(heartbeat_time_ms));   
    }
}

void network::send(string msg){
    string_ptr msgptr(new string(msg));
    clientMap_ptr cm(new clientMap);
    cm->insert(pair<socket_ptr, string_ptr>(NULL, msgptr));
    mtx.lock();
    OutMessageQueue->push(cm);
    mtx.unlock();
}

void network::respond(){
    while(true)
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
                catch(exception& e)
                {
                    clientSock->close();
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
    while(true)
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
                    string client_ip = clientSock->remote_endpoint().address().to_string();
                    if(*msg != "ack" || *msg != "syn")
                    {
                        string payload = client_ip + " " + *msg;
                        InnboundMessages.push_back(payload);
                    }
                    // Plans to implement Heartbeat system
                    if(*msg == "syn")
                    {
                        try{
                            char data[3];
                            strcpy(data, "ack");
                            clientSock->write_some(buffer(data));
                        }
                        catch(exception& e)
                        {
                            clientSock->close();
                            clientList->remove(clientSock);
                            std::string s = clientSock->remote_endpoint().address().to_string();
                            cout << s << " disconnected!" << endl;
                        }
                    }

                }
            }
            mtx.unlock();
        }
        boost::this_thread::sleep(boost::posix_time::millisec(200));
    }
}

std::vector<std::string> network::get_messages(){
      vector<std::string> messages = InnboundMessages;
      InnboundMessages = {};
      return messages;
}

std::vector<std::string> network::get_listofPeers(){
    vector<std::string> listofPeers;
    for(auto& clientSock : *clientList)
    {
        listofPeers.push_back(clientSock->remote_endpoint().address().to_string());
    }
    return listofPeers;
}

void network::udpBroadcaster(){
    //UDP broadcast, "Connect to me!"
    io_service io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), 0));
    socket.set_option(socket_base::broadcast(true));
    ip::udp::endpoint broadcast_endpoint(address_v4::broadcast(), 8887);
    char data[bufSize];
    strcpy(data, ip.c_str());
    socket.send_to(buffer(data), broadcast_endpoint);
//NOTE: to loop back to localHost, different UDP ports are used in testing! Cant use same socket
    //Listen for incomming broadcast
    udp::socket recieveSocket(io_service, udp::endpoint(udp::v4(), 8888 ));
    udp::endpoint sender_endpoint;
    while(true)
    {
        char data[bufSize] ={0};
        std::size_t bytes_transferred = recieveSocket.receive_from(buffer(data), sender_endpoint);
        string_ptr msg(new string(data, bytes_transferred));
        //Connect to given ip
        if(!msg->empty())
        {
            try
            {
                tcp::endpoint ep(address::from_string(*msg), port);
                cout << *msg << endl;
                socket_ptr sock(new tcp::socket(service));
                sock->connect(ep);
                clientList->emplace_back(sock);
                cout << "Broadcast recieved from: " << *msg <<" -> Connected!" << endl;
            }
            catch(exception& e)
            {
                cerr << e.what() << endl;
            }
        }
    }
}
