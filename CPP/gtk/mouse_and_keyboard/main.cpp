#include <gtk/gtk.h>
#include <cmath>

static void draw(GtkDrawingArea*, cairo_t* cr, int w, int h, gpointer)
{
    // a dark gray background
    cairo_set_source_rgb(cr, 0.12, 0.13, 0.16);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_fill(cr);
}

static void activate(GtkApplication* app, gpointer)
{
    GtkWidget* win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), "Step1 - Hello GTK4");
    gtk_window_set_default_size(GTK_WINDOW(win), 800, 600);

    GtkWidget* area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(area, true);
    gtk_widget_set_vexpand(area, true);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(area), draw, nullptr, nullptr);

    gtk_window_set_child(GTK_WINDOW(win), area);
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