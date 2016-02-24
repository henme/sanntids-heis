#include <iostream>
#include <string>
#include <queue>
#include <boost/asio.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/thread.hpp>
#include <boost/date_time.hpp> 

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

typedef boost::shared_ptr<tcp::socket> socket_ptr;
typedef boost::shared_ptr<string> string_ptr;
typedef boost::shared_ptr< queue<string_ptr> > messageQueue_ptr;

void printnumbers();

int main() {
    //Watchdog timer approach, thread has 5 s to finish, else terminate
    //The OS closes the socket and the backup kicks inn.
    boost::posix_time::time_duration timeout = boost::posix_time::milliseconds(5000);
    boost::thread thrd(&printnumbers);
    if (thrd.timed_join(timeout))
    {
      thrd.interrupt();
    }
    return 0;
}

void printnumbers() {
    io_service service;
    messageQueue_ptr messageQueue(new queue<string_ptr>);
    tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001);
    const int inputSize = 256;

    socket_ptr sock(new tcp::socket(service));
    sock->connect(ep);

    string msg;
    for(int i = 1; i < 5; i++){
	    cout << i << endl;
        msg = to_string(i);
        sock->write_some(buffer(msg, inputSize));
        boost::this_thread::sleep( boost::posix_time::millisec(1000));
    }
}

