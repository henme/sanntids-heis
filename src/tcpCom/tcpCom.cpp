#include <iostream>
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
typedef boost::shared_ptr< queue<string_ptr> > messageQueue_ptr;

//Hardcode until broadcast works, chat server must be running
tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001);
io_service service;
//Make public member, or make functionality to poll?
messageQueue_ptr messageQueue(new queue<string_ptr>);
const int inputSize = 256;
socket_ptr sock;
boost::mutex mtx;

tcpCom::tcpCom(){
	tcpCom::init();
	tcpCom::broadcast();
}

void tcpCom::init(){
	//Set up connection
	try
    {
        boost::thread_group threads;
        socket_ptr sock(new tcp::socket(service));

        sock->connect(ep);

        cout << "Connected" << endl;

      	//Start approriate threads
        //threads.create_thread(boost::bind(tcpCom::recieve, sock));
    }
    catch(std::exception& e)
    {
        cerr << e.what() << endl;
    }
}

void tcpCom::send(string msg){
    if(!msg.empty())
    {
    	//error
        //sock->write_some(buffer(msg, inputSize));
    }
}

void tcpCom::recieve(){
    int bytesRead = 0;
    char readBuf[1024] = {0};

    for(;;)
    {
        if(sock->available())
        {
            bytesRead = sock->read_some(buffer(readBuf, inputSize));
            string_ptr msg(new string(readBuf, bytesRead));
            mtx.lock();
            messageQueue->push(msg);
            mtx.unlock();
        }

        boost::this_thread::sleep( boost::posix_time::millisec(1000));
    }
}

void tcpCom::broadcast(){
	//Some way to broadcast and connect to peers
	//Will need a meber list/map
}
