#ifndef LOGIN_H
#define LOGIN_H

#include <gtk/gtk.h>

typedef enum {
    LOGIN_SUCCESS,
    LOGIN_ERROR_FILE,
    LOGIN_ERROR_EMAIL_NOT_FOUND,
    LOGIN_ERROR_INCORRECT_PASSWORD
} LoginStatus;

LoginStatus validate_login(const char *email, const char *password, char *role);
void show_account_not_found_error(GtkWidget *parent);
void show_incorrect_password_error(GtkWidget *parent);
void open_login_window(GtkWidget *widget, gpointer data);
void on_login_button_clicked(GtkWidget *widget, gpointer data);

#endif // LOGIN_H