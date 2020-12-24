#include <gtk/gtk.h>
#include <stdio.h>

#include "list.h"

#define APPLICATION(a) ((application_t*)a)
#define GROUP(a) ((group_t*)a)

struct application_t;
typedef struct group_t
{
	struct application_t* app;
	GtkListBoxRow* row;
	char* name;
	list_t* messages;
} group_t;

typedef struct application_t
{
	GObject* window;
	GtkListBox* group_list;
	GtkEntry* message_input;
	GtkButton* send_button;
	list_t* groups;
	group_t* current_group;
} application_t;

static int group_compare(void* comp, void* cur)
{
	if (GROUP(cur)->row == comp)
	{
		return 0;
	}
	return 1;
}

static void send_message(GtkWidget *widget, gpointer data)
{
	struct application_t* app = data;
	const gchar* message = gtk_entry_get_text(app->message_input);

	if (app->current_group == NULL)
	{
		return;
	}
	g_print("Send message to %s: %s\n", app->current_group->name, message);
	gtk_entry_set_text(app->message_input, "");
}

static void select_group(GtkListBox* list, GtkListBoxRow* row, gpointer data)
{
	group_t* group = list_find(APPLICATION(data)->groups, row, &group_compare);
	group->app->current_group = group;
	g_print("group_selected: %s\n", group->name);
}

static void add_group(application_t* app, const char* name)
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
	gtk_container_add(GTK_CONTAINER(app->group_list), row);

	group_t* group = malloc(sizeof(group_t));
	group->app = app;
	group->name = malloc(sizeof(char) * strlen(name));
	strcpy(group->name, name);
	group->row = (void*)row;
	group->messages = list_new();
	list_append(app->groups, group);

	gtk_widget_show_all(row);
}

int main(int argc, char *argv[])
{
	application_t* app = malloc(sizeof(application_t));
	app->current_group = NULL;

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

	app->group_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "group_list"));
	app->message_input = GTK_ENTRY(gtk_builder_get_object(builder, "message_input"));
	app->send_button = GTK_BUTTON(gtk_builder_get_object(builder, "send_button"));

	g_signal_connect(app->group_list, "row-selected", G_CALLBACK(select_group), app);
	g_signal_connect(app->message_input, "activate", G_CALLBACK(send_message), app);
	g_signal_connect(app->send_button, "clicked", G_CALLBACK(send_message), app);

	app->groups = list_new();
	add_group(app, "Test Group 1");
	add_group(app, "Test Group 2");

	gtk_main();

	return 0;
}
