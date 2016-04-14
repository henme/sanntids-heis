#include <iostream>
#include <list>
#include <queue>
#include <string>
#include <cstdlib>

#include <boost/thread.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "network.hpp"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
using boost::asio::ip::address_v4;

typedef boost::shared_ptr<tcp::socket> socket_ptr;
typedef boost::shared_ptr<string> string_ptr;
typedef boost::shared_ptr< list< tuple< socket_ptr, time_t, address_v4 > > > clientList_ptr;
typedef boost::shared_ptr< queue<string> > messageQueue_ptr;

const int bufSize = 128; 
const double heartbeat_time = 3;

io_service service;
boost::mutex OutMessageQueue_mtx;
boost::mutex clientList_mtx;
boost::mutex innboundMessages_mtx;
boost::mutex connectedPeers_mtx;
clientList_ptr clientList(new list< tuple< socket_ptr, time_t, address_v4 > >);
messageQueue_ptr OutMessageQueue(new queue<string>);

network::network(int port, address_v4 myIP) : port(port), myIP(myIP)
{
    new boost::thread(bind(&network::connectionHandler, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));
    new boost::thread(bind(&network::recieve, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));
    new boost::thread(bind(&network::tcpMessageBroadcaster, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));        
    new boost::thread(bind(&network::heartbeat, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));
    new boost::thread(bind(&network::udpBroadcaster, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));
    new boost::thread(bind(&network::udpListener, this));
    boost::this_thread::sleep( boost::posix_time::millisec(100));      
}

void network::connectionHandler(){
    tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), port));
    while(true)
    {
        socket_ptr clientSock(new tcp::socket(service));
        acceptor.accept(*clientSock);
        clientList_mtx.lock();
        // Check if allready connected
        for(auto& sock : *clientList)
        {
            try{
                if(get<2>(sock) == clientSock->remote_endpoint().address().to_v4()){
                    return; 
                } 
            } catch(...){
                cerr << "Could not reach remote endpoint in connectionHandler" << endl;
            }
        }
        connectedPeers_mtx.lock();
        try{
            address_v4 ip = clientSock->remote_endpoint().address().to_v4();
            clientList->emplace_back(make_tuple(clientSock, time(NULL), ip));
            connectedPeers.emplace_back(make_pair(ip,true));
            cout << ip << " connected sucsessfully!" << endl;
        } catch(...){cerr << "Could not connect to socket in connectionHandler, remote endpoint" << endl;}
        connectedPeers_mtx.unlock();
        clientList_mtx.unlock();
    }
}

void network::heartbeat(){
    while(true){
        if(!clientList->empty())
        {
            clientList_mtx.lock();
            for(auto& clientSock : *clientList)
            {
                double seconds = difftime(time(NULL), get<1>(clientSock));
                if(seconds >= heartbeat_time)
                { 
                    cout << "Client dissconected" << endl;
                    get<0>(clientSock)->close();
                    address_v4 ip = get<2>(clientSock);
                    connectedPeers_mtx.lock();
                    connectedPeers.emplace_back(make_pair(ip,false));
                    connectedPeers_mtx.unlock();
                    clientList->remove(clientSock);
                    break;
                }
                else
                {
                    sendtoSocket(get<0>(clientSock), "syn");
                }
            }
            clientList_mtx.unlock();
        }
        boost::this_thread::sleep(boost::posix_time::millisec(heartbeat_time*1000/2));
    }
}

void network::sendtoSocket(socket_ptr clientSock, string msg){
    char * data = new char[msg.size() + 1];
    copy(msg.begin(), msg.end(), data);
    data[msg.size()] = '\0';
    try{
        clientSock->write_some(buffer(data, strlen(data)));
    }
    catch(...){
        cerr << "Could not connect to socket in sendtoSocket" << endl;
    }
    delete[] data;
}

void network::tcpMessageBroadcaster(){
    while(true)
    {
        if(!OutMessageQueue->empty())
        {
            clientList_mtx.lock();
            for(auto& clientSock : *clientList)
            {
                try
                {
                    sendtoSocket(get<0>(clientSock), OutMessageQueue->front());
                }
                catch(...){
                    cerr << "Could not send to socket in tcpMessageBroadcaster" << endl;
                }
            }
            clientList_mtx.unlock();
            OutMessageQueue_mtx.lock();
            OutMessageQueue->pop();
            OutMessageQueue_mtx.unlock();
        }
        boost::this_thread::sleep(boost::posix_time::millisec(100));
    }
}

void network::recieve(){
    while(true)
    {
        if(!clientList->empty())
        {
            clientList_mtx.lock();
            for(auto& clientSock : *clientList)
            {
                if(get<0>(clientSock)->available())
                {
                    try{
                        char readBuf[bufSize] = {0};
                        int bytesRead = get<0>(clientSock)->read_some(buffer(readBuf, bufSize));
                        string_ptr msg(new string(readBuf, bytesRead));
                        messageParser(clientSock, msg);
                    }
                    catch(...){
                        cerr << "Could not read from socket in recieve" << endl;
                    }
                }
            }
            clientList_mtx.unlock();
        }
        boost::this_thread::sleep(boost::posix_time::millisec(100));
    }
}

