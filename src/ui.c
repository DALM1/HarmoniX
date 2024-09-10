#include "ui.h"
#include "playlist.h"
#include <string.h>

GstElement *pipeline;
GtkWidget *listbox;
GtkWidget *play_button, *pause_button, *skip_button, *prev_button;
GtkWidget *volume_scale;
GtkWidget *progress_bar;
gint current_track = -1;
GPtrArray *music_files = NULL;  // Déclaration globale pour les fichiers

// Déclaration de la fonction on_playlist_selected avant son utilisation
void on_playlist_selected(GtkWidget *widget, gpointer user_data);  // Ajout de la déclaration

void apply_css(GtkWidget *widget) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background-color: rgba(243, 242, 242, 0); }"
        ".icon-button { background-color: rgba(34, 34, 34, 0.5); border-radius: 50%; padding: 10px; }"
        ".icon-button:hover { background-color: rgba(51, 51, 51, 0.7); }"
        "button { background-color: rgba(34, 34, 34, 0.5); color: white; font-size: 12px; }"
        "button:hover { background-color: rgba(34, 34, 34, 0.5); }"
        "label { color: white; font-weight: bold; font-size: 14px; }"
        "listbox { background-color: rgba(243, 242, 242, 0); color: white; }"
        "scale { color: white; }"
        "scale trough { background-color: rgba(85, 85, 85, 0.5); }"
        "scale slider { background-color: grey; }",
        -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}

const gchar *get_filename_from_path(const gchar *filepath) {
    const gchar *filename = g_strrstr(filepath, "/");
    return filename ? filename + 1 : filepath;
}

const gchar *remove_extension(const gchar *filename) {
    gchar *dot = g_strrstr(filename, ".");
    if (dot) {
        return g_strndup(filename, dot - filename);
    }
    return g_strdup(filename);
}

void on_volume_changed(GtkRange *range, gpointer data) {
    gdouble volume = gtk_range_get_value(range);
    g_object_set(pipeline, "volume", volume, NULL);
}

gboolean update_progress_bar(gpointer data) {
    gint64 position = 0, duration = 0;

    if (gst_element_query_position(pipeline, GST_FORMAT_TIME, &position) &&
        gst_element_query_duration(pipeline, GST_FORMAT_TIME, &duration)) {

        gdouble progress = (gdouble)position / (gdouble)duration;

        if (progress > 0.99) {
            progress = 1.0;
        }

        gtk_range_set_value(GTK_RANGE(progress_bar), progress);

        if (progress >= 1.0) {
            on_about_to_finish(pipeline, NULL);
        }
    }

    return TRUE;
}

void play_music_by_index(gint index) {
    if (index >= 0 && index < music_files->len) {
        const char *file_path = g_ptr_array_index(music_files, index);
        gchar *uri = g_strdup_printf("file://%s", file_path);
        g_print("Lecture du fichier %s\n", uri);

        gst_element_set_state(pipeline, GST_STATE_NULL);
        g_object_set(pipeline, "uri", uri, NULL);

        GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);

        if (ret == GST_STATE_CHANGE_FAILURE) {
            g_printerr("Erreur: Impossible de démarrer la lecture.\n");
        } else {
            g_print("La musique est en cours de lecture.\n");
        }

        current_track = index;
        g_timeout_add(500, update_progress_bar, NULL);
        g_free(uri);
    }
}

void refresh_music_list() {
    gtk_list_box_invalidate_filter(GTK_LIST_BOX(listbox));
    gtk_container_foreach(GTK_CONTAINER(listbox), (GtkCallback)gtk_widget_destroy, NULL);

    for (guint i = 0; i < music_files->len; i++) {
        const char *filepath = g_ptr_array_index(music_files, i);
        const char *filename = remove_extension(get_filename_from_path(filepath));
        GtkWidget *row = gtk_button_new_with_label(filename);

        g_signal_connect(row, "clicked", G_CALLBACK(on_track_button_clicked), GINT_TO_POINTER(i));

        gtk_list_box_insert(GTK_LIST_BOX(listbox), row, -1);
        g_free((gchar *)filename);
    }
    gtk_widget_show_all(listbox);
}

void on_track_button_clicked(GtkWidget *widget, gpointer user_data) {
    gint track_index = GPOINTER_TO_INT(user_data);
    play_music_by_index(track_index);
}

void on_about_to_finish(GstElement *pipeline, gpointer user_data) {
    gint next_track = current_track + 1;
    if (next_track < music_files->len) {
        play_music_by_index(next_track);
    } else {
        g_print("Fin de la liste de lecture.\n");
        gst_element_set_state(pipeline, GST_STATE_READY);  // Changer à READY pour éviter les plantages
        current_track = -1;  // Réinitialiser l'index du morceau
    }
}

void on_play_button_clicked(GtkWidget *widget, gpointer data) {
    if (current_track == -1 && music_files->len > 0) {
        play_music_by_index(0);
    } else {
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
    }
}

void on_pause_button_clicked(GtkWidget *widget, gpointer data) {
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
}

void on_skip_button_clicked(GtkWidget *widget, gpointer data) {
    on_about_to_finish(pipeline, NULL);
}

void on_prev_button_clicked(GtkWidget *widget, gpointer data) {
    gint prev_track = current_track - 1;

    if (prev_track >= 0) {
        play_music_by_index(prev_track);
    } else {
        prev_track = music_files->len - 1;
        play_music_by_index(prev_track);
    }
}

