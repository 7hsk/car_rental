#include <gtk/gtk.h>
#include <string.h>
#include <glib.h>
#include "signup.h"
#include "user.h"
#include "utils.h"
#include "login.h"
#include <errno.h>

extern GtkWidget *stack;
extern GList *window_stack;

gboolean is_unique_user_data(const char *phonenumber, const char *cin, const char *email) {
    GDir *dir = g_dir_open("C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase", 0, NULL);
    if (dir == NULL) {
        g_print("Error opening directory!\n");
        return FALSE;
    }

    const gchar *filename;
    User user;
    while ((filename = g_dir_read_name(dir)) != NULL) {
        if (g_str_has_suffix(filename, ".bin")) {
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase\\%s", filename);

            FILE *file = fopen(file_path, "rb");
            if (file == NULL) {
                g_print("Error opening file at %s: %s\n", file_path, strerror(errno));
                continue;
            }

            fread(&user, sizeof(User), 1, file);
            fclose(file);

            if (strcmp(user.phonenumber, phonenumber) == 0 || strcmp(user.cin, cin) == 0 || strcmp(user.email, email) == 0) {
                g_dir_close(dir);
                return FALSE;
            }
        }
    }

    g_dir_close(dir);
    return TRUE;
}

void open_signup_window(GtkWidget *widget, gpointer data) {
    static int signup_view_counter = 0;
    char signup_view_name[32];
    snprintf(signup_view_name, sizeof(signup_view_name), "signup_view_%d", signup_view_counter++);

    // Create a full-screen window
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    GtkWidget **entries = g_new(GtkWidget *, 9);

    entries[0] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entries[0]), "Name");
    gtk_box_append(GTK_BOX(box), entries[0]);

    entries[1] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entries[1]), "Last Name");
    gtk_box_append(GTK_BOX(box), entries[1]);

    entries[2] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entries[2]), "Phone Number");
    gtk_box_append(GTK_BOX(box), entries[2]);

    entries[3] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entries[3]), "Email");
    gtk_box_append(GTK_BOX(box), entries[3]);

    entries[4] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entries[4]), "CIN (ID)");
    gtk_box_append(GTK_BOX(box), entries[4]);

    GtkWidget *birthday_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(birthday_box, GTK_ALIGN_CENTER);

    entries[5] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entries[5]), "DD");
    gtk_entry_set_max_length(GTK_ENTRY(entries[5]), 2);
    gtk_box_append(GTK_BOX(birthday_box), entries[5]);

    entries[6] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entries[6]), "MM");
    gtk_entry_set_max_length(GTK_ENTRY(entries[6]), 2);
    gtk_box_append(GTK_BOX(birthday_box), entries[6]);

    entries[7] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entries[7]), "YYYY");
    gtk_entry_set_max_length(GTK_ENTRY(entries[7]), 4);
    gtk_box_append(GTK_BOX(birthday_box), entries[7]);

    gtk_box_append(GTK_BOX(box), birthday_box);

    entries[8] = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entries[8]), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(entries[8]), FALSE);
    gtk_box_append(GTK_BOX(box), entries[8]);

    // Buttons
    GtkWidget *signup_button = gtk_button_new_with_label("Sign Up");
    gtk_widget_set_halign(signup_button, GTK_ALIGN_CENTER);
    g_signal_connect(signup_button, "clicked", G_CALLBACK(on_signup_button_clicked), entries);
    gtk_box_append(GTK_BOX(box), signup_button);

    GtkWidget *return_button = gtk_button_new_with_label("Return Back");
    gtk_widget_set_halign(return_button, GTK_ALIGN_CENTER);
    g_signal_connect(return_button, "clicked", G_CALLBACK(return_to_main_menu), NULL);
    gtk_box_append(GTK_BOX(box), return_button);

    window_stack = g_list_append(window_stack, box);
    gtk_stack_add_named(GTK_STACK(stack), box, signup_view_name);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), signup_view_name);
}


void show_general_error(GtkWidget *parent, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new_with_markup(
        GTK_WINDOW(parent),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_CLOSE,
        "<span foreground='red'><b>Error:</b> %s</span>",
        message
    );
    gtk_window_set_title(GTK_WINDOW(dialog), "SIGN UP ERROR");

    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_window_present(GTK_WINDOW(dialog));
}