void network::messageParser(tuple<socket_ptr, time_t, address_v4> &clientSock, string_ptr msg){
    boost::algorithm::trim(*msg);
    if(msg->find("syn") != string::npos){
        do {
            msg->erase(msg->find("syn"),3);
        } while(msg->find("syn") != string::npos);
        sendtoSocket(get<0>(clientSock), "ack");
    }
    if(msg->find("ack") != string::npos){
        get<1>(clientSock) = time(NULL);
        do {
            msg->erase(msg->find("ack"),3);
        } while(msg->find("ack") != string::npos);
    }
    if(!msg->empty()){
        innboundMessages_mtx.lock();
        try{
            address_v4 myIP = get<0>(clientSock)->remote_endpoint().address().to_v4();
            InnboundMessages.push_back(make_pair(myIP, *msg));
        } catch(...){
            cerr << "Could not reach remote endpoint in messageParser" << endl;
        }
        innboundMessages_mtx.unlock();
    }
}

void network::send(string msg){
    OutMessageQueue_mtx.lock();
    OutMessageQueue->push(msg);
    OutMessageQueue_mtx.unlock();
}

vector<pair<address_v4, string>> network::get_messages(){
    innboundMessages_mtx.lock();
    vector<pair<address_v4, string>> messages = InnboundMessages;
    InnboundMessages = {};
    innboundMessages_mtx.unlock();
    return messages;
}

vector<pair<address_v4, bool>>  network::get_listofPeers(){
    connectedPeers_mtx.lock();
    vector<pair<address_v4, bool>>  peers = connectedPeers;
    connectedPeers_mtx.unlock();
    connectedPeers = {};
    return peers;    
}


void network::udpBroadcaster(){
    io_service io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), 0));
    socket.set_option(socket_base::broadcast(true));
    udp::endpoint broadcast_endpoint(address_v4::broadcast(), 8888);
    string ip_string = myIP.to_string();
    char * data = new char[ip_string.size() + 1];
    copy(ip_string.begin(), ip_string.end(), data);
    data[ip_string.size()] = '\0';
    bool socketClosed = false;
    while(true){
        if(socketClosed){
            try{
                udp::endpoint broadcast_endpoint(address_v4::broadcast(), 8888);
                socketClosed = false;
            } catch(...){ 
                cerr << "Could not open socket in udpBroadcaster" << endl;
            }
        }
        try{
            socket.send_to(buffer(data, strlen(data)), broadcast_endpoint);
        } catch(...){
            socket.close();
            socketClosed = true;
            cerr << "Could not connect to socket in udpBroadcaster" << endl;
        }
        boost::this_thread::sleep(boost::posix_time::millisec(10000));
    }
}

void network::udpListener(){
    io_service io_service;
    udp::socket recieveSocket(io_service, udp::endpoint(udp::v4(), 8888));
    udp::endpoint sender_endpoint;
    bool socketClosed = false;
    while(true)
    {
        if (socketClosed == true){
            try{
                udp::socket recieveSocket(io_service, udp::endpoint(udp::v4(), 8888));
                udp::endpoint sender_endpoint;
                socketClosed = false;
            }catch(...){
                cerr << "Could not open socket in udpBroadcaster" << endl;
            }
        }
        char data[bufSize] ={0};
        size_t bytes_transferred = recieveSocket.receive_from(buffer(data), sender_endpoint);
        string_ptr msg(new string(data, bytes_transferred));
        if(!msg->empty())
        {
            bool allreadyConnected = false;
            clientList_mtx.lock();
            for(auto& sock : *clientList)
            {
                if(get<2>(sock) == address_v4::from_string(*msg)) {
                    cout << "client list ip: " << get<2>(sock) << endl;
                    allreadyConnected = true; 
                }
            }
            cout << *msg << endl;
            if(!allreadyConnected && myIP != address_v4::from_string(*msg)){
                connectedPeers_mtx.lock();
                try
                {
                    tcp::endpoint ep(address::from_string(*msg), port);
                    socket_ptr sock(new tcp::socket(service));
                    sock->connect(ep);
                    clientList->emplace_back(make_tuple(sock, time(NULL), address_v4::from_string(*msg)));
                    connectedPeers.emplace_back(make_pair(address_v4::from_string(*msg),true));
                    cout << "Broadcast recieved from: " << *msg << " -> Connected!" << endl;
                }
                catch(...){
                    recieveSocket.close();
                    socketClosed = true;
                    cerr << "Could not connect to socket in udpListener" << endl;
                }
                connectedPeers_mtx.unlock();
            }
            clientList_mtx.unlock();
        }
    }
    boost::this_thread::sleep(boost::posix_time::millisec(100));
}
