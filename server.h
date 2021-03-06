//
// Created by hubert on 04.06.2020.
//

#ifndef BLUETOOTH_CLIENT_SERVER_SERVER_H
#define BLUETOOTH_CLIENT_SERVER_SERVER_H

#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <pthread.h>
#include <stdbool.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <fcntl.h>

#define MAX_SERVED 10

struct clients_in_service {
	char clients_name[30];
	int client_fd;
	int sock;
	struct sockaddr_rc remote_address;
	socklen_t len;
	struct clients_in_service *next;
};

struct message {
	int flag;
	char text[512];
	char username[30];
};

enum FLAGS { REDIRECT = 0, CLOSE = 1, PLAIN = 2, HELLO = 3, RESET = 4, IGNORE = 5 };

struct clients_in_service server;
struct clients_in_service *clients;
struct clients_in_service *root;
const char name[] = "SERVER";
volatile bool server_on;
bool channels_busy[29] = {0};
int clients_served;
pthread_mutex_t mutex;

int init();

void server_lifetime();

void connection_handler();

void accept_new_connection(void *id);

int init_socket(struct clients_in_service *client, int channel);

void read_from_clients();

void handle_message(struct message *msg, struct clients_in_service *client);

void send_msg(const char content[512], const char user[30], int fl);

void close_client_connection(struct clients_in_service *client);

void close_server();

int get_next_channel(int start);

#endif //BLUETOOTH_CLIENT_SERVER_SERVER_H
