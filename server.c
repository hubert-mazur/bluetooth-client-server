//
// Created by hubert on 04.06.2020.
//

#include "server.h"

int main(int argc, char **argv)
{

	init("0000");

	pthread_t server_lifetime_management;
	pthread_t connection_handler_thread;
	pthread_create(&server_lifetime_management, NULL, (void *(*)(void *)) server_lifetime, NULL);
	pthread_create(&connection_handler_thread, NULL, (void *(*)(void *)) connection_handler, NULL);

	pthread_join(server_lifetime_management, NULL);
	pthread_join(connection_handler_thread, NULL);
}

int init(char *access_pin)
{
	PIN = access_pin;
	server_on = true;
	clients_served = 0;
	clients = (struct clients_in_service *) malloc(MAX_SERVED * sizeof(struct clients_in_service));
	for (int i = 0; i < MAX_SERVED; i++)
	{
		clients[i].len = sizeof(struct sockaddr_rc);
		clients[i].conn_established = false;
	}
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

		struct message msg;
		read(server.client_fd, &msg, sizeof(struct message));

		if (msg.flag != HELLO)
		{
			struct message ret = {RESET, "", "SERVER"};
			write(server.client_fd, &ret, sizeof(struct message));
			close(server.sock);
			continue;
		}

		strcpy(clients[clients_served].clients_name, msg.username);

		// initialize new socket for client
		init_socket(&clients[clients_served], channel + 2);


		strcpy(msg.username, "SERVER");
		char ch[10];
		sprintf(ch, "%d", channel + 2);
		strcpy(msg.text, ch);
		msg.flag = REDIRECT;

		write(server.client_fd, &msg, sizeof(struct message));

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
	pthread_mutex_lock(&mutex);
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
	tmp->conn_established = true;
	pthread_mutex_unlock(&mutex);
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

void read_from_clients()
{
	for (int i = 0; i < MAX_SERVED; i++)
	{
		if (!clients[i].conn_established)
			continue;

		struct message *msg = (struct message*) malloc(sizeof(struct message));

		int b_read = read(clients[i].client_fd, &msg, sizeof(struct message));

		if (b_read <= 0)
		{
			free(msg);
			continue;
		}


	}
}
