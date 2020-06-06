////
//// Created by hubert on 04.06.2020.
////
//
//#include "server.h"
//
//int main()
//{
//	struct sockaddr_rc local_addr;
//	struct sockaddr_rc remote_addr;
//
//	int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
//
//	if (sock < 0)
//	{
//		perror("Could not create socket\n");
//		exit(-1);
//	}
//
//	local_addr.rc_family = AF_BLUETOOTH;
//	local_addr.rc_bdaddr = *BDADDR_ANY;
//	local_addr.rc_channel = (uint8_t) 1;
//
//	if (bind(sock, (struct sockaddr*)&local_addr, sizeof(struct sockaddr_rc)) == -1)
//	{
//		perror("Could not bind\n");
//		exit(-1);
//	}
//
//	listen(sock,1);
//	while(1){}
//	close(sock);
//}

#include "server.h"

int main(int argc, char **argv)
{
//	struct sockaddr_rc loc_addr = {0}, rem_addr = {0};
//	char buf[1024] = {0};
//	int s, client, bytes_read;
//	socklen_t opt = sizeof(rem_addr);
//
//	// allocate socket
//	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
//
//	// bind socket to port 1 of the first available
//	// local bluetooth adapter
//	loc_addr.rc_family = AF_BLUETOOTH;
//	loc_addr.rc_bdaddr = *BDADDR_ANY;
//	loc_addr.rc_channel = (uint8_t) 1;
//	bind(s, (struct sockaddr *) &loc_addr, sizeof(loc_addr));
//
//	//get local address ?
//	//~ ba2str( &loc_addr.rc_bdaddr, buf );
//	//~ fprintf(stdout, "local %s\n", buf);
//
//	// put socket into listening mode
//	listen(s, 1);
//
//	// accept one connection
//	client = accept(s, (struct sockaddr *) &rem_addr, &opt);
//
//	ba2str(&rem_addr.rc_bdaddr, buf);
//	fprintf(stderr, "accepted connection from %s\n", buf);
//
//
//	memset(buf, 0, sizeof(buf));
//
//	// read data from the client
//	bytes_read = read(client, buf, sizeof(buf));
//
//	if (bytes_read > 0)
//	{
//		printf("received [%s]\n", buf);
//	}
//
//	// close connection
//	close(client);
//	close(s);
//	return 0;

	init("0000");

	pthread_t server_lifetime_management;
	pthread_t connection_handler_thread;
	pthread_create(&server_lifetime_management, NULL, (void *(*)(void *)) server_lifetime, NULL);
	pthread_create(&connection_handler_thread, NULL, (void *(*)(void *)) connection_handler, NULL);
	while (server_on)
	{

	}
}

int init(char *access_pin)
{

//	init_socket(&server, 1);

	// create main socket for bluetooth communication with hosts
//	bl_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	// allocate memory for local address info

//	memset(&local_address, 0, sizeof(struct sockaddr_rc));
//	local_address.rc_family = AF_BLUETOOTH;
//	local_address.rc_channel = (uint8_t) 1;
//	local_address.rc_bdaddr = *BDADDR_ANY;

	// try to bind socket to address
//	if (bind(bl_socket, (struct sockaddr *) &local_address, sizeof(struct sockaddr_rc)))
//	{
//		perror("Could not bind socket to address\n");
//		return -1;
//	}
//
//	listen(bl_socket, 1);

	PIN = access_pin;
	server_on = true;
	clients_served = 0;
	clients = (struct clients_in_service *) malloc(MAX_SERVED * sizeof(struct clients_in_service));
	for (int i = 0; i < MAX_SERVED; i++)
		clients[i].len = sizeof(struct sockaddr_rc);
	return 0;
}

void server_lifetime()
{
	while (true)
	{
		if (getc(stdin) == 'q')
		{
			pthread_mutex_lock(&mutex);
			server_on = false;
			pthread_mutex_unlock(&mutex);
			return;
		}
	}
}

void connection_handler()
{
	while (server_on)
	{
		if (clients_served >= MAX_SERVED)
		{
			// TODO: Implement CPU free for 500 ms
			continue;
		}

		init_socket(&server, 1);
		listen(server.sock, 1);
		server.client_fd = accept(server.sock, (struct sockaddr *) &server.remote_address, &server.len);

		printf("INFO: ACCEPTED CONNECTION\n");

		if (server.client_fd < 0)
		{
			char message[100];
			sprintf(message, "Could not obtain file descriptor, connection on channel %d\n",
					server.remote_address.rc_channel);
			perror(message);
			exit(-1);
		}

		const int channel = clients_served;
		// initialize new socket for client
		init_socket(&clients[clients_served], channel + 2);

		// transform FD into stream
		server.stream = fdopen(server.client_fd, "w");
		// tell client which channel was assigned for him
		fprintf(server.stream, "%d", channel);

		close(server.sock);
		pthread_t accept_new;
		pthread_create(&accept_new, NULL, (void *(*)(void *)) accept_new_connection, (void *) &channel);

		pthread_mutex_lock(&mutex);
		clients_served++;
		pthread_mutex_unlock(&mutex);
	}
}

void accept_new_connection(void *id)
{
	struct clients_in_service *tmp;
	tmp = &clients[*(int *) id];
	listen(tmp->sock, 1);
	tmp->client_fd = accept(tmp->sock, (struct sockaddr *) &tmp->remote_address, &tmp->len);
	if (tmp->client_fd < 0)
	{
		char message[100];
		sprintf(message, "Could not obtain FD for communication on channel %d\n",
				tmp->remote_address.rc_channel);
		perror(message);
		exit(-1);
	}
}

void init_socket(struct clients_in_service *client, int channel)
{
	client->remote_address.rc_channel = (uint8_t) channel;
	client->remote_address.rc_family = AF_BLUETOOTH;
	client->remote_address.rc_bdaddr = *BDADDR_ANY;
	client->sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);


	if (bind(client->sock, (struct sockaddr *) &client->remote_address, sizeof(struct sockaddr_rc)))
	{
		char message[100];
		sprintf(message, "Could not bind socket to channel %d\n", client->remote_address.rc_channel);
		perror(message);
		exit(-1);
	}
}