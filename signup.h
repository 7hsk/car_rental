#ifndef SIGNUP_H
#define SIGNUP_H

#include <gtk/gtk.h>
#include "user.h"

void open_signup_window(GtkWidget *widget, gpointer data);
void show_general_error(GtkWidget *parent, const char *message);
void on_error_close_clicked(GtkWidget *widget, gpointer data);
void on_signup_button_clicked(GtkWidget *widget, gpointer data);
void save_signup_data(const User *info);
gboolean is_unique_user_data(const char *phonenumber, const char *cin, const char *email);

#endif // SIGNUP_H