#ifndef ADMIN_H
#define ADMIN_H



#include <gtk/gtk.h>
void load_brand_buttons(GtkWidget *container);
void confirm_delete_brand(GtkWidget *button, gpointer user_data);
void on_confirm_add_brand(GtkWidget *button, gpointer user_data);
void on_brand_button_clicked(GtkWidget *button, gpointer user_data);
void on_add_car_clicked(GtkWidget *widget, gpointer data);
void on_confirm_add_car_with_file(GtkWidget *button, gpointer user_data);
void on_voitures_clicked(GtkWidget *widget, gpointer data);
void load_cars_page(GtkWidget *parent_container);
void confirm_delete_car(GtkWidget *button, gpointer user_data);
void on_edit_price_response(GtkWidget *button, gpointer user_data);
void display_car_page(GtkWidget *parent_container);
void on_add_car_clicked(GtkWidget *widget, gpointer data);
void on_delete_car_clicked(GtkWidget *widget, gpointer data);
void on_edit_price_clicked(GtkWidget *widget, gpointer data);
GtkWidget* create_admin_button(const char *label, GCallback callback, gpointer data);
void on_previous_page(GtkWidget *widget, gpointer data);
void on_next_page(GtkWidget *widget, gpointer data);
void confirm_delete_user(GtkWidget *dialog, gint response_id, gpointer user_data);
static void on_delete_user_clicked(GtkWidget *widget, gpointer user_data);
void open_admin_window(GtkWidget *widget, gpointer data);
void on_les_comptes_clicked(GtkWidget *widget, gpointer data);
void load_user_page(GtkWidget *parent_container, int page);
void create_moderator_account(GtkWidget *widget, gpointer data);
void open_create_mod_window(GtkWidget *widget, gpointer data);
void on_close_dialog_clicked(GtkDialog *dialog, gint response_id, gpointer user_data);
void show_success_message(GtkWidget *parent, const char *message);
#endif // ADMIN_H
