#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>

#define BUFSIZE 1024

using namespace std;

char readbuf[1024];
char sendbuf[1024];
char quitbuf[30];

int main(void) 
{
	int ret = -1; //return value temp
	int ser_sd = -1;
	int cNum = 0; // accessed client num
	int socks[10] = {0}; //client sd array
	struct sockaddr_in sAddr;
	fd_set read_fds, write_fds, error_fds;
	int maxfds = 0;
	int i,j = 0;
	int tmpsock = 0;
	strcpy(quitbuf, "One client quit the chatting.");
	memset(&sAddr, 0 ,sizeof(sAddr));

	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(6743);
	sAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* socket create */
	ser_sd = socket(PF_INET, SOCK_STREAM, 0);
	if( ser_sd == -1 ) { perror("sock create error"); exit(1); }
	maxfds = ser_sd + 1;  // sd range, would be watched

	/* bind */
	ret = bind(ser_sd, (struct sockaddr*)&sAddr, sizeof(sAddr));
	if( ret == -1 ) { perror("bind error"); exit(1); }	

	/* listen */
	ret = listen(ser_sd, 5);
	if ( ret == -1) { perror("listen error"); exit(1); }

	cout << "=> Looking for clients." << endl;
	cout << "=> Server is running on port " << ntohs(sAddr.sin_port) << endl;

	while(1) {
		cout << "=> accessed clients: " << cNum << endl;

		/* chat setting */
		memset(readbuf, 0, sizeof(readbuf));
		FD_ZERO(&read_fds);
		FD_SET(ser_sd, &read_fds); //server socket bit setting

		/* set connected client sd bits */
		for( i=0 ; i<cNum ; i++) {
			//cout << "set client bits. client bit is ***** " << socks[i] << endl;
			FD_SET(socks[i], &read_fds);
		}

		/* client accept setting */
		int conn_sd = -1;
		int fd_num = 0;
		struct sockaddr_in clientAddr;
		socklen_t size;
		size = sizeof(clientAddr);

		fd_num = select( maxfds+1 , &read_fds, 0, 0, NULL);

		if(fd_num == -1) break;

		/* server or client sd event  */
		for( i=0 ; i<maxfds ; i++ ){
			if( FD_ISSET(i, &read_fds) == 1 ) { // event at i fd
				/* server accept new client */
				if ( i == ser_sd  ) { //server socket 
					conn_sd = accept(ser_sd, (struct sockaddr*)&clientAddr, &size);

					if( cNum < 10 && conn_sd > 0 ) {
						cout << "=> New client " << cNum+1 << " accessed. " << endl;
						socks[cNum++] = conn_sd;
						maxfds = conn_sd+1; //max fd increase;
					}
					else
						perror("Accept Failed.");
				}
				
				/* else bit event (message or exit)*/
				else { //client sockect
					if( recv(i, readbuf, sizeof(readbuf), 0) == 0) { // socket close event
						cout << quitbuf << endl;
						FD_CLR(i, &read_fds); //cancel watching
						close(i);

						for ( j=0 ; j<cNum ; j++){
							if( socks[j] == i ) {
								socks[j] = socks[cNum-1];
								socks[cNum-1] = 0; 
							}
						}

						cNum--;

						cout << "=> Close client socket " << i << endl;

						for( j=0 ; j<cNum ; j++)
							send( socks[j], quitbuf, sizeof(quitbuf), 0);
					}
					
					else {	
						cout << readbuf << endl;

						for( j=0 ; j<cNum ; j++){
							if( socks[j] !=  i )
								send( socks[j], readbuf, sizeof(readbuf), 0);
						}

					}
				} //client socket else

			} // if 

		} // for
	}

	close(ser_sd);

	return 0;
}	

