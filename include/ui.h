#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>

void on_activate(GtkApplication *app, gpointer user_data);
void on_music_button_clicked(GtkWidget *widget, gpointer window);
void on_playlist_button_clicked(GtkWidget *widget, gpointer window);
void on_folder_button_clicked(GtkWidget *widget, gpointer window);

#endif
