#include <gtk/gtk.h>
#include "ui.h"


void apply_css(GtkWidget *widget) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background-image: url('media/defaultgif.gif'); background-size: cover; }"
        ".icon-button { background-color: #f0f0f0; border-radius: 50%; padding: 20px; }"
        "button image { min-width: 48px; min-height: 48px; }"
        "label { color: white; font-weight: bold; font-size: 14px; }",
        -1, NULL);

    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}


GtkWidget* create_icon_button(const char *icon_path) {
    GtkWidget *button, *image;
    image = gtk_image_new_from_file(icon_path);
    button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(button), image);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_style_context_add_class(gtk_widget_get_style_context(button), "icon-button");
    return button;
}


GtkWidget* create_horizontal_icon_bar() {
    GtkWidget *bar, *music_button, *playlist_button, *folder_button;
    GtkWidget *music_label, *playlist_label, *folder_label;
    GtkWidget *icon_container, *label_container, *button_box;


    bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 50);


    icon_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    music_button = create_icon_button("media/musique.png");
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


void play_music(const char *file_path) {
    g_print("Lecture du fichier : %s\n", file_path);
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
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}


void on_music_button_clicked(GtkWidget *widget, gpointer window) {
    GtkWidget *music_view, *scrolled_window, *listbox;
    GDir *dir;
    const gchar *filename;
    gchar *file_path;


    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    listbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), listbox);


    dir = g_dir_open("media", 0, NULL);
    while ((filename = g_dir_read_name(dir)) != NULL) {
        if (g_str_has_suffix(filename, ".mp3")) {
            GtkWidget *row = gtk_button_new_with_label(filename);
            file_path = g_strdup_printf("media/%s", filename);
            g_signal_connect(row, "clicked", G_CALLBACK(play_music), file_path);
            gtk_list_box_insert(GTK_LIST_BOX(listbox), row, -1);
        }
    }
    g_dir_close(dir);


    music_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(music_view), scrolled_window, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(window), music_view);
    gtk_widget_show_all(GTK_WIDGET(window));
}


void on_playlist_button_clicked(GtkWidget *widget, gpointer window) {
    g_print("Gestion des playlists à implémenter\n");
}

void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *grid, *icon_bar;


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
