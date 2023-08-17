#include <gtk/gtk.h>

GtkFileDialog *file_picker;
GtkWidget *window;
GtkWidget *grid;
GtkWidget *open_file_select_button;

static void print_hello(GtkWidget *widget, gpointer data) {
  g_print("Hello Wordl\n");
}

static void on_file_select(GObject *obj, GAsyncResult *result,
                           gpointer user_data) {
  GFile *file = gtk_file_dialog_select_folder_finish(file_picker, result, NULL);

  char *name = g_file_get_parse_name(file);
  printf("%i", file == NULL);
  printf("%s", name);

  free(file);
  file = NULL;
};

static void open_file_picker(GtkWidget *widget, gpointer user_data) {
  file_picker = gtk_file_dialog_new();
  gtk_file_dialog_select_folder(file_picker, GTK_WINDOW(window), NULL,
                                on_file_select, user_data);
}

static void activate(GtkApplication *app, gpointer user_data) {
  window = gtk_application_window_new(app);

  gtk_window_set_title(GTK_WINDOW(window), "Window");

  grid = gtk_grid_new();
  gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(grid));

  open_file_select_button = gtk_button_new();
  gtk_button_set_label(GTK_BUTTON(open_file_select_button), "Open file");
  g_signal_connect(GTK_BUTTON(open_file_select_button), "clicked",
                   G_CALLBACK(open_file_picker), NULL);

  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(open_file_select_button), 0, 0, 1,
                  1);

  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
  // gtk_widget_show(window);
  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  printf("%d", status);
  g_object_unref(app);

  return status;
}
