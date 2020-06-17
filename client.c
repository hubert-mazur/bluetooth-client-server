#include "client.h"


int main(int argc, char **argv)
{
	printf("Type your name: ");
	scanf("%s", user_name);

	printf("Type server address: ");
	scanf("%s", server_address);

	initscr();
	init();
	connect_to_server();


	pthread_create(&read_thread, NULL, (void *(*)(void *)) read_messages, NULL);
	wrefresh(write_window);
	wrefresh(read_window);

	char *txt = (char *) malloc(512 * sizeof(char));

	while (client_on)
	{
		strcpy(txt, "");
		mvwgetstr(write_window, 1, 1, txt);
		if (strcmp(txt, "") != 0)
		{
			send_message(txt);
			wrefresh(write_window);
			wclear(write_window);
			box(write_window, LINES - 5, 0);
		}
	}
	wrefresh(read_window);
	wrefresh(write_window);
	endwin();
	free(txt);
	return 0;
}


void init()
{
	server_address[strlen(server_address)] = '\0';
	client_on = true;
	cursor_position = 1;
	write_window = newwin(10, COLS, LINES - 10, 0);
	read_window = newwin(LINES - 11, COLS, 0, 0);
	refresh();
	wclear(read_window);
	wclear(write_window);
	box(write_window, LINES - 5, 0);
	box(read_window, LINES - 5, 0);

	conn.sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	conn.remote_address.rc_family = AF_BLUETOOTH;
	conn.remote_address.rc_channel = (uint8_t) 1;
	str2ba(server_address, &conn.remote_address.rc_bdaddr);
}

void connect_to_server()
{
	for (int i = 0; i < 5; i++)
	{
		if (connect(conn.sock, (struct sockaddr *) &conn.remote_address, sizeof(conn.remote_address)))
		{
			perror("Could not connect. Retry in 5 seconds\n");
			sleep(5);
			continue;
		}
		printf("Connection established to main \n");
		break;
	}

	struct message msg;
	strcpy(msg.username, user_name);
	strcpy(msg.text, "");
	msg.flag = HELLO;

	write(conn.sock, &msg, sizeof(msg));

	int b_read;
	int received = 0;

	while ((b_read = read(conn.sock, &msg + received, sizeof(struct message) - received)) > 0 &&
		   received < sizeof(struct message))
		received += b_read;


	close(conn.sock);
	uint8_t channel = (uint8_t) strtol(msg.text, NULL, 0);

	struct connection cc;
	cc.sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	cc.remote_address.rc_family = AF_BLUETOOTH;
	cc.remote_address.rc_channel = (uint8_t) channel;
//	printf("Channel is: %d\n", channel);
	str2ba(server_address, &cc.remote_address.rc_bdaddr);

	for (int i = 0; i < 5; i++)
	{
		if (connect(cc.sock, (struct sockaddr *) &cc.remote_address, sizeof(cc.remote_address)))
		{
			perror("Could not connect. Retry in 5 seconds\n");
			sleep(5);
			continue;
		}

		if (cc.sock < 0)
		{
			perror("Error, fd < 0 \n");
			exit(-1);
		}
		printf("Connection established. You are free to write\n");

		fcntl(cc.sock, F_SETFL, O_NONBLOCK);
		memcpy(&conn, &cc, sizeof(struct connection));
		break;
	}
}

void read_messages()
{
	while (client_on)
	{
		struct message *msg = (struct message *) malloc(sizeof(struct message));
		int b_read;
		int received = 0;

		pthread_mutex_lock(&io_mutex);
		while ((b_read = read(conn.sock, msg + received, sizeof(struct message))) > 0 &&
			   received < sizeof(struct message))
			received += b_read;

		pthread_mutex_unlock(&io_mutex);
		if (received > 0)
			handle_message(msg);

		free(msg);
	}
}

void handle_message(struct message *msg)
{
	switch (msg->flag)
	{
		case PLAIN:
		{
			char tmp[543];
			sprintf(tmp, "[%s]: %s\n", msg->username, msg->text);
			wmove(read_window, cursor_position, 1);
			wprintw(read_window, tmp);
			cursor_position++;
			if (cursor_position >= LINES - 12)
			{
				wclear(read_window);
				cursor_position = 1;
			}
			wrefresh(read_window);
			break;
		}

		case CLOSE:
		{
			client_on = false;
			close(conn.sock);
			break;
		}
		case IGNORE:
		{
			return;
		}
		default:
		{
			printf("UNDEFINED FLAG RECEIVED\n");
//			exit(-1);
		}
	}
}

void send_message(char *msg)
{
	struct message mess;
	strcpy(mess.username, user_name);
	strcpy(mess.text, msg);
	mess.flag = PLAIN;
	if (!strcmp(mess.text, "[\\q]"))
		mess.flag = CLOSE;

	pthread_mutex_lock(&io_mutex);
	write(conn.sock, &mess, sizeof(struct message));
	pthread_mutex_unlock(&io_mutex);

	if (!strcmp(mess.text, "[\\q]"))
	{
		close(conn.sock);
		client_on = false;
	}
}
