#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <signal.h>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
#define TIMEOUT     1.5

using namespace std;

bool changed = false;

void isModified(){
  char buffer[BUF_LEN];
  changed = false;
  int fd = inotify_init();

  if ( fd < 0 ) {
    return;
  }

  int wd = inotify_add_watch( fd, "backup.txt", IN_MODIFY);
  //Blocking read
  int length = read( fd, buffer, BUF_LEN );  
  if ( length < 0 ) {
    return;
  }
  changed = true;
  inotify_rm_watch( fd, wd );

  //closing the INOTIFY instance
  close( fd );
  return;
}

int main(){
  while(true){
    do{
      boost::thread t(&isModified);
      t.timed_join(boost::posix_time::seconds(TIMEOUT));
    } while(changed == true);
    //cout << "timed out" << endl;
    system("kill Heis"); //problems with not superuser?
    system("./bin/Heis");
  }
  exit(0);
}