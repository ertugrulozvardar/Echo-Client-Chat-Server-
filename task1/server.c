#include<stdio.h>
#include<string.h>	
#include<stdlib.h>	
#include<sys/socket.h>
#include<arpa/inet.h>	
#include<unistd.h>	
#include<pthread.h> 

//the thread function
void *client_task(void *);

int main(int argc, char *argv[])
{
	struct sockaddr_in server_sockaddr; // Server socket properties
	int server_sockaddr_length = sizeof(server_sockaddr); 

	//Create server socket
	int server_socket_desc = socket(AF_INET, SOCK_STREAM, 0);

	if (server_socket_desc == -1)
	{
		printf("Could not create server socket");
	}

	puts("Socket created");
	
	//Prepare the sockaddr_in structure, assign socket to port 8888
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_addr.s_addr = INADDR_ANY;
	server_sockaddr.sin_port = htons(8888);
	
	// Binds the socket to the address and port number specified in addr(custom data structure)
	if(bind(server_socket_desc, (struct sockaddr *)&server_sockaddr, server_sockaddr_length) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");
	
	// Puts the server socket in a passive mode, where it waits for the client to approach the server to make a connection
	// The backlog (3), defines the maximum length to which the queue of pending connections for sockfd may grow. 
	// If a connection request arrives when the queue is full, 
	// the client may receive an error with an indication of ECONNREFUSED
	listen(server_socket_desc , 3); 
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");

	int client_sock_desc;
	// accept = extracts the first connection request on the queue of pending connections for the listening socket, 
	// socket file desc, creates a new connected socket, and returns a new file descriptor referring to that socket
	while((client_sock_desc = accept(server_socket_desc, (struct sockaddr *)&server_sockaddr, (socklen_t*)&server_sockaddr_length)))
	{
		puts("Connection accepted");
		
		pthread_t sniffer_thread;

		int *new_client_socket = (int *)malloc(sizeof(client_sock_desc));
		*new_client_socket = client_sock_desc;

		if(pthread_create(&sniffer_thread, NULL, client_task, (void*) new_client_socket) < 0)
		{
			perror("could not create thread");
			return 1;
		}
		
		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( sniffer_thread , NULL);
		puts("Handler assigned");
	}
	
	if (client_sock_desc < 0)
	{
		perror("accept failed");
		return 1;
	}
	
	return 0;
}

/*
 * This will handle connection for each client
 * */
void *client_task(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;
	char buffer[1024] = {0};
		
	//Receive a message from client. This thread is stuck in here.
	while((read_size = recv(sock , buffer , 1024 , 0)) > 0)
	{
		//Send the message back to client
		send(sock , buffer , strlen(buffer), 0);
		memset(buffer, 0, sizeof buffer);
	}

	if(read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}
		
	// Free the socket pointer
	close(sock);
	free(socket_desc);
	
	return 0;
}