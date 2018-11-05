#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



int main(int argc, char **argv){
	struct sockaddr_in dest;
	char buffer[1024];
	int port = atoi(argv[1]);	
	int sockfd,err = 0;

	struct sockaddr_in remoteaddr[100];
	int addrLen = sizeof(remoteaddr);
	// cread socket, AF_INET for IPv4, SOCK_STREAM for TCP	
	sockfd = socket(AF_INET,SOCK_STREAM,0);


	//init value in dest
	memset(&dest,0,sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(port);
	dest.sin_addr.s_addr = INADDR_ANY;

	// Connect to server
	err = bind(sockfd,(struct sockaddr*)&dest, sizeof(dest));
	if(err == -1){
		printf("bind fail\n");
		return 0;
	}

	// listen max
	err = listen(sockfd,100);
	if(err == -1){
		printf("listen fail\n");
		return 0;
	}
	fd_set readfds,writefds,master;
	
	FD_ZERO(&master);
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	
	FD_SET(sockfd,&master);
	int maxer = sockfd;
	int newfd = -1;
	srand(time(NULL));
	while(1){
		readfds = master;
		if(select(maxer + 1,&readfds,&writefds,NULL,NULL) == -1){
			printf("select err\n");
			return 0;
		}
		for (int i= 0;i<=maxer;i++){
			if(FD_ISSET(i,&readfds)){
				if(i == sockfd){
					//new connect
					struct sockaddr_in tmpaddr;
					newfd = accept(sockfd,(struct sockaddr*)&tmpaddr,&addrLen);
					memcpy(&(remoteaddr[newfd]),&tmpaddr,addrLen);
					if (newfd == -1){
						printf("accept err\n");
						return 0;
					}
					else{
						FD_SET(newfd,&master);
						if(newfd > maxer)
							maxer = newfd;

					}
				}
				else{
					int getLen = 0;
					getLen = recv(i,buffer,1024,0);
					if (getLen < 0){
						printf("recv err\n");
					}
					else if (getLen == 0){
						//printf("close connection!\n");
						close(i);
						FD_CLR(i,&master);
					}
					else{
						printf("recv from %s:%d\n",inet_ntoa(remoteaddr[i].sin_addr),remoteaddr[i].sin_port);
						// about 33% to sleep for 10 sec
						int rd = 1;
//						printf("%d\n",rd);
						if (rd == 0){
//							printf("wait\n");
							//sleep(10);
						}
						else{
//							sleep(1);
							send(i,buffer,getLen,0);
						}
					}



				}


			
			}
		}
	}
}