void on_signup_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **entries = (GtkWidget **)data;
    User info;
    GtkWidget *parent = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);

    for (int i = 0; i < 9; i++) {
        if (!GTK_IS_ENTRY(entries[i])) {
            g_print("Entry %d is not a valid GtkEditable!\n", i);
            return;
        }
    }

    strncpy(info.name, gtk_editable_get_text(GTK_EDITABLE(entries[0])), sizeof(info.name) - 1);
    info.name[sizeof(info.name) - 1] = '\0';

    strncpy(info.lastname, gtk_editable_get_text(GTK_EDITABLE(entries[1])), sizeof(info.lastname) - 1);
    info.lastname[sizeof(info.lastname) - 1] = '\0';

    strncpy(info.phonenumber, gtk_editable_get_text(GTK_EDITABLE(entries[2])), sizeof(info.phonenumber) - 1);
    info.phonenumber[sizeof(info.phonenumber) - 1] = '\0';

    strncpy(info.email, gtk_editable_get_text(GTK_EDITABLE(entries[3])), sizeof(info.email) - 1);
    info.email[sizeof(info.email) - 1] = '\0';

    strncpy(info.cin, gtk_editable_get_text(GTK_EDITABLE(entries[4])), sizeof(info.cin) - 1);
    info.cin[sizeof(info.cin) - 1] = '\0';

    info.day = atoi(gtk_editable_get_text(GTK_EDITABLE(entries[5])));
    info.month = atoi(gtk_editable_get_text(GTK_EDITABLE(entries[6])));
    info.year = atoi(gtk_editable_get_text(GTK_EDITABLE(entries[7])));

    char *hashed_password = hash_password(gtk_editable_get_text(GTK_EDITABLE(entries[8])));
    strncpy(info.password, hashed_password, sizeof(info.password) - 1);
    info.password[sizeof(info.password) - 1] = '\0';
    g_free(hashed_password);

    strncpy(info.role, "Client", sizeof(info.role) - 1);
    info.role[sizeof(info.role) - 1] = '\0';

    // Add phone number validation
    if (!is_valid_moroccan_phone(info.phonenumber)) {
        show_general_error(GTK_WIDGET(parent), "Invalid phone number! Must start with 06 or 07 and contain exactly 10 digits.");
        return;
    }

    // Add birthday validation
    if (info.year < 1912) {
        show_general_error(GTK_WIDGET(parent), "Invalid year! Year must be 1912 or later.");
        return;
    }
    if (info.month < 1 || info.month > 12) {
        show_general_error(GTK_WIDGET(parent), "Invalid month! Month must be between 1 and 12.");
        return;
    }
    int max_days = get_days_in_month(info.month, info.year);
    if (info.day < 1 || info.day > max_days) {
        show_general_error(GTK_WIDGET(parent), "Invalid day for the selected month and year.");
        return;
    }

    if (!is_valid_name(info.name) || !is_valid_name(info.lastname)) {
        show_general_error(GTK_WIDGET(parent), "Name and Last Name must be between 3 and 20 characters and cannot contain numbers.");
        return;
    }

    if (!is_valid_email(info.email)) {
        show_general_error(GTK_WIDGET(parent), "Invalid email format. Ensure it contains '@' and a domain.");
        return;
    }

    if (!is_strong_password(gtk_editable_get_text(GTK_EDITABLE(entries[8])))) {
        show_general_error(GTK_WIDGET(parent), "Password must be at least 8 characters long and include uppercase, lowercase, a digit, and a special character.");
        return;
    }

    if (!is_older_than_21(info.year, info.month, info.day)) {
        show_general_error(GTK_WIDGET(parent), "You must be older than 21 years to sign up.");
        return;
    }

    if (!is_unique_user_data(info.phonenumber, info.cin, info.email)) {
        show_general_error(GTK_WIDGET(parent), "Phone number, CIN, or email has already been used.");
        return;
    }

    save_signup_data(&info);
    g_print("Signup successful!\n");

    g_free(entries);

    open_login_window(widget, NULL);
}

void save_signup_data(const User *info) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase\\%s.bin", info->cin);

    FILE *file = fopen(file_path, "wb");
    if (file) {
        fwrite(info, sizeof(User), 1, file);
        fclose(file);
    } else {
        g_print("Error opening file at %s: %s\n", file_path, strerror(errno));
    }
}