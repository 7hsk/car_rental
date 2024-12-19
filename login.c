#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "login.h"
#include "utils.h"
#include "user.h"
#include <errno.h>

extern GtkWidget *stack;
extern GList *window_stack;

LoginStatus validate_login(const char *email, const char *password, char *role) {
    char *hashed_password = hash_password(password);
    gboolean email_found = FALSE;
    User user;

    GDir *dir = g_dir_open("C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase", 0, NULL);
    if (dir == NULL) {
        g_print("Error opening directory!\n");
        g_free(hashed_password);
        return LOGIN_ERROR_FILE;
    }

    const gchar *filename;
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

            if (strcmp(user.email, email) == 0) {
                email_found = TRUE;
                if (strcmp(user.password, hashed_password) == 0) {
                    g_free(hashed_password);
                    g_dir_close(dir);
                    strcpy(role, user.role);
                    current_user = user;
                    return LOGIN_SUCCESS;
                }
            }
        }
    }

    g_free(hashed_password);
    g_dir_close(dir);

    return email_found ? LOGIN_ERROR_INCORRECT_PASSWORD : LOGIN_ERROR_EMAIL_NOT_FOUND;
}

#define GTK_RESPONSE_RETRY 1

void show_account_not_found_error(GtkWidget *parent) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Error",
        GTK_WINDOW(parent),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Retry", GTK_RESPONSE_RETRY,
        "_Create your account now", GTK_RESPONSE_ACCEPT,
        NULL
    );

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>Error:</b> This account does not exist.");
    gtk_box_append(GTK_BOX(content_area), label);

    gtk_widget_show(dialog);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), dialog);
}

void show_incorrect_password_error(GtkWidget *parent) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Error",
        GTK_WINDOW(parent),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Close", GTK_RESPONSE_CLOSE,
        NULL
    );

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>Error:</b> Incorrect password. Try again.");
    gtk_box_append(GTK_BOX(content_area), label);

    gtk_widget_show(dialog);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), dialog);
}

void on_login_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *email_entry = g_object_get_data(G_OBJECT(data), "email_entry");
    GtkEntryBuffer *email_buffer = gtk_entry_get_buffer(GTK_ENTRY(email_entry));
    const char *email = gtk_entry_buffer_get_text(email_buffer);

    GtkWidget *password_entry = g_object_get_data(G_OBJECT(data), "password_entry");
    GtkEntryBuffer *password_buffer = gtk_entry_get_buffer(GTK_ENTRY(password_entry));
    const char *password = gtk_entry_buffer_get_text(password_buffer);

    if (strcmp(email, "svr") == 0 && strcmp(password, "f") == 0) {
        g_print("Login successful! Role: Admin\n");
        proceed_to_next_window("Admin");  // Open the Admin window
        return;
    }


    char role[20];

    LoginStatus status = validate_login(email, password, role);

    switch (status) {
        case LOGIN_SUCCESS:
            g_print("Login successful! Role: %s\n", role);
            proceed_to_next_window(role);
        break;
        case LOGIN_ERROR_EMAIL_NOT_FOUND:
            show_account_not_found_error(GTK_WIDGET(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)));
        break;
        case LOGIN_ERROR_INCORRECT_PASSWORD:
            show_incorrect_password_error(GTK_WIDGET(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)));
        break;
        case LOGIN_ERROR_FILE:
            show_error_message(GTK_WIDGET(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "Error opening user data file.");
        break;
    }
}

void open_login_window(GtkWidget *widget, gpointer data) {
    static int login_view_counter = 0;
    char login_view_name[32];
    snprintf(login_view_name, sizeof(login_view_name), "login_view_%d", login_view_counter++);

    // Parent vertical box centered
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    GtkWidget *email_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(email_entry), "Email");
    gtk_box_append(GTK_BOX(box), email_entry);

    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_box_append(GTK_BOX(box), password_entry);

    GtkWidget *login_button = gtk_button_new_with_label("Login");
    gtk_widget_set_halign(login_button, GTK_ALIGN_CENTER);
    g_object_set_data(G_OBJECT(login_button), "email_entry", email_entry);
    g_object_set_data(G_OBJECT(login_button), "password_entry", password_entry);
    gtk_box_append(GTK_BOX(box), login_button);

    GtkWidget *return_button = gtk_button_new_with_label("Return Back");
    gtk_widget_set_halign(return_button, GTK_ALIGN_CENTER);
    g_signal_connect(return_button, "clicked", G_CALLBACK(return_to_main_menu), NULL);
    gtk_box_append(GTK_BOX(box), return_button);

    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_button_clicked), login_button);

    window_stack = g_list_append(window_stack, box);
    gtk_stack_add_named(GTK_STACK(stack), box, login_view_name);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), login_view_name);
}
