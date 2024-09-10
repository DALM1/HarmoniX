#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <gtk/gtk.h>
#include <gst/gst.h>

typedef struct {
    gchar *name;
    GPtrArray *tracks;
} Playlist;

extern GPtrArray *playlists;
extern gint current_playlist;

void add_playlist(const char *playlist_name);
void load_playlist(gint playlist_index);
void add_file_to_playlist(const char *filename, gint playlist_index);
void refresh_playlist_view(GtkWidget *listbox);

#endif
