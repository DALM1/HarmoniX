#include <gtk/gtk.h>
#include <gst/gst.h>
#include "ui.h"

GstElement *pipeline;
GtkWidget *listbox;
GPtrArray *music_files;
gint current_track = -1; // Pour savoir quelle piste est en cours de lecture

void apply_css(GtkWidget *widget);

void apply_css(GtkWidget *widget) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background-image: url('media/defaultgif.gif'); background-size: cover; }"
        ".icon-button { background-color: white; border-radius: 50%; padding: 20px; }"
        ".icon-button:hover { background-color: #f0f0f0; }"
        "button image { min-width: 64px; min-height: 64px; }"
        "label { color: white; font-weight: bold; font-size: 14px; }",
        -1, NULL);

    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}

GtkWidget* create_icon_button(const char *icon_path) {
    GtkWidget *button, *image;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(icon_path, 64, 64, NULL);

    pixbuf = gdk_pixbuf_add_alpha(pixbuf, TRUE, 0xFF, 0xFF, 0xFF);
    image = gtk_image_new_from_pixbuf(pixbuf);

    button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(button), image);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_style_context_add_class(gtk_widget_get_style_context(button), "icon-button");

    return button;
}

void play_music_by_index(gint index) {
    if (index >= 0 && index < music_files->len) {
        const char *file_path = g_ptr_array_index(music_files, index);
        gchar *uri = g_strdup_printf("file://%s", file_path);
        g_print("Lecture du fichier : %s\n", uri);

        gst_element_set_state(pipeline, GST_STATE_NULL); // Stop le pipeline en cours
        g_object_set(pipeline, "uri", uri, NULL); // Définit la nouvelle URI
        gst_element_set_state(pipeline, GST_STATE_PLAYING); // Démarre la lecture

        current_track = index; // Met à jour la piste en cours
        g_free(uri);
    }
}

// Appelé lorsque la piste se termine pour jouer la suivante
void on_about_to_finish(GstElement *pipeline, gpointer user_data) {
    gint next_track = current_track + 1;
    if (next_track < music_files->len) {
        play_music_by_index(next_track);
    } else {
        g_print("Fin de la liste de lecture.\n");
    }
}

void play_music(GtkWidget *widget, gpointer file_path) {
    gint index = g_ptr_array_find_with_equal_func(music_files, file_path, (GEqualFunc)g_str_equal, NULL);
    play_music_by_index(index);
}

void refresh_music_list() {
    gtk_list_box_invalidate_filter(GTK_LIST_BOX(listbox));
    gtk_container_foreach(GTK_CONTAINER(listbox), (GtkCallback)gtk_widget_destroy, NULL);

    for (guint i = 0; i < music_files->len; i++) {
        const char *filepath = g_ptr_array_index(music_files, i);
        GtkWidget *row = gtk_button_new_with_label(filepath);
        g_signal_connect(row, "clicked", G_CALLBACK(play_music), g_strdup(filepath));
        gtk_list_box_insert(GTK_LIST_BOX(listbox), row, -1);
    }
    gtk_widget_show_all(listbox);
}

void add_file_to_music_list(const char *filename) {
    for (guint i = 0; i < music_files->len; i++) {
        if (g_strcmp0(filename, g_ptr_array_index(music_files, i)) == 0) {
            g_print("Le fichier %s est déjà dans la liste.\n", filename);
            return;
        }
    }
    g_ptr_array_add(music_files, g_strdup(filename));
    refresh_music_list();
}

void on_music_button_clicked(GtkWidget *widget, gpointer window) {
    GtkWidget *music_view, *scrolled_window;

    music_view = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(music_view), "Liste des musiques");
    gtk_window_set_default_size(GTK_WINDOW(music_view), 600, 400);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    listbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), listbox);

    refresh_music_list();

    gtk_container_add(GTK_CONTAINER(music_view), scrolled_window);
    gtk_widget_show_all(music_view);
}

GtkWidget* create_horizontal_icon_bar() {
    GtkWidget *bar, *music_button, *playlist_button, *folder_button;
    GtkWidget *icon_container;

    bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 50);

    icon_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    music_button = create_icon_button("media/musique.png");
    gtk_box_pack_start(GTK_BOX(icon_container), music_button, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(bar), icon_container, FALSE, FALSE, 20);
    g_signal_connect(music_button, "clicked", G_CALLBACK(on_music_button_clicked), NULL);

    return bar;
}

void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *grid, *icon_bar;

    gst_init(NULL, NULL);
    pipeline = gst_element_factory_make("playbin", "player");
    g_signal_connect(pipeline, "about-to-finish", G_CALLBACK(on_about_to_finish), NULL);

    music_files = g_ptr_array_new_with_free_func(g_free);

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "HarmoniX");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 600);

    apply_css(window);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    icon_bar = create_horizontal_icon_bar();
    gtk_grid_attach(GTK_GRID(grid), icon_bar, 0, 0, 1, 1);

    gtk_widget_show_all(window);
}
