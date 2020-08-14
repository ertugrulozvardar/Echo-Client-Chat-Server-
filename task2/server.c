#include<stdio.h>
#include<string.h>	
#include<stdlib.h>	
#include<sys/socket.h>
#include<arpa/inet.h>	
#include<unistd.h>	
#include<pthread.h> 
#include <stdbool.h> 

//the thread function
void *client_handler(void *);

int* live_socks[3]; 
int num_of_live_socks = 0;
pthread_mutex_t lock;

int main(int argc, char *argv[])
{
	if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex initialization failed\n");
        return 1;
    }
	
	int socket_desc, new_socket, c, *new_sock;
	struct sockaddr_in server; // Server-socket properties

	//Create server socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0); 

	if (socket_desc == -1)
	{
		printf("Could not create server socket");
	}

	puts("Socket created");
	
	//Prepare the sockaddr_in structure, assign socket to port 8888
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);
	
	// Binds the socket to the address and port number specified in addr(custom data structure)
	if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");
	
	//Listening the socket
	listen(socket_desc , 3); 
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	int new_socket;

	while((new_socket = accept(socket_desc, (struct sockaddr *)&server, (socklen_t*)&c)))
	{		
		pthread_t sniffer_thread;

		int *new_sock = (int *)malloc(sizeof(new_socket));
		*new_sock = new_socket;

		if (num_of_live_socks == 3) {
			puts("Connection refused because there are 3 clients currently active.");
			continue;
		} else {
			puts("Connection accepted");
		}

		if(pthread_create(&sniffer_thread, NULL, client_handler, (void*) new_sock) < 0)
		{
			perror("could not create thread");
			return 1;
		}

    	pthread_mutex_lock(&lock);
		
		num_of_live_socks = num_of_live_socks + 1;
		
		pthread_mutex_unlock(&lock);

		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( sniffer_thread , NULL);
		puts("Handler assigned");
	}
	
	if (new_socket < 0)
	{
		perror("accept failed");
		return 1;
	}
	
	return 0;
}

/*
 * This will handle connection for each client
 * */
void *client_handler(void *socket_desc)
{
	int sock = *(int*)socket_desc;

	if (num_of_live_socks == 3) {
		puts("Next one will be wait since 2 clients are currently in operation");

		char msg[1024] = "You will be wait since 2 clients are currently in operation");
		send(sock , msg , strlen(msg), 0);
	}

	while (num_of_live_socks == 3);

	// initialize the timer
	time_t init = time(0);

	//Get the socket descriptor
	int read_size;
	char cl_message[2000] = {0};

	//Receive a message from client.
	while(time(0) <= init + 10)
	{
		while ((read_size = recv(sock , cl_message , 2000 , MSG_DONTWAIT)) > 0) {
		//Send the message back to client
			write(sock , cl_message , strlen(cl_message));
		}
	}
	
	if(read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		if (time(0) <= start + 10) {
			perror("recv failed");
		} else {
			char msg[1000] = "Client connection will be terminated due to time-out)";
			send(sock , msg , strlen(msg), 0);
			puts("Client connection will be terminated due to time-out");
		}
	} 

	// Free the socket pointer
	close(sock);
	free(socket_desc);

    pthread_mutex_lock(&lock);

	num_of_live_socks = num_of_live_socks - 1;

	pthread_mutex_unlock(&lock);

	return 0;
}