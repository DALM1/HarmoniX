#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/audio/audio.h>
#include <string.h>
#include "ui.h"

GstElement *pipeline;
GtkWidget *listbox;
GtkWidget *play_button, *pause_button, *skip_button, *prev_button;
GtkWidget *volume_scale;
GtkWidget *background_image;
GPtrArray *music_files;
gint current_track = -1;

void apply_css(GtkWidget *widget) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background-color: rgba(0, 0, 0, 0); }"
        ".icon-button { background-color: rgba(34, 34, 34, 0.5); border-radius: 50%; padding: 10px; }"
        ".icon-button:hover { background-color: rgba(51, 51, 51, 0.7); }"
        "button { background-color: rgba(34, 34, 34, 0.5); color: white; font-size: 12px; }"
        "button:hover { background-color: rgba(51, 51, 51, 0.7); }"
        "label { color: white; font-weight: bold; font-size: 14px; }"
        "listbox { background-color: rgba(34, 34, 34, 0.5); color: white; }"
        "listbox row:nth-child(even) { background-color: rgba(51, 51, 51, 0.5); }"
        "listbox row:nth-child(odd) { background-color: rgba(34, 34, 34, 0.5); }"
        "scale { color: white; }"
        "scale trough { background-color: rgba(85, 85, 85, 0.5); }"
        "scale slider { background-color: white; }",  // Curseur de volume blanc
        -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}

const gchar *get_filename_from_path(const gchar *filepath) {
    const gchar *filename = g_strrstr(filepath, "/");
    return filename ? filename + 1 : filepath;
}

// Retirer l'extension du nom du fichier pour un affichage plus propre
const gchar *remove_extension(const gchar *filename) {
    gchar *dot = g_strrstr(filename, ".");
    if (dot) {
        return g_strndup(filename, dot - filename); // Retourne le nom sans l'extension
    }
    return g_strdup(filename); // Si pas d'extension, retourne le nom complet
}

GtkWidget* create_icon_button(const char *icon_path) {
    GtkWidget *button, *image;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(icon_path, 16, 16, NULL);  // Réduire la taille des icônes
    image = gtk_image_new_from_pixbuf(pixbuf);
    button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(button), image);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_style_context_add_class(gtk_widget_get_style_context(button), "icon-button");
    return button;
}

void on_volume_changed(GtkRange *range, gpointer data) {
    gdouble volume = gtk_range_get_value(range);
    g_object_set(pipeline, "volume", volume, NULL);
}

void play_music_by_index(gint index) {
    if (index >= 0 && index < music_files->len) {
        const char *file_path = g_ptr_array_index(music_files, index);
        gst_element_set_state(pipeline, GST_STATE_NULL);
        g_object_set(pipeline, "uri", g_strdup_printf("file://%s", file_path), NULL);
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
        current_track = index;
    }
}

void play_music(GtkWidget *widget, gpointer file_path) {
    gint index = g_ptr_array_find_with_equal_func(music_files, file_path, (GEqualFunc)g_str_equal, NULL);
    play_music_by_index(index);
}

void refresh_music_list() {
    gtk_list_box_invalidate_filter(GTK_LIST_BOX(listbox));
    gtk_list_box_invalidate_headers(GTK_LIST_BOX(listbox));  // Mise à jour pour les doublons
    gtk_container_foreach(GTK_CONTAINER(listbox), (GtkCallback)gtk_widget_destroy, NULL);  // Supprimer les anciens éléments

    for (guint i = 0; i < music_files->len; i++) {
        const char *filepath = g_ptr_array_index(music_files, i);
        const char *filename = remove_extension(get_filename_from_path(filepath));  // Retirer l'extension
        GtkWidget *row = gtk_button_new_with_label(filename);
        g_signal_connect(row, "clicked", G_CALLBACK(play_music), g_strdup(filepath));
        gtk_list_box_insert(GTK_LIST_BOX(listbox), row, -1);
        g_free((gchar *)filename);  // Libérer la mémoire allouée pour le nom du fichier sans extension
    }
    gtk_widget_show_all(listbox);
}

