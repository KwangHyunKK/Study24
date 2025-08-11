#include <gtk/gtk.h>

static void activate(GtkApplication* app, gpointer)
{
    GtkWidget* win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), "Step1 - Hello GTK4");
    gtk_window_set_default_size(GTK_WINDOW(win), 800, 600);
    gtk_window_present(GTK_WINDOW(win));
}

int main(int argc, char** argv)
{
    GtkApplication* app = gtk_application_new("dev.step.gtk4.s1", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), nullptr);
    int code = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return code;
}