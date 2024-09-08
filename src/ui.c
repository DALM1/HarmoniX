#include <gtk/gtk.h>
#include <gst/gst.h>
#include "ui.h"

GstElement *pipeline;
GtkWidget *listbox;
GtkWidget *play_button, *pause_button, *skip_button, *prev_button;
GPtrArray *music_files;


void apply_css(GtkWidget *widget) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background-image: url('media/defaultgif.gif'); background-size: cover; }"
        ".icon-button { background-color: white; border-radius: 50%; padding: 10px; }"
        ".icon-button:hover { background-color: #f0f0f0; }"
        "button image { min-width: 22px; min-height: 22px; }"
        "label { color: white; font-weight: bold; font-size: 14px; }"
        "listbox, scrolledwindow, button { background-color: #333; color: white; }"
        , -1, NULL);

    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}


const gchar *get_filename_from_path(const gchar *filepath) {
    const gchar *filename = g_strrstr(filepath, "/");
    return filename ? filename + 1 : filepath;
}


GtkWidget* create_icon_button(const char *icon_path) {
    GtkWidget *button, *image;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(icon_path, 22, 22, NULL);
    image = gtk_image_new_from_pixbuf(pixbuf);

    button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(button), image);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_style_context_add_class(gtk_widget_get_style_context(button), "icon-button");
    return button;
}


void play_music(GtkWidget *widget, gpointer file_path) {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        g_object_set(pipeline, "uri", g_strdup_printf("file://%s", (char*)file_path), NULL);
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
        g_print("Lecture du fichier : %s\n", (char*)file_path);
    }
}


void refresh_music_list() {
    gtk_list_box_invalidate_filter(GTK_LIST_BOX(listbox));
    for (guint i = 0; i < music_files->len; i++) {
        const char *filepath = g_ptr_array_index(music_files, i);
        const char *filename = get_filename_from_path(filepath);
        GtkWidget *row = gtk_button_new_with_label(filename);
        g_signal_connect(row, "clicked", G_CALLBACK(play_music), g_strdup(filepath));
        gtk_list_box_insert(GTK_LIST_BOX(listbox), row, -1);
    }
    gtk_widget_show_all(listbox);
}


void add_file_to_music_list(const char *filename) {
    g_ptr_array_add(music_files, g_strdup(filename));
    refresh_music_list();
}


void on_play_button_clicked(GtkWidget *widget, gpointer data) {
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void on_pause_button_clicked(GtkWidget *widget, gpointer data) {
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
}

void on_stop_button_clicked(GtkWidget *widget, gpointer data) {
    gst_element_set_state(pipeline, GST_STATE_NULL);
}


void on_music_button_clicked(GtkWidget *widget, gpointer window) {
    GtkWidget *music_view, *scrolled_window, *player_box;


    music_view = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(music_view), "Liste des musiques");
    gtk_window_set_default_size(GTK_WINDOW(music_view), 600, 400);

    apply_css(music_view);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    listbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), listbox);

    refresh_music_list();


    prev_button = create_icon_button("media/skipp2.png");
    play_button = create_icon_button("media/jouer.png");
    pause_button = create_icon_button("media/pause.png");
    skip_button = create_icon_button("media/skipp.png");

    g_signal_connect(play_button, "clicked", G_CALLBACK(on_play_button_clicked), NULL);
    g_signal_connect(pause_button, "clicked", G_CALLBACK(on_pause_button_clicked), NULL);
    g_signal_connect(skip_button, "clicked", G_CALLBACK(on_stop_button_clicked), NULL);


    player_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(player_box), prev_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(player_box), play_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(player_box), pause_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(player_box), skip_button, TRUE, TRUE, 5);


    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), player_box, FALSE, FALSE, 10);

    gtk_container_add(GTK_CONTAINER(music_view), vbox);
    gtk_widget_show_all(music_view);
}


void on_folder_button_clicked(GtkWidget *widget, gpointer window) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Ajouter un fichier audio ou vidéo",
                                         GTK_WINDOW(window),
                                         action,
                                         "_Annuler", GTK_RESPONSE_CANCEL,
                                         "_Ouvrir", GTK_RESPONSE_ACCEPT,
                                         NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        g_print("Fichier ajouté : %s\n", filename);
        add_file_to_music_list(filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}


void on_playlist_button_clicked(GtkWidget *widget, gpointer window) {
    g_print("Gestion des playlists à implémenter\n");
}


GtkWidget* create_horizontal_icon_bar() {
    GtkWidget *bar, *music_button, *playlist_button, *folder_button;
    GtkWidget *music_label, *playlist_label, *folder_label;
    GtkWidget *icon_container, *label_container, *button_box;


    bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 50);


    icon_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    music_button = create_icon_button("media/musique-notes.png");
    music_label = gtk_label_new("Musique");
    gtk_box_pack_start(GTK_BOX(icon_container), music_button, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(icon_container), music_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bar), icon_container, FALSE, FALSE, 20);
    g_signal_connect(music_button, "clicked", G_CALLBACK(on_music_button_clicked), NULL);


    playlist_button = create_icon_button("media/playlist.png");
    playlist_label = gtk_label_new("Playlists");
    label_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(label_container), playlist_button, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(label_container), playlist_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bar), label_container, FALSE, FALSE, 20);
    g_signal_connect(playlist_button, "clicked", G_CALLBACK(on_playlist_button_clicked), NULL);


    folder_button = create_icon_button("media/dossier.png");
    folder_label = gtk_label_new("Ajouter des fichiers");
    button_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(button_box), folder_button, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box), folder_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bar), button_box, FALSE, FALSE, 20);
    g_signal_connect(folder_button, "clicked", G_CALLBACK(on_folder_button_clicked), NULL);

    return bar;
}


void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *grid, *icon_bar;


    gst_init(NULL, NULL);
    pipeline = gst_element_factory_make("playbin", "player");


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
