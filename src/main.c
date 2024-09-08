#include <gtk/gtk.h>
#include "ui.h"

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // Initialiser GTK
    app = gtk_application_new("com.harmonix.app", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    // Exécuter l'application
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
