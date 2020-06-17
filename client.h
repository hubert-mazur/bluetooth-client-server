//
// Created by hubert on 08.06.2020.
//

#ifndef BLUETOOTH_CLIENT_SERVER_CLIENT_H
#define BLUETOOTH_CLIENT_SERVER_CLIENT_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <signal.h>
#include <errno.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/sco.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdbool.h>
#include<pthread.h>

struct connection {
	int sock;
	struct sockaddr_rc remote_address;
	socklen_t len;
};

struct message {
	int flag;
	char text[512];
	char username[30];
};

enum FLAGS { REDIRECT = 0, CLOSE = 1, PLAIN = 2, HELLO = 3, RESET = 4, IGNORE = 5 };

struct connection conn;
volatile bool client_on;
char user_name[30];
char server_address[32];
int cursor_position;
pthread_t read_thread;
pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;
WINDOW *write_window;
WINDOW *read_window;

void init();

void connect_to_server();

void handle_message(struct message *msg);

void read_messages();

void send_message(char *msg);


#endif //BLUETOOTH_CLIENT_SERVER_CLIENT_H
