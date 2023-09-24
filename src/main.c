#include <gtk/gtk.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct Namespace {
  char *name;
  char *fullPath;
} Namespace;

typedef struct Language {
  char *lngName;
  size_t namaespaces_length;
  struct Namespace **namespaces;
} Language;

typedef struct Langs {
  size_t languages_length;
  struct Language **language;
} Langs;

GtkFileDialog *file_picker;
GtkWidget *window;
GtkWidget *grid;
GtkWidget *open_file_select_button;

/**
 * @param *dir, caller owns, language directory containing json langNamespaces
 */
static struct Language *get_locales_lang_struct(GFile *dir) {

  Language *lang = malloc(sizeof(Language));
  lang->namespaces = malloc(500 * sizeof(Namespace));
  lang->namaespaces_length = 500;
  for (int i = 0; i < lang->namaespaces_length; i++) {
    lang->namespaces[i] = NULL;
  }

  if (dir == NULL) {
    // File select cancel
    return lang;
  }

  lang->lngName = malloc(strlen(g_file_get_parse_name(dir)));
  strcpy(lang->lngName, g_file_get_parse_name(dir));

  GFileEnumerator *namespaces =
      g_file_enumerate_children(dir, NULL, G_FILE_QUERY_INFO_NONE, NULL, NULL);

  if (namespaces == NULL) {
    return lang;
  }

  while (1) {
    GFileInfo *ns = g_file_enumerator_next_file(namespaces, NULL, NULL);
    if (ns == NULL) {
      break;
    }

    const char *name = g_file_info_get_name(ns);
    GFile *nsFile = g_file_get_child(dir, name);
    if (nsFile == NULL) {
      break;
    }

    if (strstr(name, ".json") == NULL) {
      continue;
    }

    char *fullPath = g_file_get_path(nsFile);
    Namespace *nsStruct = malloc(sizeof(Namespace));
    nsStruct->name = (char *)malloc(strlen(name));
    strcpy(nsStruct->name, name);
    nsStruct->fullPath = (char *)malloc(strlen(fullPath));
    strcpy(nsStruct->fullPath, fullPath);

    for (int i = 0; i < lang->namaespaces_length; i++) {
      if (lang->namespaces[i] != NULL) {
        continue;
      }

      lang->namespaces[i] = nsStruct;
      break;
    }

    g_object_unref(ns);
  }

  g_object_unref(namespaces);
  namespaces = NULL;

  return lang;
  // return NULL;
}

/**
 * @param *dir GFile, caller owns, locales root
 */
static struct Langs *get_locales_struct(GFile *dir) {

  Langs *langs;
  langs = malloc(sizeof(Langs));
  langs->language = malloc(500 * sizeof(Language));
  langs->languages_length = 500;
  for (int i = 0; i < langs->languages_length; i++) {
    langs->language[i] = NULL;
  }

  if (dir == NULL) {
    // File select cancel
    return langs;
  }

  GFileEnumerator *locales =
      g_file_enumerate_children(dir, NULL, G_FILE_QUERY_INFO_NONE, NULL, NULL);

  if (locales == NULL) {
    return langs;
  }

  while (1) {
    GFileInfo *file = g_file_enumerator_next_file(locales, NULL, NULL);

    if (file == NULL) {
      break;
    }

    const char *name = g_file_info_get_name(file);
    GFile *langDir = g_file_get_child(dir, name);
    if (langDir == NULL) {
      continue;
    }

    Language *lang = get_locales_lang_struct(langDir);
    for (int i = 0; i < langs->languages_length; i++) {
      if (langs->language[i] != NULL) {
        continue;
      }

      langs->language[i] = lang;
      break;
    }

    g_object_unref(file);
  }

  g_object_unref(locales);
  locales = NULL;
  return langs;
}

static void handle_translation_dir(GObject *obj, GAsyncResult *result,
                                   gpointer user_data) {
  GFile *dir = gtk_file_dialog_select_folder_finish(file_picker, result, NULL);
  Langs *langs = get_locales_struct(dir);

  Language *en_lang = NULL;
  for (int i = 0; i < langs->languages_length; i++) {
    Language *lang = langs->language[i];
    if (lang == NULL) {
      break;
    }
    if (strstr(lang->lngName, "/en") == NULL) {
      continue;
    }
    en_lang = lang;
  }

  if (en_lang == NULL) {
    g_print("Did not find default lang.");
    return;
  }

  for (int i = 0; i < en_lang->namaespaces_length; i++) {
    Namespace *ns = en_lang->namespaces[i];
    if (ns == NULL) {
      continue;
    }

    GtkWidget *nsButton = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(nsButton), ns->name);

    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(nsButton), 1, i, 1, 1);
  }

  g_print("%s", en_lang->lngName);
  return;

  // Add button for each name space for en lang
  // for (int i = 0; i < langs->languages_length; i++) {
  //   if (langs->language[i] == NULL) {
  //     break;
  //   }
  //
  //   if (!strcmp(langs->language[i]->lngName, "en")) {
  //     continue;
  //   }
  //
  //   for (int j = 0; j < langs->language[i]->namaespaces_length; j++) {
  //     Namespace *ns = langs->language[i]->namespaces[j];
  //     if (ns == NULL) {
  //       break;
  //     }
  //     GtkWidget *nsBtn = gtk_button_new();
  //     gtk_button_set_label(GTK_BUTTON(nsBtn), ns->name);
  //
  //     gtk_grid_attach(GTK_GRID(grid), nsBtn, 0, 1 + j, 1, 1);
  //   }
  // }

  //  for (int i = 0; i < langs->languages_length; i++) {
  //    if (langs->language[i] == NULL) {
  //      break;
  //    }
  //
  //    if (!strcmp(langs->language[i]->lngName, "en")) {
  //      continue;
  //    }
  //
  //    Namespace *namespaces = langs->language[i]->namespaces[0];
  //    if (namespaces == NULL) {
  //      break;
  //    }
  //
  //    for (int j = 0; j < langs->languages_length; j++) {
  //
  //      Language *lang = langs->language[j];
  //      if (lang == NULL) {
  //        break;
  //      }
  //      for (int k = 0; k < lang->namaespaces_length; k++) {
  //        Namespace *ns = lang->namespaces[k];
  //
  //        if (ns == NULL) {
  //          break;
  //        }
  //
  //        if (!strcmp(ns->name, namespaces->name)) {
  //          continue;
  //        }
  //
  //        g_print("\n\n%s", ns->fullPath);
  //
  //        GFile *file = g_file_new_for_path(ns->fullPath);
  //        gchar **contents;
  // gsize *length;
  //
  //        g_file_load_contents(file, NULL, contents, length, NULL, NULL);
  //
  //        g_print("\n\n\n\n%zu\n\n\n\n", *length);
  //        g_print("\n\n\n\n%s\n\n\n\n", *contents);
  //
  //        g_object_unref(*contents);
  //        g_object_unref(file);
  //      }
  //    }
  //    break;
  //  }
  g_object_unref(dir);
  dir = NULL;
};

static void open_file_picker(GtkWidget *widget, gpointer user_data) {
  file_picker = gtk_file_dialog_new();
  gtk_file_dialog_set_title(file_picker, "Pick directory");
  gtk_file_dialog_select_folder(file_picker, GTK_WINDOW(window), NULL,
                                handle_translation_dir, user_data);
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
  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
