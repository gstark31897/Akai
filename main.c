#include <gtk/gtk.h>
#include <stdio.h>

#include "list.h"

#define APPLICATION(a) ((application_t*)a)
#define CHANNEL(a) ((channel_t*)a)


struct application_t;

typedef struct message_t
{
	char* sender;
	char* body;
} message_t;

typedef struct channel_t
{
	struct application_t* app;
	GtkListBoxRow* row;
	char* name;
	char* msg_tmp;
	list_t* messages;
} channel_t;

typedef struct application_t
{
	GObject* window;
	GtkListBox* channel_list;
	GtkBox* message_list;
	GtkTextView* message_input;
	GtkScrolledWindow* message_scroll;
	GtkTextBuffer* message_buffer;
	GtkButton* send_button;
	list_t* channels;
	channel_t* current_channel;
} application_t;

static application_t* g_app;


static void clear_message(GtkWidget* widget, gpointer parent)
{
	gtk_widget_destroy(widget);
}

static void clear_messages(application_t* app)
{
	gtk_container_foreach(GTK_CONTAINER(app->message_list), &clear_message, (void*)0);
	gtk_widget_show_all(GTK_WIDGET(app->message_list));
}

static void display_message(application_t* app, const char* sender, const char* message)
{
	GtkWidget* row = gtk_list_box_row_new();
	GtkWidget* grid = gtk_grid_new();
	GtkWidget* head = gtk_label_new(sender);
	gtk_label_set_xalign(GTK_LABEL(head), 0);
	GtkWidget* body = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(body), GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(body), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(body), FALSE);
	gtk_widget_set_hexpand(body, TRUE);
	gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(body)), message, -1);
	gtk_grid_attach(GTK_GRID(grid), head, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), body, 0, 1, 2, 1);
	gtk_container_add(GTK_CONTAINER(row), grid);
	gtk_container_add(GTK_CONTAINER(app->message_list), row);
	gtk_widget_show_all(row);
}

static void load_messages(application_t* app)
{
	node_t* cur = app->current_channel->messages->head;
	while (cur)
	{
		message_t* msg = (message_t*)(cur->data);
		display_message(app, msg->sender, msg->body);
		cur = cur->next;
	}
}

static void add_message(application_t* app, const char* sender, const char* message)
{
	message_t* msg = malloc(sizeof(message_t));
	msg->sender = malloc(sizeof(char) * (strlen(sender) + 1));
	strcpy(msg->sender, sender);
	msg->body = malloc(sizeof(char) * (strlen(message) + 1));
	strcpy(msg->body, message);
	list_append(app->current_channel->messages, msg);

	display_message(app, sender, message);
}

static void test(GtkWidget* widget, message_t* message)
{
	g_print("Test from %s: %s\n", message->sender, message->body);
}

static void send_message(GtkWidget* widget, gpointer data)
{
	struct application_t* app = data;
	GtkTextIter start;
	GtkTextIter end;
	gtk_text_buffer_get_start_iter(app->message_buffer, &start);
	gtk_text_buffer_get_end_iter(app->message_buffer, &end);
	const gchar* message = gtk_text_buffer_get_text(app->message_buffer, &start, &end, TRUE);

	if (app->current_channel == NULL)
	{
		return;
	}

	add_message(app, "Me", message);

	gtk_text_buffer_set_text(app->message_buffer, "", 0);
}

static gboolean key_message(GtkWidget* widget, GdkEventKey* event)
{
	if (event->keyval == GDK_KEY_Return && !(event->state & GDK_SHIFT_MASK))// && !event->is_modifier)
	{
		send_message(widget, g_app);
		return TRUE;
	}
	return FALSE;
}

static gboolean resize_message(GtkWidget* widget, gpointer data)
{
	application_t* app = g_app;
	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(app->message_scroll));
	gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));
	gtk_scrolled_window_set_vadjustment(app->message_scroll, adj);
	return FALSE;
}


static int channel_compare(void* comp, void* cur)
{
	if (CHANNEL(cur)->row == comp)
	{
		return 0;
	}
	return 1;
}

