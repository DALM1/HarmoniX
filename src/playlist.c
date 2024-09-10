#include "playlist.h"
#include "ui.h"

GPtrArray *playlists = NULL;
gint current_playlist = -1;

void add_playlist(const char *playlist_name) {
    Playlist *new_playlist = g_new(Playlist, 1);
    new_playlist->name = g_strdup(playlist_name);
    new_playlist->tracks = g_ptr_array_new_with_free_func(g_free);
    g_ptr_array_add(playlists, new_playlist);
}

void load_playlist(gint playlist_index) {
    if (playlist_index >= 0 && playlist_index < playlists->len) {
        Playlist *playlist = g_ptr_array_index(playlists, playlist_index);
        music_files = playlist->tracks;
        current_playlist = playlist_index;
        refresh_music_list();
    }
}

void add_file_to_playlist(const char *filename, gint playlist_index) {
    if (playlist_index >= 0 && playlist_index < playlists->len) {
        Playlist *playlist = g_ptr_array_index(playlists, playlist_index);

        for (guint i = 0; i < playlist->tracks->len; i++) {
            if (g_strcmp0(filename, g_ptr_array_index(playlist->tracks, i)) == 0) {
                g_print("Le fichier %s est déjà dans la playlist.\n", filename);
                return;
            }
        }

        g_ptr_array_add(playlist->tracks, g_strdup(filename));
    }
}

void refresh_playlist_view(GtkWidget *listbox) {
    gtk_list_box_invalidate_filter(GTK_LIST_BOX(listbox));
    gtk_container_foreach(GTK_CONTAINER(listbox), (GtkCallback)gtk_widget_destroy, NULL);

    if (current_playlist >= 0 && current_playlist < playlists->len) {
        Playlist *playlist = g_ptr_array_index(playlists, current_playlist);
        for (guint i = 0; i < playlist->tracks->len; i++) {
            const char *filepath = g_ptr_array_index(playlist->tracks, i);
            const char *filename = remove_extension(get_filename_from_path(filepath));
            GtkWidget *row = gtk_button_new_with_label(filename);

            g_signal_connect(row, "clicked", G_CALLBACK(on_track_button_clicked), GINT_TO_POINTER(i));

            gtk_list_box_insert(GTK_LIST_BOX(listbox), row, -1);
            g_free((gchar *)filename);
        }
    }

    gtk_widget_show_all(listbox);
}
