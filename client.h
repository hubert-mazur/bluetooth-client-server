//
// Created by hubert on 08.06.2020.
//

#ifndef BLUETOOTH_CLIENT_SERVER_CLIENT_H
#define BLUETOOTH_CLIENT_SERVER_CLIENT_H

#include <stdio.h>
// POSIX sys lib: fork, pipe, I/O (read, write)
#include <unistd.h>
// superset of unistd, same
#include <stdlib.h>
#include <ncurses.h>

//Bluetooth
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/sco.h>
#include <fcntl.h>
//socket
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

enum FLAGS { REDIRECT = 0, CLOSE = 1, PLAIN = 2, HELLO = 3, RESET = 4 };


struct connection conn;
char server_name[30];
bool client_on;
pthread_t lifetime_thread;
pthread_t read_thread;
WINDOW *write_window;
WINDOW *read_window;
int cursor_position;

void init();
void connect_to_server();
void handle_message(struct message *msg);
void read_messages();
void send_message(char *msg);


#endif //BLUETOOTH_CLIENT_SERVER_CLIENT_H
