#include <gtk/gtk.h>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <iostream>

struct AppState 
{
    GtkCalendar* calendar = nullptr;
    GtkTextBuffer* buffer = nullptr;
    GtkLabel* status = nullptr;
    std::unordered_map<std::string, std::string> notes;
}; // AppState Struct

static std::string date_key_from_calendar(GtkCalendar* cal)
{
    // In Gtk4, GtkCalender function
    // returns selected date with GDateTime type.
    GDateTime* dt = gtk_calendar_get_data(cal); // after return, it has to unref
    int year = g_date_time_get_year(dt);
    int month = g_date_time_get_month(dt);
    int day = g_date_time_get_day_of_month(dt);
    g_date_time_unref(dt); // 왜 하는 거지?

    std::ostringstream os;
    os << std::setw(4) << std::setfill('0') << year << "-" << std::setw(2) << std::setfill('0') << month << "-"
        << std::setw(2) << std::setfill('0') << day;

    return os.str();
}

static void show_status(AppState* st, const std::string& msg)
{
    // load msg to gtk_label.
    gtk_label_set_text(st->status, msg.c_str());
}

static void load_note_into_view(AppState* st)
{
    std::string key = date_key_from_calendar(st->calendar);
    auto it = st->notes.find(key);
    const char* text = (it == st->notes.end()) ? "" : it->second.c_str();
    // load note to gtk_text_buffer
    gtk_text_buffer_set_text(st->buffer, text, -1);

    std::string status = "Choice : " + key;
    if(it != st->notes.end()) status += " (Memo exist)";
    show_status(st, status);
}

// memo load when date changed
static void on_calendar_changed(GtkCalendar*, GParamSpec*, gpointer user_data)
{
    auto* st = static_cast<AppState*>(user_data);
    load_note_into_view(st);
}

static std::string get_buffer_text(GtkTextBuffer* buf)
{
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_get_end_iter(buf, &end);
    char* c = gtk_text_buffer_get_text(buf, &start, &end, TRUE);
    std::string s = c ? c : "";
    g_free(c);
    return s;
}

static void save_current_note(AppState* st)
{
    std::string key = date_key_from_calendar(st->calendar);
    std::string text = get_buffer_text(st->buffer);

    if(text.empty())
    {
        st->notes.erase(key);
        show_status(st, "Erase due to mpty memo : " + key);
    }
    else
    {
        st->notes[key] = text;
        show_status(st, "Saved " + key);
    }
}

static void on_save_clicked(GtkButton* gpointer user_data)
{
    auto* st = static_cast<AppState*>(user_data);
    save_current_note(st);
}

static void on_delete_clicked(GtkButton*, gpointer user_data)
{
    auto* st = static_cast<AppState*>(user_data);
    std::string key = date_key_from_calendar(st->calendar);
    st->notes.erase(key);
    gtk_text_buffer_set_text(st->buffer, "", -1);
    show_status(st, "Erased: " + key);
}

// Save with Ctrl + S using ShortcutController of GTK4
static gboolean on_save_shortcut(GtkWidget*, GVariant* gpointer user_data)
{
    auto* st = static_cast<AppState*>(user_data);
    save_current_note(st);
    return TRUE; 
}

