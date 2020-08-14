#include<stdio.h>	
#include<string.h>	
#include<sys/socket.h>	
#include<arpa/inet.h>	
#include<unistd.h>	

int main(int argc , char *argv[])
{
	int client_socket_desc;
	struct sockaddr_in server_sockaddr;
	char message[1024] = {0};
	char server_reply[1024] = {0};
	
	//Create socket
	client_socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket_desc == -1)
	{
		printf("Could not create socket");
	}

	puts("Socket created");
	
	server_sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(7244);

	//Connect to remote server
	if (connect(client_socket_desc , (struct sockaddr *)&server_sockaddr , sizeof(server_sockaddr)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}
	
	puts("Connected\n");
	
	//keep communicating with server
	while(1)
	{
		printf("Enter message : ");
		scanf("%s" , message);
		
		//Send some data
		if(send(client_socket_desc , message , strlen(message) , 0) < 0)
		{
			puts("Send failed");
			return 1;
		}
		
		//Receive a reply from the server
		if(recv(client_socket_desc , server_reply , 1024 , 0) < 0)
		{
			puts("recv failed");
			break;
		}
		
		puts("Server reply :");
		puts(server_reply);
		memset(server_reply, 0, sizeof server_reply);
	}
	
	close(client_socket_desc);
	return 0;
}