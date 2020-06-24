//
// Created by hubert on 04.06.2020.
//

#include "server.h"

int main(int argc, char **argv)
{
	init();
	pthread_t server_lifetime_management;
	pthread_t connection_handler_thread;
	pthread_t collect;

	pthread_create(&server_lifetime_management, NULL, (void *(*)(void *)) server_lifetime, NULL);
	pthread_create(&connection_handler_thread, NULL, (void *(*)(void *)) connection_handler, NULL);
	pthread_create(&collect, NULL, (void *(*)(void *)) read_from_clients, NULL);

	pthread_join(server_lifetime_management, NULL);
	// thread that handles connection is canceled, because it might be waiting for new connection- put in listening mode, and will not be immediately affected by 'end' flag until client comes
	pthread_cancel(connection_handler_thread);
	pthread_join(collect, NULL);
	close_server();

	printf("SERVER quit\n");
	return 0;
}

int init()
{
	server_on = true;
	clients_served = 0;
	root = NULL;
	return 0;
}

/**
 * Function handling server lifetime, sets flag when user wants to exit by typing q key
 * */
void server_lifetime()
{
	while (true)
	{
		if (getchar() == 'q')
		{
			pthread_mutex_lock(&mutex);
			server_on = false;
			pthread_mutex_unlock(&mutex);
			return;
		}
	}
}

/**
 * Function responsible for new connections with clients, if there are not more than 10 clients server, listens on channel 1 for new connections,
 * returns back to client new channel number and closes connection.
 */
void connection_handler()
{
	while (server_on)
	{
		if (clients_served >= MAX_SERVED)
			continue;

		// initialize socket on channel one for new connection
		if (init_socket(&server, 1))
		{
			perror("Could not initialize socket on channel 1\n");
			exit(-1);
		}

		// put socket into listening mode
		listen(server.sock, 1);

		// accept connections from clients
		server.client_fd = accept(server.sock, (struct sockaddr *) &server.remote_address, &server.len);

//		fcntl(server.sock, F_SETFL, O_NONBLOCK);
		printf("INFO: ACCEPTED CONNECTION\n");

		if (server.client_fd < 0)
		{
			char message[100];
			sprintf(message, "Could not obtain file descriptor, connection on channel %d\n",
					server.remote_address.rc_channel);
			perror(message);
			exit(-1);
		}
		// prepare space for new client in chat server
		struct clients_in_service *elem_to_add = (struct clients_in_service *) malloc(
				sizeof(struct clients_in_service));


		struct message msg;

		// read from client what he sent
		int b_read;
		int received = 0;
		while ((b_read = read(server.client_fd, &msg + received, sizeof(struct message) - received)) > 0 &&
			   received < sizeof(struct message))
			received += b_read;

		// client got to say hello first, and say who is he, otherwise connection can not be established
		if (msg.flag != HELLO)
		{
			printf("Client DID NOT sent HELLO FLAG\n");
			struct message ret = {RESET, "", "SERVER"};
			write(server.client_fd, &ret, sizeof(struct message));
			close(server.sock);
			free(elem_to_add);
			continue;
		}

		strcpy(elem_to_add->clients_name, msg.username);

		// initialize new socket for client

		const int channel = get_next_channel(0);
		printf("CHANNEL IS: %d\n", channel);
		init_socket(elem_to_add, channel);
		pthread_t accept_new;
		pthread_create(&accept_new, NULL, (void *(*)(void *)) accept_new_connection, (void *) elem_to_add);
		channels_busy[channel] = true;
		// spawn thread waiting for client to come on new channel
		printf("new channel is: %d\n", channel);

		strcpy(msg.username, name);
		char ch[10];
		sprintf(ch, "%d", channel);
		strcpy(msg.text, ch);
		msg.flag = REDIRECT;

		// write client information about channel number which will be used for permanent communication
		write(server.client_fd, &msg, sizeof(struct message));
		printf("sent channel for communication\n");
		// close this socket
		close(server.sock);

//		pthread_mutex_lock(&mutex);
		clients_served++;
//		pthread_mutex_unlock(&mutex);
	}
}

