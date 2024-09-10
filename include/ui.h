#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>
#include <gst/gst.h>

extern GstElement *pipeline;

GtkWidget* create_icon_bar();
GtkWidget* create_icon_button(const char *icon_path);
void on_play_button_clicked(GtkWidget *widget, gpointer data);
void on_pause_button_clicked(GtkWidget *widget, gpointer data);
void on_skip_button_clicked(GtkWidget *widget, gpointer data);
void on_prev_button_clicked(GtkWidget *widget, gpointer data);
void on_playlist_button_clicked(GtkWidget *widget, gpointer window);
void on_music_button_clicked(GtkWidget *widget, gpointer window);
void on_folder_button_clicked(GtkWidget *widget, gpointer window);
void on_activate(GtkApplication *app, gpointer user_data);
void apply_css(GtkWidget *widget);
GtkWidget* create_player_box();
gboolean update_progress_bar(gpointer data);
void refresh_music_list();
void play_music_by_index(gint index);

#endif
