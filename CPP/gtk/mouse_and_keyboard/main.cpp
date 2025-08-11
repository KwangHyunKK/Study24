#include <gtk/gtk.h>
#include <cmath>
#include <vector>

struct Circle
{
    double x, y;
    double r;
};

struct AppState{
    GtkWidget* area = nullptr;
    std::vector<Circle> circles;
};

static void draw(GtkDrawingArea*, cairo_t* cr, int w, int h, gpointer user_data)
{

    auto* S = static_cast<AppState*>(user_data);

    // a dark gray background
    cairo_set_source_rgb(cr, 0.12, 0.13, 0.16);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_fill(cr);

    // Grid
    cairo_set_source_rgba(cr, 1,1,1,0.06);
    cairo_set_line_width(cr, 1.0);
    const int step = 32;
    for(int x = 0;x<w;x+= step)
    {
        cairo_move_to(cr, x + 0.5, 0);
        cairo_line_to(cr, x + 0.5, h);
    }
    for(int y=0;y<h;y+=step)
    {
        cairo_move_to(cr, 0, y + 0.5);
        cairo_line_to(cr, w, y + 0.5);
    }
    cairo_stroke(cr);

    for(const auto& c : S->circles)
    {
        // fill circle
        cairo_set_source_rgba(cr, 0.35, 0.72, 0.98, 0.75);
        cairo_arc(cr, c.x, c.y, c.r, 0, 2*M_PI);
        cairo_fill_preserve(cr);

        // circle line
        cairo_set_source_rgba(cr, 1, 1, 1, 0.9);
        cairo_set_line_width(cr, 2.0);
        cairo_stroke(cr);
    }
}

// click -> add circle
static void on_click(GtkGestureClick* gesture, gint n_press, gdouble x, gdouble y, gpointer user_data)
{
    (void)gesture; (void)n_press;
    auto* S = static_cast<AppState*>(user_data);

    // fixed r
    S->circles.push_back(Circle{x, y, 28.0});
    gtk_widget_queue_draw(S->area); // call in manual scheduling.
}

static void activate(GtkApplication* app, gpointer)
{
    auto* S = new AppState();

    GtkWidget* win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), "Step3 - Click to add Circles");
    gtk_window_set_default_size(GTK_WINDOW(win), 800, 600);

    S->area = gtk_drawing_area_new(); // make canvas widget
    g_return_if_fail(S->area != nullptr); // if nullptr -> log and exit
    gtk_widget_set_hexpand(S->area, TRUE);
    gtk_widget_set_vexpand(S->area, TRUE);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(S->area), draw, S, nullptr); // function when used in draw situation

    // Click Gesture
    GtkEventController* click = GTK_EVENT_CONTROLLER(gtk_gesture_click_new());
    g_signal_connect(click, "pressed", G_CALLBACK(on_click), S);
    gtk_widget_add_controller(S->area, click); // add click controller in widget.

    gtk_window_set_child(GTK_WINDOW(win), S->area);
    gtk_window_present(GTK_WINDOW(win));
}

int main(int argc, char** argv)
{
    GtkApplication* app = gtk_application_new("dev.step.gtk4.s3", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), nullptr);
    int code = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return code;
}