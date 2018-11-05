#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>

int isIP(int output[5], char input[]){
	int l = strlen(input);

	int tmp[5] = {};
	int counter = 0, flag = 0, portFlag = 0;

	for (int i=0;i<l;i++){
		if (input[i] == ':'){
			if (counter != 3){
				return -1;
			}
		}
		else if(input[i] == '.'){
			if (flag == 0){
				return -1;
			}
			else{
				counter += 1;
				flag = 0;
			}
		}
		else if(input[i] <= '9' && input[i] >= '0'){
			flag = 1;
		}
		else{
			return -1;
		}
	}

	return 0;
}

int parseArg(int *t,int *n,char addrList[][1024],int argc, char *argv[],int portList[],int *serverNum){
	int addrCount = 0;
	for(int i=1;i<argc;i++){
		if (argv[i][0] == '-'){

			if(argv[i][1] == 'n'){
				i += 1;
				*n = atoi(argv[i]);
			}
			else if(argv[i][1] == 't'){
				i += 1;
				*t = atoi(argv[i]);
			}
			else{
				printf("wrong command\n");
				return -1;
			}
		}
		else{
			int l = strlen(argv[i]);
			int flag = 0;
			for (int j=0;j<l;j++){
				if (j > 1023){
					printf("IP too long\n");
					return -1;
				}
				
				else if (argv[i][j] == ':'){
					strncpy(addrList[addrCount],argv[i],j);
					addrList[addrCount][j] = '\0';
					char tmpPort[10];
					strncpy(tmpPort,argv[i]+j+1,l - j - 1);
					tmpPort[l - j - 1] = '\0';
					portList[addrCount] = atoi(tmpPort);
					flag = 1;
					addrCount += 1;

				}
			}
			if (!flag){
				printf("doesn't set IP\n");
				return -1;
			}
		}	
	}
	*serverNum = addrCount;
	return 0;

}

int main(int argc, char **argv){
	struct sockaddr_in dest[10];
	int n = 0,t = 1000,err,serverNum;
	char addrList[10][1024];
	clock_t start[10];
	int portList[10];	
	int sockfd[10];
	struct timeval tv;

	err = parseArg(&t,&n,addrList,argc,argv,portList,&serverNum);

	for(int i = 0;i < serverNum;i++){	
		struct addrinfo hints, *res, *p;
		int status;
		char ipstr[INET_ADDRSTRLEN];
		memset(&hints,0,sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

//	printf("%s\n",addrList[0]);
		status = getaddrinfo(addrList[i],NULL,&hints,&res);

//	printf("%d\n",status);	
		for (p = res;p!=NULL;p = p->ai_next){
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			inet_ntop(p->ai_family,&(ipv4->sin_addr),addrList[i],sizeof(addrList[i]));
//			printf("%s\n",addrList[i]);
		}	

	}
	if(err == -1){
		printf("parse fail\n");
		return 0;
	
	}


	// cread socket, AF_INET for IPv4, SOCK_STREAM for TCP	
	for(int i=0;i<serverNum;i++)
		sockfd[i] = socket(AF_INET,SOCK_STREAM,0);


	for(int i= 0;i<serverNum;i++){
		//init value in dest
		memset(&(dest[i]),0,sizeof(dest[i]));
		dest[i].sin_family = PF_INET;
		dest[i].sin_port = htons(portList[i]);

		dest[i].sin_addr.s_addr = inet_addr(addrList[i]);

		// Connect to server
		err = connect(sockfd[i],(struct sockaddr*)(dest+i),sizeof(dest[i]));



		if(err == -1){
			printf("timeout when connect to %s\n",addrList[i]);
		}
		

	}

	//Here can be parallel
	srand(time(NULL));
	
	for(int packN = -10;packN < n-1;packN += 1){
		if(n == 0){
			packN = -100;
		}
		for(int i=0;i<serverNum;i++){

			tv.tv_usec = (t%1000)*1000;

			tv.tv_sec = t/1000;
//			printf("%d %d\n",tv.tv_usec,tv.tv_sec);
			setsockopt(sockfd[i],SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(tv));

//			printf("%s:%d , socketfd = %d\n",addrList[i],portList[i],sockfd[i]);
			char msg[100] = "hello!";
			char outputMsg[100];
			memset(outputMsg,0,100);
			int msgI = rand()%100000000;

			sprintf(msg,"%x",msgI);


			start[i] = clock();
			struct timeval tm,after;
			gettimeofday(&tm,NULL);
			int ii = send(sockfd[i],msg,strlen(msg),0);
	//		printf("%d %d\n",ii,i);

			ii = recv(sockfd[i],outputMsg,99,0);


			clock_t end = clock();

			if(ii == -1 && errno == EAGAIN){
				printf("timeout when connect to %s\n",addrList[i]);
			}
			else if (strcmp(outputMsg,msg) == 0){
				gettimeofday(&after,NULL);
			//	printf("tm : %d %d, after : %d %d\n",tm.tv_usec,tm.tv_sec,after.tv_usec,after.tv_sec);
				float msec = 1000*(after.tv_sec - tm.tv_sec) + (after.tv_usec - tm.tv_usec)/1000.0;
				printf("recv from %s, RTT = %f msec\n",addrList[i],msec);
			}
			else{
				
				//printf("msg : %s\n",msg);
				//printf("get %s\n",outputMsg);
				//printf("wrong msg\n");

			}
		}

	}	
}
