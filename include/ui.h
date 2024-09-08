#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>

void on_play_button_clicked(GtkWidget *widget, gpointer data);
void on_pause_button_clicked(GtkWidget *widget, gpointer data);
void on_stop_button_clicked(GtkWidget *widget, gpointer data);
void on_activate(GtkApplication *app, gpointer user_data);

#endif
