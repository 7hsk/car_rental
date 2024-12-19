#include <gtk/gtk.h>
#include "car.h"
#include <glib.h>


// Define catalogue here
GList *catalogue = NULL;  // Initialize catalogue as NULL

void add_car_to_catalogue(const char *marque, const char *model, int available, float price_per_day, const char *image_path) {
    GList *iterator = catalogue;
    MarqueCatalogue *existing_marque = NULL;

    // Check if the brand exists
    while (iterator != NULL) {
        MarqueCatalogue *marque_item = (MarqueCatalogue *)iterator->data;
        if (strcmp(marque_item->marque, marque) == 0) {
            existing_marque = marque_item;
            break;
        }
        iterator = iterator->next;
    }

    // If the brand doesn't exist, create a new one
    if (!existing_marque) {
        existing_marque = (MarqueCatalogue *)malloc(sizeof(MarqueCatalogue));
        strcpy(existing_marque->marque, marque);
        existing_marque->logo_path[0] = '\0';  // Initialize logo path
        existing_marque->models = NULL;  // Initialize models list
        catalogue = g_list_append(catalogue, existing_marque);
    }

    // Create a new car model
    Car *new_car = (Car *)malloc(sizeof(Car));
    strcpy(new_car->model, model);
    strcpy(new_car->marque, marque);
    new_car->available = available;
    new_car->price_per_day = price_per_day;
    strcpy(new_car->image_path, image_path);

    // Add the car to the models list of the existing brand
    existing_marque->models = g_list_append(existing_marque->models, new_car);
}

// Function to populate the catalogue with brands and models
void populate_catalogue_with_brands_and_models(void) {
    const char *brands_dir = "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase_cars\\DataBase_brands";
    const char *models_dir = "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase_cars\\DataBase_models";

    // Read brands from the DataBase_brands directory
    GDir *brands_dir_ptr = g_dir_open(brands_dir, 0, NULL);
    if (brands_dir_ptr == NULL) {
        g_print("Error opening brands directory: %s\n", brands_dir);
        return;
    }

    const gchar *filename;
    while ((filename = g_dir_read_name(brands_dir_ptr)) != NULL) {
        if (g_str_has_suffix(filename, ".bin")) {  // Process .bin files only
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "%s\\%s", brands_dir, filename);

            // Open the binary file and read the brand data
            FILE *file = fopen(file_path, "rb");
            if (file == NULL) {
                g_print("Error opening file: %s\n", file_path);
                continue;
            }

            // Read the brand data from the file
            BrandData brand_data;
            fread(&brand_data, sizeof(BrandData), 1, file);
            fclose(file);

            // Add the brand to the catalogue list
            MarqueCatalogue *new_marque = (MarqueCatalogue *)malloc(sizeof(MarqueCatalogue));
            strcpy(new_marque->marque, brand_data.brand_name);
            strcpy(new_marque->logo_path, brand_data.logo_path);
            new_marque->models = NULL;  // Initialize the models list for the brand

            // Add the brand to the catalogue list
            catalogue = g_list_append(catalogue, new_marque);

            g_print("Added brand: %s with logo: %s\n", brand_data.brand_name, brand_data.logo_path);
        }
    }

    g_dir_close(brands_dir_ptr);  // Close the brands directory

    // Read models from the DataBase_models directory and assign them to the correct brand
    GDir *models_dir_ptr = g_dir_open(models_dir, 0, NULL);
    if (models_dir_ptr == NULL) {
        g_print("Error opening models directory: %s\n", models_dir);
        return;
    }

    while ((filename = g_dir_read_name(models_dir_ptr)) != NULL) {
        if (g_str_has_suffix(filename, ".bin")) {  // Process .bin files only
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "%s\\%s", models_dir, filename);

            // Open the binary file and read the model data
            FILE *file = fopen(file_path, "rb");
            if (file == NULL) {
                g_print("Error opening file: %s\n", file_path);
                continue;
            }

            // Read the model data from the file
            Car car_data;
            fread(&car_data, sizeof(Car), 1, file);
            fclose(file);

            // Find the corresponding brand in the catalogue
            GList *iterator = catalogue;
            while (iterator != NULL) {
                MarqueCatalogue *marque_item = (MarqueCatalogue *)iterator->data;
                if (strcmp(marque_item->marque, car_data.marque) == 0) {
                    // Add the model to the brand's model list
                    Car *new_car = (Car *)malloc(sizeof(Car));
                    memcpy(new_car, &car_data, sizeof(Car));  // Copy model data into the new car
                    marque_item->models = g_list_append(marque_item->models, new_car);
                    break;
                }
                iterator = iterator->next;
            }
        }
    }

    g_dir_close(models_dir_ptr);  // Close the models directory
}


GtkWidget* create_item_button(const char *name, const char *logo_path, gboolean is_brand) {
    // Create the button
    GtkWidget *button = gtk_button_new();

    // Create a horizontal box to hold the name and logo
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);  // 10px spacing
    gtk_button_set_child(GTK_BUTTON(button), box);  // Set the box as the button's child

    // Create a label for the name (either brand name or model name)
    GtkWidget *label;
    if (is_brand) {
        label = gtk_label_new(name);  // For brand, show the brand name
    } else {
        label = gtk_label_new(name);  // For model, show the model name
    }
    gtk_box_append(GTK_BOX(box), label);  // Add the label to the box (left side)

    // Create an image for the logo (same for both models and brands)
    GtkWidget *image = gtk_image_new_from_file(logo_path);
    gtk_box_append(GTK_BOX(box), image);  // Add the image to the box (right side)

    // Set the image to be at the right side of the button by aligning it to the right
    gtk_widget_set_halign(image, GTK_ALIGN_END);

    return button;
}