void add_file_to_music_list(const char *filename) {
    // Vérifie si le fichier est déjà dans la liste avant de l'ajouter
    for (guint i = 0; i < music_files->len; i++) {
        if (g_strcmp0(filename, g_ptr_array_index(music_files, i)) == 0) {
            g_print("Le fichier %s est déjà dans la liste.\n", filename);
            return;
        }
    }
    g_ptr_array_add(music_files, g_strdup(filename));
    refresh_music_list();
}

void on_about_to_finish(GstElement *pipeline, gpointer user_data) {
    gint next_track = current_track + 1;
    if (next_track < music_files->len) {
        play_music_by_index(next_track);
    } else {
        g_print("Fin de la liste de lecture.\n");
    }
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

void on_skip_button_clicked(GtkWidget *widget, gpointer data) {
    on_about_to_finish(pipeline, NULL);
}

void on_prev_button_clicked(GtkWidget *widget, gpointer data) {
    gint prev_track = current_track - 1;
    if (prev_track >= 0) {
        play_music_by_index(prev_track);
    }
}

GtkWidget* create_icon_bar() {
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

GtkWidget* create_player_box() {
    GtkWidget *player_box;
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

    volume_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.01);
    gtk_range_set_value(GTK_RANGE(volume_scale), 0.5);
    g_signal_connect(volume_scale, "value-changed", G_CALLBACK(on_volume_changed), NULL);
    gtk_box_pack_start(GTK_BOX(player_box), volume_scale, TRUE, TRUE, 5);

    return player_box;
}

void on_music_button_clicked(GtkWidget *widget, gpointer window) {
    refresh_music_list();
}

void on_folder_button_clicked(GtkWidget *widget, gpointer window) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;
    dialog = gtk_file_chooser_dialog_new("Ajouter un fichier audio ou vidéo", GTK_WINDOW(window), action, "_Annuler", GTK_RESPONSE_CANCEL, "_Ouvrir", GTK_RESPONSE_ACCEPT, NULL);
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        add_file_to_music_list(filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void on_playlist_button_clicked(GtkWidget *widget, gpointer window) {
    g_print("Gestion des playlists à implémenter\n");
}

// Fonction pour charger et afficher le GIF en arrière-plan
GtkWidget* create_background_image(const char* gif_path) {
    GdkPixbufAnimation *animation = gdk_pixbuf_animation_new_from_file(gif_path, NULL);
    return gtk_image_new_from_animation(animation);
}

void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *main_vbox, *icon_bar, *player_box, *scrolled_window;
    GtkWidget *overlay;

    gst_init(NULL, NULL);
    pipeline = gst_element_factory_make("playbin", "player");
    g_signal_connect(pipeline, "about-to-finish", G_CALLBACK(on_about_to_finish), NULL);

    music_files = g_ptr_array_new_with_free_func(g_free);

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "HarmoniX");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 600);

    // Utiliser GtkOverlay pour superposer les widgets
    overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(window), overlay);

    // Charger et ajouter le GIF en arrière-plan
    background_image = create_background_image("media/defaultgif.gif");
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), background_image);
    gtk_overlay_set_overlay_pass_through(GTK_OVERLAY(overlay), background_image, TRUE);  // Permettre les interactions sur les widgets au-dessus

    // Créer une boîte verticale contenant le reste de l'interface
    main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 20);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), main_vbox);  // Ajouter main_vbox par-dessus l'arrière-plan GIF

    icon_bar = create_icon_bar();
    gtk_box_pack_start(GTK_BOX(main_vbox), icon_bar, FALSE, FALSE, 0);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    listbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), listbox);
    gtk_widget_set_margin_start(scrolled_window, 50);
    gtk_widget_set_margin_end(scrolled_window, 50);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrolled_window, TRUE, TRUE, 10);

    player_box = create_player_box();
    gtk_widget_set_halign(player_box, GTK_ALIGN_CENTER);
    gtk_box_pack_end(GTK_BOX(main_vbox), player_box, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
}
