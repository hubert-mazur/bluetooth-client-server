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

#define MAX_SERVED 10

struct clients_in_service {
	int client_fd;
	char* clients_name;
	struct sockaddr_rc remote_address;
	socklen_t len;
	FILE *stream;
	int sock;
};

int bl_socket;
char *PIN;
int clients_served;
bool server_on;
pthread_mutex_t mutex;
struct clients_in_service *clients;
struct clients_in_service server;

int init(char *access_pin);
void server_lifetime();
void connection_handler();
void accept_new_connection(void * id);
void init_socket(struct clients_in_service *client, int channel);

#endif //BLUETOOTH_CLIENT_SERVER_SERVER_H
