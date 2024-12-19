#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>

extern const char *month_names[];
void show_info_message(GtkWidget *parent, const char *message);
const char* get_month_name(int month);
int is_leap_year(int year);
int get_month_from_name(const char *month_name);
int get_days_in_month(int month, int year);
void on_quit_clicked(GtkWidget *widget, gpointer data);
gboolean is_valid_name(const char *name);
gboolean is_valid_email(const char *email);
gboolean is_strong_password(const char *password);
gboolean is_older_than_21(int year, int month, int day);
gboolean is_valid_moroccan_phone(const char *phone);
void return_back(GtkWidget *widget, gpointer data);
void return_to_main_menu(GtkWidget *widget, gpointer data);
void proceed_to_next_window(const char *role);
void show_error_message(GtkWidget *parent, const char *message);
char* hash_password(const char *password);
void show_main_menu(GtkApplication *app);

#endif // UTILS_H