void accept_new_connection(void *id)
{
	pthread_mutex_lock(&mutex);
	struct clients_in_service *elem = (struct clients_in_service *) (id);

	listen(elem->sock, 1);
	printf("WAITING for connection on channel %d\n", elem->remote_address.rc_channel);
	elem->client_fd = accept(elem->sock, (struct sockaddr *) &elem->remote_address, &elem->len);
	// set socket in non-blocking mode- read does not block write
	fcntl(elem->client_fd, F_SETFL, O_NONBLOCK);
	if (elem->client_fd < 0)
	{
		char message[100];
		sprintf(message, "Could not obtain FD for communication on channel %d\n",
				elem->remote_address.rc_channel);
		perror(message);
		exit(-1);
	}
	printf("ACCEPTED ON channel: %d\n", elem->remote_address.rc_channel);

	// put new client into clients served list
	if (!root)
	{
		root = elem;
	}
	else
	{
		struct clients_in_service *tmp = root;
		while (tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = elem;
		elem->next = NULL;
	}

//	struct message mess = {IGNORE, "", "SERVER"};
//	write(elem->client_fd, &mess, sizeof(struct message));
	// notify every client in chat of new user
	char *info_msg = (char *) malloc(512 * sizeof(char));
	sprintf(info_msg, "%s has joined the chat", elem->clients_name);
	send_msg(info_msg, name, PLAIN);
	free(info_msg);
	pthread_mutex_unlock(&mutex);
}

int init_socket(struct clients_in_service *client, int channel)
{
	// set channel, protocol etc.
	client->remote_address.rc_channel = (uint8_t) channel;
	client->remote_address.rc_family = AF_BLUETOOTH;
	client->remote_address.rc_bdaddr = *BDADDR_ANY;
	client->sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	if (bind(client->sock, (struct sockaddr *) &client->remote_address, sizeof(struct sockaddr_rc)))
	{
		char message[100];
		sprintf(message, "Could not bind socket to channel %d\n", client->remote_address.rc_channel);
		perror(message);
		return -1;
	}

	return 0;
}

/**
 * Iterate through clients and try to read from them
 */
void read_from_clients()
{
	while (server_on)
	{
		struct clients_in_service *tmp = root;
		while (tmp != NULL)
		{
			struct message *msg = (struct message *) malloc(sizeof(struct message));

			int b_read;
			int received = 0;
			while ((b_read = read(tmp->client_fd, msg + received, sizeof(struct message))) > 0 &&
				   received < sizeof(struct message))
				received += b_read;

			if (received == 0)
			{
				free(msg);
				tmp = tmp->next;
				continue;
			}
			else
			{
				printf("%s\n", msg->text);
				handle_message(msg, tmp);
				tmp = tmp->next;
				free(msg);
			}
		}
	}
}

/**
 * Decide what to do with received message
 */
void handle_message(struct message *msg, struct clients_in_service *client)
{
	switch (msg->flag)
	{
		// normal message, just retransmit
		case PLAIN:
		{
			send_msg(msg->text, msg->username, msg->flag);
			break;
		}
			// client wants to close connection, ok then
		case CLOSE:
		{
			char *tmp = (char *) malloc(512 * sizeof(char));
			sprintf(tmp, "%s has left the chat", client->clients_name);
			close_client_connection(client);
			send_msg(tmp, name, PLAIN);
			free(tmp);
		}
			// unused
		case IGNORE:
		{
			return;
		}
	}
}

// send message to client
void send_msg(const char content[512], const char user[30], int fl)
{
	struct message ms;
	ms.flag = fl;
	strcpy(ms.text, content);
	strcpy(ms.username, user);
	struct clients_in_service *client = root;
	while (client != NULL)
	{
		write(client->client_fd, &ms, sizeof(struct message));
		client = client->next;
	}
}

/**
 * close connection with client and delete him from list
 */
void close_client_connection(struct clients_in_service *client)
{
//	channels_busy[(int)client->remote_address.rc_channel] = false;
	struct clients_in_service *tmp = root;
	if (client == root)
	{
		root = root->next;
		close(client->client_fd);
		free(client);
		return;
	}

	while (tmp->next != client)
	{
		tmp = tmp->next;

	}
	close(client->client_fd);
	struct clients_in_service *nn = tmp->next->next;
	free(client);
	tmp->next = nn;

}
/**
 * Close connections with clients and shutdown server
 */
void close_server()
{
	char *ms = (char *) malloc(512 * sizeof(char));
	strcpy(ms, "SERVER IS BEING SHUT DOWN");
	send_msg(ms, name, PLAIN);

	struct clients_in_service *tmp = root;
	struct message msg = {CLOSE, "", "SERVER"};
	while (tmp != NULL)
	{
		struct clients_in_service *next = tmp->next;
		write(tmp->client_fd, &msg, sizeof(struct message));
		close(tmp->client_fd);
		free(tmp);
		tmp = next;
	}
	root = NULL;
}

int get_next_channel(int start)
{
	for (int i = 4; i < 29; i++)
	{
		if (channels_busy[i] == false && i > start)
			return i;
	}
	return -1;
}

