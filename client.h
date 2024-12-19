#ifndef CLIENT_H
#define CLIENT_H

#include <gtk/gtk.h>
#include "car.h"


void display_user_requests(GtkWidget *parent_grid, const char *directory_path, const char *user_cin);
void on_cancel_request_clicked(GtkWidget *widget, gpointer user_data);
void on_etat_demande_button_clicked(GtkWidget *widget, gpointer user_data);
void close_popup(GtkWidget *widget, gpointer user_data);
void close_popup_and_return(GtkWidget *widget, gpointer user_data);
void show_success_popup(GtkWidget *parent_window);
void on_confirm_request(GtkWidget *widget, gpointer data);
void open_request_window(GtkWidget *widget, gpointer user_data);
void open_model_dispo_details_window(GtkWidget *widget, gpointer user_data);
void open_car_dispo_window(GtkApplication *app, gpointer user_data);
void open_edit_popup(GtkWidget *widget, gpointer data);
void open_profile_window(GtkWidget *widget, gpointer user_data);
void open_model_details_window(GtkWidget *widget, gpointer user_data);
void open_models_window(GtkWidget *widget, gpointer user_data);
void open_catalogue_window(GtkApplication *app, gpointer user_data);
static void on_button_clicked(GtkWidget *widget, gpointer data);
void open_client_window(GtkApplication *app, gpointer data);
void show_brand_models(GtkWidget *catalogue_grid, MarqueCatalogue *marque);
#endif // CLIENT_H