#include "client.h"


int main(int argc, char **argv)
{
//	printf("Type server's name: ");
//	scanf("%s", server_name);
	strcpy(server_name, "hubert-Lenovo");
	init();
	printf("Search for BT Devices...\n");
	connect_to_server();
//	noecho();
	cbreak();
//	wtimeout(read_window, 100);
//	wtimeout(write_window, 100);
//	timeout(100);
	initscr();
	client_on = true;
	write_window = newwin(10, COLS, LINES - 10, 0);
	read_window = newwin(LINES - 11, COLS, 0, 0);
	refresh();
	box(write_window, LINES - 5, 0);
	box(read_window, LINES - 5, 0);
	wrefresh(write_window);
	wrefresh(read_window);

	pthread_create(&read_thread, NULL, (void *(*)(void *)) read_messages, NULL);

	char *txt = (char *) malloc(512 * sizeof(char));

	while (client_on)
	{
		txt[0] = '\0';
		mvwgetstr(write_window, 1, 1, txt);
		if (txt[0] != '\0')
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
	conn.sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	conn.remote_address.rc_family = AF_BLUETOOTH;
	conn.remote_address.rc_channel = (uint8_t) 1;
	str2ba("58:00:E3:4D:0D:B8", &conn.remote_address.rc_bdaddr);
	client_on = true;
	cursor_position = 1;
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

	struct message msg = {HELLO, "", "testUser"};

	write(conn.sock, &msg, sizeof(msg));
	int b_read = 0;
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
	str2ba("58:00:E3:4D:0D:B8", &cc.remote_address.rc_bdaddr);


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
//		fcntl(cc.sock, F_SETFL, O_NONBLOCK);
		break;
	}
}

void read_messages()
{
	while (client_on)
	{
//		wrefresh(read_window);
//		wrefresh(write_window);
		struct message *msg = (struct message *) malloc(sizeof(struct message));

		int b_read;
		int received = 0;
//		pthread_mutex_lock(&io_mutex);
		while ((b_read = read(conn.sock, msg + received, sizeof(struct message))) >= 0 &&
			   received < sizeof(struct message))
			received += b_read;

		wrefresh(read_window);
//		pthread_mutex_unlock(&io_mutex);
		if (received > 0)
			handle_message(msg);
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
			wrefresh(read_window);
//			wrefresh(write_window);
			free(msg);
			break;
		}

		case CLOSE:
		{
			client_on = false;
			close(conn.sock);
			free(msg);
			break;
		}
	}
}

void send_message(char *msg)
{
	struct message mess;
	strcpy(mess.username, "testUser");
	strcpy(mess.text, msg);
	mess.flag = PLAIN;
	if (!strcmp(mess.text, "[/q]"))
	{
		mess.flag = CLOSE;
		client_on = false;
		close(conn.sock);
	}
//	pthread_mutex_lock(&io_mutex);
	write(conn.sock, &mess, sizeof(struct message));
//	pthread_mutex_unlock(&io_mutex);
}
