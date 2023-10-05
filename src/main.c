#include "cJSON.h"
#include <gtk/gtk.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct Namespace {
  char *name;
  char *fullPath;
  cJSON *localales_file_data;
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
GtkGrid *grid;
GtkWidget *open_file_select_button;
GtkButton *save_btn;
Langs *langs;

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
    nsStruct->localales_file_data = NULL;

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

typedef struct SelectedNamespaceKeyParams {
  cJSON *key;
  Namespace *ns;
  cJSON *full;
} SelectedNamespaceKeyParams;
static void on_translation_node_text_change(GtkEditable *entry,
                                            SelectedNamespaceKeyParams *pt) {

  gchar *currentInput = gtk_editable_get_chars(entry, 0, -1);
  gchar *old = pt->key->valuestring;

  pt->key->valuestring = currentInput;
  free(old);
}
static void select_namespace_key(GtkWidget *widget,
                                 SelectedNamespaceKeyParams *params) {
  for (int i = 0; i < langs->languages_length; i++) {
    Language *l = langs->language[i];
    if (l == NULL) {
      break;
    }

    GtkEntry *input = GTK_ENTRY(gtk_entry_new());
    GtkEntryBuffer *buf = gtk_entry_buffer_new(
        params->key->valuestring, strlen(params->key->valuestring));
    g_signal_connect(GTK_ENTRY(input), "changed",
                     G_CALLBACK(on_translation_node_text_change), params);
    gtk_entry_set_buffer(input, buf);
    gtk_grid_attach(grid, GTK_WIDGET(input), 4, i, 1, 1);
  }
}

static void select_namespace(GtkWidget *widget, Namespace *clicked_ns) {
  for (int i = 0; i < langs->languages_length; i++) {
    Language *lang = langs->language[i];
    if (lang == NULL) {
      return;
    }

    for (int j = 0; j < lang->namaespaces_length; j++) {
      Namespace *ns = lang->namespaces[j];
      if (ns == NULL || strstr(ns->name, clicked_ns->name) == 0) {
        break;
      }

      gchar **jsonRaw = malloc(sizeof(gchar *));
      gsize *jsonRawLength = malloc(sizeof(gsize *));
      g_file_get_contents(ns->fullPath, jsonRaw, jsonRawLength, NULL);
      cJSON *json = cJSON_ParseWithLength(*jsonRaw, *jsonRawLength);
      ns->localales_file_data = json;
      cJSON *root = json->child;
      cJSON *child = root->child;
      for (int i = 0; i < cJSON_GetArraySize(json->child); i++) {
        if (child != NULL) {

          GtkButton *keyBtn = GTK_BUTTON(gtk_button_new());
          gtk_button_set_label(keyBtn, child->string);
          SelectedNamespaceKeyParams *selected =
              malloc(sizeof(SelectedNamespaceKeyParams));
          selected->ns = ns;
          selected->key = child;
          selected->full = json;
          g_signal_connect(keyBtn, "clicked", G_CALLBACK(select_namespace_key),
                           selected);
          gtk_grid_attach(grid, GTK_WIDGET(keyBtn), 3, i, 1, 1);

          child = child->next;
        } else {
          break;
        }
      }

      free(jsonRawLength);
      free(*jsonRaw);
      free(jsonRaw);
    }
  }
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
  if (dir == NULL) {
    return;
  }

  langs = get_locales_struct(dir);

  Language *default_lang = NULL;
  for (int i = 0; i < langs->languages_length; i++) {
    Language *lang = langs->language[i];
    if (lang == NULL) {
      break;
    }
    if (strstr(lang->lngName, "/en") == NULL) {
      continue;
    }
    default_lang = lang;
  }

  if (default_lang == NULL) {
    g_print("Did not find default lang.");
    return;
  }

  for (int i = 0; i < default_lang->namaespaces_length; i++) {
    Namespace *ns = default_lang->namespaces[i];
    if (ns == NULL) {
      continue;
    }

    GtkWidget *nsButton = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(nsButton), ns->name);
    g_signal_connect(GTK_BUTTON(nsButton), "clicked",
                     G_CALLBACK(select_namespace), ns);

    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(nsButton), 1, i, 1, 1);
  }

  g_object_unref(dir);
  dir = NULL;
};
static void save_translations_to_file(GtkButton *save_button,
                                      gpointer user_data) {
  g_print("\n\n##############################\nSaving");

  for (int i = 0; i < langs->languages_length; i++) {
    Language *lang = langs->language[i];
    if (lang == NULL) {
      break;
    }
    for (int j = 0; j < lang->namaespaces_length; j++) {
      Namespace *ns = lang->namespaces[j];
      if (ns == NULL) {
        break;
      }

      cJSON *ns_json = ns->localales_file_data;

      if (ns_json == NULL) {
        break;
      }

      char *json_str = cJSON_Print(ns_json);
      g_file_set_contents(ns->fullPath, json_str, strlen(json_str), NULL);

      g_print("\n\n%s\n\n", json_str);
    }
  }
  g_print("\n\n##############################\nDone saving");
}
static void open_file_picker(GtkWidget *widget, gpointer user_data) {
  file_picker = gtk_file_dialog_new();
  gtk_file_dialog_set_title(file_picker, "Pick directory");
  gtk_file_dialog_select_folder(file_picker, GTK_WINDOW(window), NULL,
                                handle_translation_dir, user_data);
}

static void activate(GtkApplication *app, gpointer user_data) {
  window = gtk_application_window_new(app);

  gtk_window_set_title(GTK_WINDOW(window), "Window");

  grid = GTK_GRID(gtk_grid_new());
  gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(grid));

  open_file_select_button = gtk_button_new();
  gtk_button_set_label(GTK_BUTTON(open_file_select_button), "Open file");
  g_signal_connect(GTK_BUTTON(open_file_select_button), "clicked",
                   G_CALLBACK(open_file_picker), NULL);

  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(open_file_select_button), 0, 0, 1,
                  1);

  save_btn = GTK_BUTTON(gtk_button_new());
  gtk_button_set_label(GTK_BUTTON(save_btn), "Save data");
  g_signal_connect(GTK_BUTTON(save_btn), "clicked",
                   G_CALLBACK(save_translations_to_file), NULL);

  gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(save_btn), 0, 1, 1, 1);
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
