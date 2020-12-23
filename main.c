#include <gtk/gtk.h>

typedef struct application_t
{
	GObject* window;
	GtkListBox* group_list;
	GtkEntry* message_input;
	GtkButton* send_button;
} application_t;

static void send_message(GtkWidget *widget, gpointer data)
{
	struct application_t* app = data;
	const gchar* message = gtk_entry_get_text(app->message_input);

	g_print("Send message: %s\n", message);
	gtk_entry_set_text(app->message_input, "");
}

static void add_group(application_t* app, char* name)
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
	// TODO: Connect the signals here

	gtk_widget_show_all(row);
}

int main(int argc, char *argv[])
{
	application_t* app = malloc(sizeof(application_t));
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
	g_signal_connect(app->message_input, "activate", G_CALLBACK(send_message), app);
	g_signal_connect(app->send_button, "clicked", G_CALLBACK(send_message), app);

	add_group(app, "Test Group 1");
	add_group(app, "Test Group 2");

	gtk_main();

	return 0;
}
