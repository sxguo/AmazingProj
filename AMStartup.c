/* ========================================================================== */
/* File: AMStartup.c
 *
 * Project name: CS50 Amazing Project
 * Component name: AMStartup
 *
 * Author: Ian DeLaney
 * Date: Wed, Mar 2016
 *
 * Init sends the AM_INIT message to the server and then beings threads for 
 * each avatar. Then waits until the process is done.
 *
 */
/* ========================================================================== */

// ---------------- Open Issues

// ---------------- System includes e.g., <stdio.h>
#include <stdlib.h>
#include <unistd.h>			//sleep functionality
#include <stdio.h>			//I/O functionality
#include <sys/types.h>			//Additional types
#include <sys/socket.h>			//socket functionality
#include <netinet/in.h>			//net functionality
#include <string.h>			//string functionality
#include <arpa/inet.h>			//net functionality
#include <pthread.h>			//Thread functionality

// ---------------- Local includes  e.g., "file.h"
#include "amazing.h"

// ---------------- Constant definitions

// ---------------- Macro definitions

// ---------------- Structures/Types

// ---------------- Private variables
char *hostIP;
int mazePort;
int mazeWidth;
int mazeHeight;

// ---------------- Private prototypes

// ---------------- Public functions


void* newAvatar(void *avatar_id)
{
	int AvatarId = *((int *) avatar_id);
	printf("Creating avatar %d\n", AvatarId);

	//create a new socket for this avatar
	int sockfd;
	struct sockaddr_in servaddr;

	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
        	perror("Problem in creating the socket");
        	exit(2);
    	}

    	//Setup socket
    	memset(&servaddr, 0, sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
     	servaddr.sin_addr.s_addr= inet_addr(hostIP);
     	servaddr.sin_port =  htons(mazePort); //convert to network order

     	//Connection of the client to the socket 
     	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
        	perror("Problem in connecting to the server");
        	exit(3);
     	}


	AM_Message *ready_msg = (AM_Message *) calloc(1, sizeof(AM_Message));
	ready_msg->type = htonl(AM_AVATAR_READY);
	ready_msg->avatar_ready.AvatarId = htonl(AvatarId);
	send(sockfd, ready_msg, sizeof(AM_Message), 0);

	AM_Message *response = (AM_Message *) calloc(1, sizeof(AM_Message));
	while(1 == 1) {
		recv(sockfd, response, sizeof(AM_Message), 0);
		printf("%u\n", ntohl(response->type));
		sleep(1);
	}
	return NULL;
}

void AMStartup(int nAvatars, int Difficulty)
{
	int sockfd;
	struct sockaddr_in servaddr;

	//create socket for startup
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
        	perror("Problem in creating the socket");
        	exit(2);
    	}

    	//Setup socket
    	memset(&servaddr, 0, sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
     	servaddr.sin_addr.s_addr= inet_addr(hostIP);
     	servaddr.sin_port =  htons(atoi(AM_SERVER_PORT)); //convert to network order

     	//Connection of the client to the socket 
     	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
        	perror("Problem in connecting to the server");
        	exit(3);
     	}

	AM_Message *AM_INIT_msg = (AM_Message *) calloc(1, sizeof(AM_Message));
	AM_INIT_msg->type = htonl(AM_INIT);
	AM_INIT_msg->init.nAvatars = htonl(nAvatars);
	AM_INIT_msg->init.Difficulty = htonl(Difficulty);

	send(sockfd, AM_INIT_msg, sizeof(AM_Message), 0);

	AM_Message *AM_INIT_resp = (AM_Message *) calloc(1, sizeof(AM_Message));
	recv(sockfd, AM_INIT_resp, sizeof(AM_Message), 0);
	if (ntohl(AM_INIT_resp->type) != AM_INIT_OK) {
		printf("Initialization failed with type %u\n", ntohl(AM_INIT_resp->type));
		return;
	}
	if (ntohl(AM_INIT_resp->type) == AM_INIT_OK) {
		printf("Message type is %u\n", ntohl(AM_INIT_resp->type));
		printf("MazePort is %u\n", ntohl(AM_INIT_resp->init_ok.MazePort));
		printf("Maze width is %u\n", ntohl(AM_INIT_resp->init_ok.MazeWidth));
		printf("Maze height is %u\n", ntohl(AM_INIT_resp->init_ok.MazeHeight));

	}
	mazeHeight = ntohl(AM_INIT_resp->init_ok.MazeHeight);
	mazeWidth = ntohl(AM_INIT_resp->init_ok.MazeWidth);
	mazePort = ntohl(AM_INIT_resp->init_ok.MazePort);

	int curr_id = 0;
	pthread_t avatars[nAvatars];
	while(curr_id < nAvatars) {
		int iret1 = pthread_create(&(avatars[curr_id]), NULL, newAvatar, &curr_id);
		if(iret1) {
			fprintf(stderr,"Cannot create thread, rc=%d\n", iret1);
			return;
		}
		sleep(1);
		curr_id++;
	}
	AM_Message *message = (AM_Message *) calloc(1, sizeof(AM_Message));
	while (1 == 1) {
		sleep(1);
	}

}


int main(int argc, char **argv)
{
	hostIP = argv[3];
	AMStartup(atoi(argv[1]), atoi(argv[2]));
	return 0;
}