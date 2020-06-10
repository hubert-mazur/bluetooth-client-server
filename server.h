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

enum FLAGS { REDIRECT = 0, CLOSE = 1, PLAIN = 2, HELLO = 3, RESET = 4, OK = 5 };

int clients_served;
char *PIN;
struct clients_in_service server;
struct clients_in_service *clients;
pthread_mutex_t mutex;
bool server_on;
struct clients_in_service *root;


int init(char *access_pin);

void server_lifetime();

void connection_handler();

void accept_new_connection(void *id);

void init_socket(struct clients_in_service *client, int channel);

void read_from_clients();

void handle_message(struct message *msg, struct clients_in_service *client);

void send_msg(const char content[512], const char user[30], int fl);

void close_client_connection(struct clients_in_service *client);


#endif //BLUETOOTH_CLIENT_SERVER_SERVER_H
