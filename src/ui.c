#include <gtk/gtk.h>
#include "ui.h"


void on_play_button_clicked(GtkWidget *widget, gpointer data) {
    g_print("Lecture démarrée\n");
}


void on_pause_button_clicked(GtkWidget *widget, gpointer data) {
    g_print("Lecture en pause\n");
}


void on_stop_button_clicked(GtkWidget *widget, gpointer data) {
    g_print("Lecture arrêtée\n");
}


void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *play_button, *pause_button, *stop_button;


    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "MediaFlow");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);


    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);


    play_button = gtk_button_new_with_label("Lecture");
    pause_button = gtk_button_new_with_label("Pause");
    stop_button = gtk_button_new_with_label("Arrêt");


    g_signal_connect(play_button, "clicked", G_CALLBACK(on_play_button_clicked), NULL);
    g_signal_connect(pause_button, "clicked", G_CALLBACK(on_pause_button_clicked), NULL);
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop_button_clicked), NULL);


    gtk_grid_attach(GTK_GRID(grid), play_button, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), pause_button, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), stop_button, 2, 0, 1, 1);


    gtk_widget_show_all(window);
}
