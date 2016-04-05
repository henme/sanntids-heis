#include <iostream>
#include <list>
#include <queue>
#include <string>
#include <cstdlib>
#include <ctime>
#include <tuple>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "network.hpp"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

typedef boost::shared_ptr<tcp::socket> socket_ptr;
typedef boost::shared_ptr<string> string_ptr;
typedef boost::shared_ptr< list< tuple< socket_ptr, time_t, string > > > clientList_ptr;
typedef boost::shared_ptr< queue<string> > messageQueue_ptr;

const int bufSize = 128; 
const double heartbeat_time = 3; //Acceptable waiting time?

io_service service;
boost::mutex OutMessageQueue_mtx;
boost::mutex clientList_mtx;
clientList_ptr clientList(new list< tuple< socket_ptr, time_t, string > >);
messageQueue_ptr OutMessageQueue(new queue<string>);


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
    //cout << "Waiting for peers..." << endl;
    tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), port));
    while(true)
    {
        socket_ptr clientSock(new tcp::socket(service));
        acceptor.accept(*clientSock);
        clientList_mtx.lock();
        for(auto& sock : *clientList)
        {
            if(get<2>(sock) == clientSock->remote_endpoint().address().to_string())
                return; 

        }
        string s = clientSock->remote_endpoint().address().to_string();
        clientList->emplace_back(make_tuple(clientSock, time(NULL), s));
        connectedPeers[s] = true;
        clientList_mtx.unlock();
        cout << s << " connected sucsessfully!" << endl;
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
                    string s = get<2>(clientSock);
                    connectedPeers[s] = false;
                    clientList->remove(clientSock);
                    break;
                }
                else
                {
                    char data[3];
                    string syn = "syn";
                    strcpy(data,syn.c_str());
                    try{
                        get<0>(clientSock)->write_some(buffer(data));
                    }
                    catch(exception& e){}
                }
            }
            clientList_mtx.unlock();
        }
        boost::this_thread::sleep(boost::posix_time::millisec(heartbeat_time*1000/2));
    }
}

void network::respond(){
    while(true)
    {
        if(!OutMessageQueue->empty())
        {
            char data[bufSize] = {0};
            string message = OutMessageQueue->front();
            strcpy(data, message.c_str());
            clientList_mtx.lock();
            for(auto& clientSock : *clientList)
            {
                try
                {
                    get<0>(clientSock)->write_some(buffer(data));
                }
                catch(exception& e){}
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
                        if ((msg->find("syn") == string::npos) && (msg->find("ack") == string::npos))
                        {
                            if(msg->length() > 10){
                                InnboundMessages.push_back(*msg);
                            }
                        }
                        if(msg->find("syn") != string::npos)
                        {
                            //cout << "syn received " << endl;

                            while(msg->find("syn") != string::npos){
                                //cout << "syn removed" << endl;
                                msg->erase(msg->find("syn"),3);
                            }

                            char data[3];
                            string ack = "ack";
                            strcpy(data,ack.c_str());
                            try{
                                get<0>(clientSock)->write_some(buffer(data));
                            }
                            catch(exception& e){}
                            //Guard against concocted messages
                            if(msg->length() > 10){
                                //cout << "syn parse" << endl;
                                InnboundMessages.push_back(*msg);
                            }
                        }
                        if(msg->find("ack") != string::npos)
                        {
                            //cout << "ack received " << endl;
                            while(msg->find("ack") != string::npos){
                                //cout << "ack removed" << endl;
                                msg->erase(msg->find("ack"),3);
                            }
                            get<1>(clientSock) = time(NULL);
                            if(msg->length() > 10){
                                //cout << "ack parse" << endl;
                                InnboundMessages.push_back(*msg);
                            }
                        }
                    }
                    catch(exception& e){}
                }
            }
            clientList_mtx.unlock();
        }
        boost::this_thread::sleep(boost::posix_time::millisec(100));
    }
}

void network::send(string msg){
    OutMessageQueue_mtx.lock();
    OutMessageQueue->push(msg);
    OutMessageQueue_mtx.unlock();
}

vector<string> network::get_messages(){
      vector<string> messages = InnboundMessages;
      InnboundMessages = {};
      return messages;
}

map<string, bool> network::get_listofPeers(){
      map<string, bool> peers = connectedPeers;
      connectedPeers.clear();
      return peers;    
}


void network::udpBroadcaster(){
    //UDP broadcast, "Connect to me!" once on startup
    io_service io_service;
    udp::socket socket(io_service, udp::endpoint(udp::v4(), 0));
    socket.set_option(socket_base::broadcast(true));
    udp::endpoint broadcast_endpoint(address_v4::broadcast(), 8888);
    char data[bufSize];
    strcpy(data, ip.c_str());
    socket.send_to(buffer(data), broadcast_endpoint);
    //Listen for incomming broadcast
    udp::socket recieveSocket(io_service, udp::endpoint(udp::v4(), 8888 ));
    udp::endpoint sender_endpoint;
    while(true)
    {
        char data[bufSize] ={0};
        size_t bytes_transferred = recieveSocket.receive_from(buffer(data), sender_endpoint);
        string_ptr msg(new string(data, bytes_transferred));
        bool allreadyConnected = false;
        if(!msg->empty())
        {
            for(auto& sock : *clientList)
            {
                if(get<2>(sock) == *msg) 
                    allreadyConnected = true; 
            }
            if(!allreadyConnected){
                try
                {
                    tcp::endpoint ep(address::from_string(*msg), port);
                    socket_ptr sock(new tcp::socket(service));
                    sock->connect(ep);
                    clientList_mtx.lock();
                    clientList->emplace_back(make_tuple(sock, time(NULL), *msg));
                    connectedPeers[*msg] = true;
                    clientList_mtx.unlock();
                    cout << "Broadcast recieved from: " << *msg <<" -> Connected!" << endl;
                }
                catch(exception& e){}
            }
        }
    }
}