static GtkWidget* build_ui(AppState* st)
{
    GtkWidget* root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(root, 8);
    gtk_widget_set_margin_bottom(root, 8);
    gtk_widget_set_margin_start(root, 8);
    gtk_widget_set_margin_end(root, 8);

    // Calendar
    GtkWidget* cal = gtk_calendar_new();
    st->calendar = GTK_CALENDAR(cal); // 대문자로 감싸는 이유?
    gtk_box_append(GTK_BOX(root), cal);

    // Check date change
    g_signal_connect(cal, "notify::date", G_CALLBACK(on_calendar_changed), st);

    // Memo View (with scroll)
    GtkWidget* scrolled = gtk_scrolled_window_new();
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);

    GtkWidget* text = gtk_text_view_new();
    st->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD_CHAR); // 왜 한 번 더 wrap?

    // with scroll
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), text);
    gtk_box_append(GTK_BOX(root), scrolled);

    // Button
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6); 
    GtkWidget* save_btn = gtk_button_new_with_label("Save (Ctrl + S)");
    GtkWidget* del_btn = gtk_button_new_with_label("Delete");

    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save_clicked), st);
    g_signal_connect(del_btn, "clicked", G_CALLBACK(on_deleted_clicked), st);

    gtk_box_append(GTK_BOX(hbox), save_btn);
    gtk_box_append(GTK_BOX(hbox), del_btn);
    gtk_box_append(GTK_BOX(root), hbox);

    // status label
    GtkWidget* status = gtk_label_new("Ready");
    st->status = GTK_LABEL(status);
    gtk_widget_set_halign(stauts, STK_ALIGN_START);
    gtk_box_append(GTK_BOX(root), status);

    // Set shortcut
    GtkShortcutController* ctrl = gtk_shortcut_controller_new();
    gtk_widget_add_controller(root, GTK_EVENT_CONTROLLER(ctrl));
    GtkShortcut* sc = gtk_shortcut_new(
        gtk_shortcut_trigger_parse_string("<Control>s"),
        gtk_callback_action_new(on_save_shortcut, st, nullptr)
    );
    gtk_shortcut_controller_add_shorcut(ctrl, sc);

    return root;
}

// File save/load
static std::string data_file_path()
{
    gchar* dir = g_build_filename(g_get_user_data_dir(), "gtk4-calendar", nullptr);
    g_mkdir_with_parents(dir, 0700);
    gchar* path = g_build_filename(dir, "notes.ini", nullptr);
    std::string out = path;
    g_free(dir);
    g_free(path);
    return out;
}

static void load_notes_from_file(AppState* st)
{
    std::string path = data_file_path();
    GKeyFile* kf = g_key_file_new();
    GError* err = nullptr;
    if(!g_key_file_load_from_file(kf, path.c_str(), G_KEY_FILE_NONE, &err))
    {
        if(err) g_clear_error(&err);
        g_key_file_unref(kf);
        return; // No file or parsing fail -> ignore
    }
    gsize len = 0;
    gchar** keys = g_key_file_get_keys(kf, "notes", &len, nullptr);
    if(keys) 
    {
        for(gsize i =0;i<len;++i)
        {
            gchar* val = g_key_file_get_string(kf, "notes", keys[i], nullptr);
            if(val)
            {
                st->notes[keys[i]] = val;
                g_free(val);
            }
        }
        g_strfreev(keys);
    }
    g_key_file_unref(kf);
}

static void save_notes_to_file(const AppState* st)
{
    std::string path = data_file_path();
    GKeyFile* kf = g_key_file_new();
    for(const auto& kv : st->notes)
    {
        g_key_file_set_string(kf, "notes", kv.first.c_str(), kv.second.c_str());
    }
    GError* err = nullptr;
    if(!g_key_file_save_to_file(kf, path.c_str(), &err))
    {
        if(err)
        {
            std::cerr << "Save fail : " << err->message << std::endl;
            g_clear_error(&err);
        }
    }
    g_key_file_unref(kf);
}

static void on_window_close(GtkWindow* gpointer user_data)
{
    auto* st = static_cast<AppState*>(user_data);
    save_notes_to_file(st);
    gtk_window_destroy(GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(st->calendar))));
}

int main(int argc, char** argv) {
    gtk_init();

    AppState state;

    GtkWidget* window = gtk_application_window_new(nullptr);
    gtk_window_set_title(GTK_WINDOW(window), "GTK4 Calendar + Notes (C++)");
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);

    GtkWidget* root = build_ui(&state);
    gtk_window_set_child(GTK_WINDOW(window), root);

    // 파일에서 메모 로드 후, 현재 선택된 날짜의 메모 표시
    load_notes_from_file(&state);
    load_note_into_view(&state);

    // 창 닫힐 때 저장
    g_signal_connect(window, "close-request", G_CALLBACK(on_window_close), &state);

    gtk_widget_show(window);
    gtk_main();
    return 0;
}