GtkWidget* create_icon_button(const char *icon_path) {
    GtkWidget *button, *image;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(icon_path, 16, 16, NULL);
    image = gtk_image_new_from_pixbuf(pixbuf);
    button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(button), image);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_style_context_add_class(gtk_widget_get_style_context(button), "icon-button");
    return button;
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
    g_signal_connect(skip_button, "clicked", G_CALLBACK(on_skip_button_clicked), NULL);
    g_signal_connect(prev_button, "clicked", G_CALLBACK(on_prev_button_clicked), NULL);

    player_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(player_box), prev_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(player_box), play_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(player_box), pause_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(player_box), skip_button, TRUE, TRUE, 5);

    volume_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.01);
    gtk_range_set_value(GTK_RANGE(volume_scale), 0.5);
    g_signal_connect(volume_scale, "value-changed", G_CALLBACK(on_volume_changed), NULL);
    gtk_box_pack_start(GTK_BOX(player_box), volume_scale, TRUE, TRUE, 5);

    progress_bar = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.01);
    gtk_box_pack_start(GTK_BOX(player_box), progress_bar, TRUE, TRUE, 5);

    return player_box;
}

// Gestion du Drag & Drop
void on_drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, gpointer user_data) {
    gchar **uris = gtk_selection_data_get_uris(data);

    for (gint i = 0; uris[i] != NULL; i++) {
        gchar *file_path = g_filename_from_uri(uris[i], NULL, NULL);
        if (file_path) {
            add_file_to_playlist(file_path, current_playlist);  // Ajouter le fichier à la playlist
            g_free(file_path);
        }
    }

    gtk_drag_finish(context, TRUE, FALSE, time);
}

void on_folder_button_clicked(GtkWidget *widget, gpointer window) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;
    dialog = gtk_file_chooser_dialog_new("Ajouter un fichier audio ou vidéo", GTK_WINDOW(window), action, "_Annuler", GTK_RESPONSE_CANCEL, "_Ouvrir", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    res = gtk_dialog_run(GTK_DIALOG(dialog));

    if (res == GTK_RESPONSE_ACCEPT) {
        GSList *filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
        for (GSList *iter = filenames; iter != NULL; iter = iter->next) {
            add_file_to_playlist((const char *)iter->data, current_playlist);  // Ajouter les fichiers à la playlist sélectionnée
            g_free(iter->data);
        }
        g_slist_free(filenames);
    }

    gtk_widget_destroy(dialog);
    refresh_playlist_view(listbox);  // Rafraîchir l'affichage de la playlist
}

void on_music_button_clicked(GtkWidget *widget, gpointer window) {
    refresh_music_list();
}

void on_playlist_button_clicked(GtkWidget *widget, gpointer window) {
    g_print("Affichage des playlists...\n");

    gtk_container_foreach(GTK_CONTAINER(listbox), (GtkCallback)gtk_widget_destroy, NULL);

    if (playlists && playlists->len > 0) {
        for (guint i = 0; i < playlists->len; i++) {
            Playlist *playlist = g_ptr_array_index(playlists, i);
            GtkWidget *row = gtk_button_new_with_label(playlist->name);

            g_signal_connect(row, "clicked", G_CALLBACK(on_playlist_selected), GINT_TO_POINTER(i));

            gtk_list_box_insert(GTK_LIST_BOX(listbox), row, -1);
        }
    } else {
        g_print("Aucune playlist disponible.\n");
    }

    gtk_widget_show_all(listbox);
}

// Définition de la fonction on_playlist_selected
void on_playlist_selected(GtkWidget *widget, gpointer user_data) {
    gint playlist_index = GPOINTER_TO_INT(user_data);
    load_playlist(playlist_index);

    // Ajouter des fichiers à la playlist sélectionnée
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Ajouter des fichiers à la playlist",
        NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Annuler", GTK_RESPONSE_CANCEL,
        "_Ajouter", GTK_RESPONSE_ACCEPT,
        NULL);

    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        GSList *files = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
        for (GSList *iter = files; iter != NULL; iter = iter->next) {
            add_file_to_playlist((char *)iter->data, playlist_index);
            g_free(iter->data);
        }
        g_slist_free(files);
    }

    gtk_widget_destroy(dialog);
}

void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *main_vbox, *icon_bar, *player_box, *scrolled_window;

    gst_init(NULL, NULL);
    pipeline = gst_element_factory_make("playbin", "player");
    g_signal_connect(pipeline, "about-to-finish", G_CALLBACK(on_about_to_finish), NULL);

    music_files = g_ptr_array_new_with_free_func(g_free);

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "HarmoniX");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 600);

    main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 20);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    icon_bar = create_icon_bar();
    gtk_box_pack_start(GTK_BOX(main_vbox), icon_bar, FALSE, FALSE, 0);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    listbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), listbox);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrolled_window, TRUE, TRUE, 10);

    player_box = create_player_box();
    gtk_widget_set_halign(player_box, GTK_ALIGN_CENTER);
    gtk_box_pack_end(GTK_BOX(main_vbox), player_box, FALSE, FALSE, 0);

    gtk_widget_show_all(window);

    // Support du Drag & Drop
    GtkTargetEntry targets[] = {
        {"text/uri-list", 0, 0},
    };
    gtk_drag_dest_set(window, GTK_DEST_DEFAULT_ALL, targets, G_N_ELEMENTS(targets), GDK_ACTION_COPY);
    g_signal_connect(window, "drag-data-received", G_CALLBACK(on_drag_data_received), NULL);

    playlists = g_ptr_array_new_with_free_func(g_free);
    add_playlist("Default Playlist");
    load_playlist(0);
}