static void select_channel(GtkListBox* list, GtkListBoxRow* row, gpointer data)
{
	channel_t* channel = list_find(APPLICATION(data)->channels, row, &channel_compare);
	application_t* app = channel->app;
	// save the text input the user was typing for the channel
	GtkTextIter start;
	GtkTextIter end;
	gtk_text_buffer_get_start_iter(app->message_buffer, &start);
	gtk_text_buffer_get_end_iter(app->message_buffer, &end);
	const gchar* message = gtk_text_buffer_get_text(app->message_buffer, &start, &end, TRUE);
	size_t len = strlen(message);
	if (app->current_channel && len > 0)
	{
		app->current_channel->msg_tmp = malloc(sizeof(char) * (len + 1));
		strcpy(app->current_channel->msg_tmp, message);
	}
	app->current_channel = channel;
	if (channel->msg_tmp)
	{
		gtk_text_buffer_set_text(app->message_buffer, channel->msg_tmp, -1);
		free(channel->msg_tmp);
		channel->msg_tmp = NULL;
	}
	else
	{
		gtk_text_buffer_set_text(app->message_buffer, "", 0);
	}
	// load the messages into the message panle
	clear_messages(app);
	load_messages(app);
}

static void add_channel(application_t* app, const char* name)
{
	GtkWidget* row = gtk_list_box_row_new();
	GtkWidget* grid = g_object_new(GTK_TYPE_GRID, 
			"orientation", GTK_ORIENTATION_VERTICAL,
			"border-width", 3,
			NULL);
	GtkWidget* label = gtk_label_new(name);
	gtk_widget_set_hexpand(label, TRUE);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_container_add(GTK_CONTAINER(grid), label);
	gtk_container_add(GTK_CONTAINER(row), grid);
	gtk_container_add(GTK_CONTAINER(app->channel_list), row);

	channel_t* channel = malloc(sizeof(channel_t));
	channel->app = app;
	channel->name = malloc(sizeof(char) * strlen(name));
	strcpy(channel->name, name);
	channel->msg_tmp = NULL;
	channel->row = (void*)row;
	channel->messages = list_new();
	list_append(app->channels, channel);

	gtk_widget_show_all(row);
}

int main(int argc, char *argv[])
{
	g_app = malloc(sizeof(application_t));
	application_t* app = g_app;
	app->current_channel = NULL;

	GtkBuilder* builder;
	GError* error = NULL;

	gtk_init(&argc, &argv);

	builder = gtk_builder_new();
	if (gtk_builder_add_from_file(builder, "akai.ui", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	app->window = gtk_builder_get_object(builder, "window");
	g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	app->channel_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "channel_list"));
	app->message_scroll = GTK_SCROLLED_WINDOW(gtk_builder_get_object(builder, "message_scroll"));
	app->message_list = GTK_BOX(gtk_builder_get_object(builder, "message_list"));
	app->message_input = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "message_input"));
	app->message_buffer = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(app->message_input));
	app->send_button = GTK_BUTTON(gtk_builder_get_object(builder, "send_button"));

	g_signal_new("message",
			G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST,
			0, NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1, G_TYPE_POINTER);

	message_t test_message;
	test_message.sender = "octalus";
	test_message.body = "hello";
	g_signal_connect(app->window, "message", G_CALLBACK(test), NULL);
	g_signal_emit_by_name(app->window, "message", &test_message);

	g_signal_connect(app->channel_list, "row-selected", G_CALLBACK(select_channel), app);
	g_signal_connect(app->message_list, "size-allocate", G_CALLBACK(resize_message), app);
	//g_signal_connect(app->message_input, "activate", G_CALLBACK(send_message), app);
	g_signal_connect(app->message_input, "key-press-event", G_CALLBACK(key_message), NULL);
	g_signal_connect(app->send_button, "clicked", G_CALLBACK(send_message), app);

	app->channels = list_new();
	add_channel(app, "Test Group 1");
	add_channel(app, "Test Group 2");

	gtk_main();

	return 0;
}
