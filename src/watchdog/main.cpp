#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

using namespace std;

int main( int argc, char **argv ) 
{
  int i = 0;
  char buffer[BUF_LEN];

  if(true){
      string STRING;
      ifstream infile;
      infile.open ("backup.txt");
            while(!infile.eof) // To get you all the lines.
            {
              getline(infile,STRING); // Saves the line in STRING.
              cout<<STRING; // Prints our STRING.
            }
      infile.close();
  }

  int fd = inotify_init();

  if ( fd < 0 ) {
    perror( "inotify_init" );
  }

  int wd = inotify_add_watch( fd, "backup.txt", IN_MODIFY);
  int length = read( fd, buffer, BUF_LEN );
  
  cout << "read" << endl;

  if ( length < 0 ) {
    perror( "read" );
  }

while(true){
    while ( i < length ) {
      struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
      if ( event->len ) {
          if ( event->mask & IN_MODIFY ) {
            cout << "The file " << event->name << "was modified." << endl;
          }
      i += EVENT_SIZE + event->len;
      }
    }
    length = read( fd, buffer, BUF_LEN );
  }
  inotify_rm_watch( fd, wd );

  /*closing the INOTIFY instance*/
  close( fd );

  exit( 0 );
}