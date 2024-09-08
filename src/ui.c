#include <gtk/gtk.h>
#include <gst/gst.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include "ui.h"


GstElement *pipeline;
char **mp3_files = NULL;
int num_files = 0;
int current_file_index = 0;


void load_mp3_files(const char *directory) {
    DIR *dir;
    struct dirent *entry;
    char filepath[512];

    if ((dir = opendir(directory)) != NULL) {

        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name, ".mp3") != NULL) {
                snprintf(filepath, sizeof(filepath), "file://%s/%s", directory, entry->d_name);


                mp3_files = realloc(mp3_files, sizeof(char*) * (num_files + 1));
                mp3_files[num_files] = strdup(filepath);
                num_files++;
            }
        }
        closedir(dir);
    } else {
        g_printerr("Impossible d'ouvrir le répertoire : %s\n", directory);
    }
}


void play_current_file() {
    if (num_files > 0 && pipeline) {
        g_object_set(pipeline, "uri", mp3_files[current_file_index], NULL);
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
        g_print("Lecture du fichier : %s\n", mp3_files[current_file_index]);
    }
}


void on_play_button_clicked(GtkWidget *widget, gpointer data) {
    if (pipeline) {
        play_current_file();
    }
}


void on_pause_button_clicked(GtkWidget *widget, gpointer data) {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_PAUSED);
        g_print("Lecture en pause\n");
    }
}


void on_stop_button_clicked(GtkWidget *widget, gpointer data) {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        g_print("Lecture arrêtée\n");
    }
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


    pipeline = gst_element_factory_make("playbin", "player");
    if (!pipeline) {
        g_printerr("Le pipeline n'a pas pu être créé.\n");
        return;
    }


    load_mp3_files("media");
}
