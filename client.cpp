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
#include <string.h>

#define BUFSIZE 1024

using namespace std;
char name[10];
char buf[1024];
char recvbuf[1024];
char sendbuf[1024];
int main (int argc, char *argv[]){
	strcpy(name, argv[1]);
	int clientsd = -1;
	int portNum = 6743;
	fd_set readfds;
	struct sockaddr_in serAddr;
	int maxfds = 0; 
	int ret = 0;

	cout << "=> Your nickname is " << argv[1] << "." << endl;
	clientsd = socket(AF_INET, SOCK_STREAM, 0);
	
	if( clientsd == -1 ) { cout << "sock create error" << endl; exit(1); }

	memset(&serAddr, 0, sizeof(serAddr));

	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(portNum);
	serAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if( connect(clientsd, (struct sockaddr *)&serAddr, sizeof(serAddr)) == 0 ){
		cout << "=> Connection to the server port: " << portNum << endl;
		cout << "=> Chatting is started. " << endl;
		maxfds = clientsd + 1;
	}

	memset(recvbuf, 0, sizeof(recvbuf));	

	while(1) {
		FD_ZERO(&readfds);
		FD_SET(clientsd, &readfds);
		FD_SET(0, &readfds); //stdin watch (keyboard)
		select(maxfds+1, &readfds, 0, 0, 0);
		memset(buf, 0, sizeof(buf));
		if( FD_ISSET(clientsd, &readfds) != 0 ){
			ret = read(clientsd, recvbuf, BUFSIZE);

			if(ret == -1){ perror("recv error"); exit(1); }

			cout << recvbuf << endl;	
		}
		else if( FD_ISSET(0, &readfds) != 0 ){
			cin >> buf;

			if(!strncmp(buf, "quitchat", 8))
				break;

			sprintf(sendbuf, "%s %s", name, ": ");
			strncat(sendbuf, buf, 1000);
			ret = write(clientsd, sendbuf, strlen(sendbuf));
		}
	}

	close (clientsd);

	return 0;
}
