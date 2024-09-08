#include <gtk/gtk.h>
#include <gst/gst.h>
#include "ui.h"

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;


    gst_init(&argc, &argv);


    app = gtk_application_new("com.example.MediaFlow", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);


    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
