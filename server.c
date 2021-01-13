#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "list.h"

typedef struct socket_t
{
	int fd;
	struct sockaddr_in addr;
	fd_set read_set;
	list_t* clients;
} socket_t;

int socket_server_init(socket_t* sock)
{
	int opt = 1;

	FD_ZERO(&sock->read_set);
	sock->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock->fd == -1)
	{
		fprintf(stderr, "Unable to create server socket fd\n");
		return -1;
	}

	sock->addr.sin_family = AF_INET;
	sock->addr.sin_addr.s_addr = INADDR_ANY;
	sock->addr.sin_port = htons(8888);

	if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)))
	{
		fprintf(stderr, "Unable to set socket resuse addr\n");
	}

	if (bind(sock->fd, (struct sockaddr*)&sock->addr, sizeof(sock->addr)))
	{
		fprintf(stderr, "Unable to bind server socket\n");
		return -1;
	}
	listen(sock->fd, 100);

	sock->clients = list_new();

	return 0;
}

void socket_server_remove(socket_t* sock, int client)
{
	node_t* cur = sock->clients->head;
	while (cur)
	{
		int fd = (int)cur->data;
		if (fd == client)
		{
			close(fd);
			list_remove_node(sock->clients, cur);
			return;
		}
		cur = cur->next;
	}
}

int socket_handle(socket_t* sock, char* data, size_t len)
{
	JsonParser* parser = json_parser_new();
	GError* error = NULL;
	if (!json_parser_load_from_data(parser, data, len, &error))
	{
		return -1;
	}

	JsonNode* root = json_parser_get_root(parser);
	JsonObject* root_obj = json_node_get_object(root);
	if (!root_obj)
	{
		return -1;
	}

	JsonNode* method_node = json_object_get_member(root_obj, "method");
	if (!method_node)
	{
		return -1;
	}
	const gchar* method = json_node_get_string(method_node);

	g_print("Got method: %s\n", method);

	g_object_unref(parser);
	return 0;
}

int socket_server_process(socket_t* sock)
{
	// zero and add server socket
	FD_ZERO(&sock->read_set);
	FD_SET(sock->fd, &sock->read_set);
	int max_fd = sock->fd;
	char buf[2048];

	// add all the clients
	node_t* cur = sock->clients->head;
	while (cur)
	{
		int fd = (int)cur->data;
		FD_SET(fd, &sock->read_set);
		if (fd > max_fd)
		{
			max_fd = fd;
		}
		cur = cur->next;
	}

	int result = select(max_fd + 1, &sock->read_set, NULL, NULL, NULL);

	if (result < 0)
	{
		fprintf(stderr, "Unable to select socket list\n");
		return 0;
	}

	if (FD_ISSET(sock->fd, &sock->read_set))
	{
		int new_sock, c;
		struct sockaddr_in client;
		c = sizeof(struct sockaddr_in);
		if ((new_sock = accept(sock->fd,  (struct sockaddr *)&client, (socklen_t*)&c)) < 0)
		{
			fprintf(stderr, "Unable to accept new client: %s\n", strerror(errno));
			//return -1;
			return 0;
		}
		list_append(sock->clients, (void*)new_sock);
	}

	cur = sock->clients->head;
	while (cur)
	{
		int fd = (int)cur->data;
		printf("processing client: %d\n", fd);
		if (FD_ISSET(fd, &sock->read_set))
		{
			int len;
			if ((len = read(fd, buf, 2046)) == 0)
			{
				printf("REMOVE CLIENT\n");
				cur = cur->next;
				socket_server_remove(sock, fd);
				continue;
			}
			else
			{
				buf[len] = '\0';
				socket_handle(sock, buf, len);
				printf(buf);
			}
		}
		cur = cur->next;
	}

	return 0;
}

int main()
{
	socket_t server;
	socket_server_init(&server);
	while (!socket_server_process(&server))
	{
		printf("Loop\n");
	}

	return 0;
}
