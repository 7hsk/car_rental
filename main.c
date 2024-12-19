#include <gtk/gtk.h>
#include "utils.h"
#include "fewrfre.h"
#include "car.h"

GList *window_stack = NULL;

GtkWidget *stack;

static void activate (GtkApplication *app, gpointer user_data) {
        show_main_menu(app);
}



int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    populate_catalogue_with_brands_and_models();
    GList *brands = g_list_first(catalogue);
    while (brands != NULL) {
        MarqueCatalogue *marque = (MarqueCatalogue *)brands->data;
        g_print("Brand: %s, Logo: %s\n", marque->marque, marque->logo_path);

        // Iterate through the models for this brand and print model information
        GList *models = g_list_first(marque->models);
        while (models != NULL) {
            Car *car = (Car *)models->data;
            g_print("  Model: %s, Price: %.2f, Available: %s\n", car->model, car->price_per_day, car->available ? "Yes" : "No");
            models = g_list_next(models);
        }

        brands = g_list_next(brands);
    }

    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    int num_users;
    User *users = read_all_user_data(&num_users);
    if (users != NULL) {
        for (int i = 0; i < num_users; i++) {
            g_print("User %d: %s %s %s %s %s %d/%d/%d %s %s\n", i + 1, users[i].name, users[i].lastname, users[i].phonenumber, users[i].email, users[i].cin, users[i].day, users[i].month, users[i].year, users[i].password, users[i].role);
        }
        g_free(users);
    }

    return status;
}