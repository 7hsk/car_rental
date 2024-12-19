#include <gtk/gtk.h>
#include "admin.h"
#include "utils.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "user.h"
#include "car.h"
#include <ctype.h>

#define USER_DATABASE_PATH "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase"
#define CAR_DATABASE_PATH "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase_cars\\DataBase_models"
#define BRAND_DATABASE_PATH "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase_cars\\DataBase_brands"
#define CAR_IMAGES_DIR "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase_cars\\"

GList *brand_buttons = NULL;  // Holds all brand buttons
GtkWidget *brand_grid = NULL; // Holds the grid replacing the selected button

extern GtkWidget *stack;

#define USERS_PER_PAGE 20
static int total_users = 0;

static char selected_brand_name[50] = "";
static char selected_brand_logo_path[512] = "";

gboolean does_file_exist(const char *directory_path, const char *name, const char *extension) {
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s\\%s%s", directory_path, name, extension);
    return access(file_path, F_OK) == 0;
}

// Function to check if a string contains any digits
gboolean contains_digits(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (isdigit((unsigned char)str[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

// Function to display error dialogs (GTK 4 version)
void show_error_dialog(const char *message) {
    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    // Content area for the dialog
    GtkWidget *content_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(dialog), content_area);

    // Error message label
    GtkWidget *label = gtk_label_new(message);
    gtk_box_append(GTK_BOX(content_area), label);

    // Button for closing the dialog
    GtkWidget *close_button = gtk_button_new_with_label("Close");
    gtk_box_append(GTK_BOX(content_area), close_button);

    // Connect the button to destroy the dialog
    g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_window_destroy), dialog);

    // Present the dialog
    gtk_window_present(GTK_WINDOW(dialog));
}

void on_delete_brand_clicked(GtkWidget *widget, gpointer user_data) {
    const char *file_path = (const char *)user_data;

    // Read brand name for display purposes
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        g_print("Failed to open file: %s\n", file_path);
        return;
    }

    BrandData brand;
    fread(&brand, sizeof(BrandData), 1, file);
    fclose(file);

    // Create the confirmation dialog
    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Delete Brand");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    // Set transient parent for proper pop-up behavior
    GtkWidget *parent_window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
    if (GTK_IS_WINDOW(parent_window)) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent_window));
    }

    // Add content area
    GtkWidget *content_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(dialog), content_area);

    // Add confirmation text
    GtkWidget *label = gtk_label_new("Are you sure you want to delete this brand?");
    gtk_box_append(GTK_BOX(content_area), label);

    // Add "Confirm" and "Cancel" buttons to the content area
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(content_area), button_box);

    // Add Confirm button
    GtkWidget *confirm_button = gtk_button_new_with_label("Confirm");
    g_signal_connect(confirm_button, "clicked", G_CALLBACK(confirm_delete_brand), g_strdup(file_path));
    gtk_box_append(GTK_BOX(button_box), confirm_button);

    // Add Cancel button
    GtkWidget *cancel_button = gtk_button_new_with_label("Cancel");
    g_signal_connect_swapped(cancel_button, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(button_box), cancel_button);

    // Show the dialog
    gtk_window_present(GTK_WINDOW(dialog));
}


void confirm_delete_brand(GtkWidget *button, gpointer user_data) {
    const char *file_path = (const char *)user_data;

    // Delete the brand file
    if (remove(file_path) == 0) {
        g_print("Brand file deleted: %s\n", file_path);

        // Also delete the logo associated with the brand
        char brand_name[256];
        snprintf(brand_name, sizeof(brand_name), "%s", g_path_get_basename(file_path));
        char *dot = strrchr(brand_name, '.');
        if (dot) *dot = '\0'; // Remove the .bin extension

        char logo_path[512];
        snprintf(logo_path, sizeof(logo_path), "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase_cars\\%s.png", brand_name);
        if (remove(logo_path) == 0) {
            g_print("Brand logo deleted: %s\n", logo_path);
        } else {
            g_print("Failed to delete brand logo: %s\n", logo_path);
        }

        // Refresh the "voitures" view
        const char *voitures_view_name = "voitures_view_0"; // Assuming first window name
        GtkWidget *voitures_view = gtk_stack_get_child_by_name(GTK_STACK(stack), voitures_view_name);
        if (voitures_view) {
            gtk_stack_remove(GTK_STACK(stack), voitures_view); // Remove the existing view
        }

        // Reopen the voitures view
        on_voitures_clicked(NULL, NULL);
    } else {
        g_print("Failed to delete file: %s\n", file_path);
    }

    // Clean up
    g_free((void *)file_path);

    // Destroy the parent dialog window
    GtkWidget *dialog = gtk_widget_get_ancestor(button, GTK_TYPE_WINDOW);
    if (GTK_IS_WINDOW(dialog)) {
        gtk_window_destroy(GTK_WINDOW(dialog));
    }
}

// Function to copy the logo image to the destination directory
void copy_logo_to_destination(const char *source_path, const char *brand_name, char *output_path, size_t output_path_len) {
    if (!source_path || !brand_name) return;

    // Construct destination path
    char destination_path[512];
    snprintf(destination_path, sizeof(destination_path), "%s%s.png", CAR_IMAGES_DIR, brand_name);

    GFile *source_file = g_file_new_for_path(source_path);
    GFile *destination_file = g_file_new_for_path(destination_path);

    GError *error = NULL;

    // Copy the file
    if (g_file_copy(source_file, destination_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error)) {
        g_print("File copied to: %s\n", destination_path);
        strncpy(output_path, destination_path, output_path_len - 1); // Save final path
        output_path[output_path_len - 1] = '\0';
    } else {
        g_print("Error copying file: %s\n", error->message);
        g_error_free(error);
    }

    g_object_unref(source_file);
    g_object_unref(destination_file);
}

// Function to save the brand data
void save_brand_data(const char *brand_name, const char *logo_path) {
    BrandData brand;
    strncpy(brand.brand_name, brand_name, sizeof(brand.brand_name) - 1);
    brand.brand_name[sizeof(brand.brand_name) - 1] = '\0';
    strncpy(brand.logo_path, logo_path, sizeof(brand.logo_path) - 1);
    brand.logo_path[sizeof(brand.logo_path) - 1] = '\0';

    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s\\%s.bin", BRAND_DATABASE_PATH, brand_name);

    FILE *file = fopen(file_path, "wb");
    if (file) {
        fwrite(&brand, sizeof(BrandData), 1, file);
        fclose(file);
        g_print("Brand data saved successfully: %s\n", file_path);
    } else {
        g_print("Error: Unable to save brand data to the database.\n");
    }
}

// Function to handle the brand addition process
void on_confirm_add_brand(GtkWidget *button, gpointer user_data) {
    GList *fields = (GList *)user_data;

    const char *brand_name = gtk_editable_get_text(GTK_EDITABLE(fields->data));
    const char *logo_path = gtk_editable_get_text(GTK_EDITABLE(fields->next->data));

    if (strlen(brand_name) == 0 || strlen(logo_path) == 0) {
        show_error_dialog("All fields are required.");
        return;
    }

    if (contains_digits(brand_name)) {
        show_error_dialog("Brand name must not contain digits.");
        return;
    }

    if (does_file_exist(BRAND_DATABASE_PATH, brand_name, ".bin")) {
        show_error_dialog("A brand with this name already exists. Please choose a different name.");
        return;
    }

    char final_logo_path[512] = {0};
    copy_logo_to_destination(logo_path, brand_name, final_logo_path, sizeof(final_logo_path));

    if (strlen(final_logo_path) == 0) {
        show_error_dialog("Could not copy the logo file.");
        return;
    }

    save_brand_data(brand_name, final_logo_path);

    g_list_free(fields);

    // Return to the "voitures" view
    on_voitures_clicked(NULL, NULL);
}

// Function to create the brand addition UI
void on_add_brand_clicked(GtkWidget *widget, gpointer user_data) {
    static int add_brand_window_counter = 0;
    char window_name[32];
    snprintf(window_name, sizeof(window_name), "add_brand_window_%d", add_brand_window_counter++);

    if (gtk_stack_get_child_by_name(GTK_STACK(stack), window_name)) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
        return;
    }

    GtkWidget *parent_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(parent_container, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(parent_container, GTK_ALIGN_CENTER);

    GtkWidget *title_label = gtk_label_new("<b>Add New Brand</b>");
    gtk_label_set_use_markup(GTK_LABEL(title_label), TRUE);
    gtk_box_append(GTK_BOX(parent_container), title_label);

    GtkWidget *brand_name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(brand_name_entry), "Brand Name");
    gtk_box_append(GTK_BOX(parent_container), brand_name_entry);

    GtkWidget *logo_path_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(logo_path_entry), "Path to Logo Image");
    gtk_box_append(GTK_BOX(parent_container), logo_path_entry);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(parent_container), button_box);

    GtkWidget *cancel_button = gtk_button_new_with_label("Cancel");
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(on_voitures_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), cancel_button);

    GtkWidget *confirm_button = gtk_button_new_with_label("Add Brand");
    GList *fields = g_list_append(NULL, brand_name_entry);
    fields = g_list_append(fields, logo_path_entry);
    g_signal_connect(confirm_button, "clicked", G_CALLBACK(on_confirm_add_brand), fields);
    gtk_box_append(GTK_BOX(button_box), confirm_button);

    gtk_stack_add_named(GTK_STACK(stack), parent_container, window_name);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
}



void on_close_brand_grid_clicked(GtkWidget *close_button, gpointer user_data) {
    GtkWidget *button = GTK_WIDGET(user_data);

    // Get the parent container
    GtkWidget *container = gtk_widget_get_parent(button);
    GtkWidget *child;

    // Remove the grid and restore the buttons
    for (child = gtk_widget_get_first_child(container); child != NULL; child = gtk_widget_get_next_sibling(child)) {
        if (GTK_IS_GRID(child)) {
            gtk_widget_unparent(child); // Remove the grid
        } else {
            gtk_widget_set_visible(child, TRUE); // Show the buttons
        }
    }

    gtk_widget_set_visible(button, TRUE); // Show the original button
}

void on_x_button_clicked(GtkWidget *x_button, gpointer user_data) {
    // Show all buttons
    for (GList *iter = brand_buttons; iter != NULL; iter = iter->next) {
        gtk_widget_set_visible(GTK_WIDGET(iter->data), TRUE);
    }

    // Remove the grid
    if (brand_grid) {
        gtk_widget_unparent(brand_grid);
        brand_grid = NULL;
    }
}


void load_brands_container(GtkWidget *parent_container) {
    GtkWidget *brand_label = gtk_label_new("<b>List of Brands</b>");
    gtk_label_set_use_markup(GTK_LABEL(brand_label), TRUE);
    gtk_widget_set_halign(brand_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(parent_container), brand_label);

    GtkWidget *brand_scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(brand_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_widget_set_size_request(brand_scrolled_window, 1100, 200);

    GtkWidget *brand_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(brand_scrolled_window), brand_box);

    DIR *dir = opendir(BRAND_DATABASE_PATH);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!g_str_has_suffix(entry->d_name, ".bin")) continue;

        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/%s", BRAND_DATABASE_PATH, entry->d_name);

        FILE *file = fopen(file_path, "rb");
        if (!file) continue;

        BrandData brand;
        fread(&brand, sizeof(BrandData), 1, file);
        fclose(file);

        GtkWidget *brand_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_widget_set_size_request(brand_card, 150, 150);

        GtkWidget *image = gtk_image_new_from_file(brand.logo_path);
        gtk_widget_set_size_request(image, 120, 100);
        gtk_box_append(GTK_BOX(brand_card), image);

        GtkWidget *label = gtk_label_new(brand.brand_name);
        gtk_label_set_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
        gtk_box_append(GTK_BOX(brand_card), label);

        GtkWidget *delete_button = gtk_button_new_with_label("Delete Brand");
        gtk_widget_set_size_request(delete_button, 100, 30);
        g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_brand_clicked), g_strdup(file_path));
        gtk_box_append(GTK_BOX(brand_card), delete_button);

        gtk_box_append(GTK_BOX(brand_box), brand_card);
    }
    closedir(dir);

    gtk_box_append(GTK_BOX(parent_container), brand_scrolled_window);
}

// Function to copy PNG file to the destination directory
void copy_png_to_destination(const char *source_path, const char *model_name, char *output_path, size_t output_path_len) {
    if (!source_path || !model_name) return;

    // Construct destination path
    char destination_path[512];
    snprintf(destination_path, sizeof(destination_path), "%s%s.png", CAR_IMAGES_DIR, model_name);

    GFile *source_file = g_file_new_for_path(source_path);
    GFile *destination_file = g_file_new_for_path(destination_path);

    GError *error = NULL;

    // Copy the file
    if (g_file_copy(source_file, destination_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error)) {
        g_print("File copied to: %s\n", destination_path);
        strncpy(output_path, destination_path, output_path_len - 1); // Save final path
        output_path[output_path_len - 1] = '\0';
    } else {
        g_print("Error copying file: %s\n", error->message);
        g_error_free(error);
    }

    g_object_unref(source_file);
    g_object_unref(destination_file);
}

// Ensure selected_brand_name is set correctly
void on_brand_button_clicked(GtkWidget *button, gpointer user_data) {
    const char *brand_logo_path = (const char *)user_data;

    // Extract the brand name from the logo path
    const char *brand_name = g_path_get_basename(brand_logo_path);
    char *dot_position = strrchr(brand_name, '.');
    if (dot_position != NULL) {
        *dot_position = '\0'; // Remove the file extension
    }

    // Store the brand name and logo path globally
    strncpy(selected_brand_name, brand_name, sizeof(selected_brand_name) - 1);
    selected_brand_name[sizeof(selected_brand_name) - 1] = '\0'; // Ensure null termination
    strncpy(selected_brand_logo_path, brand_logo_path, sizeof(selected_brand_logo_path) - 1);
    selected_brand_logo_path[sizeof(selected_brand_logo_path) - 1] = '\0'; // Ensure null termination

    g_print("Selected brand: %s\n", selected_brand_name); // Debug print

    // Hide all buttons
    for (GList *iter = brand_buttons; iter != NULL; iter = iter->next) {
        gtk_widget_set_visible(GTK_WIDGET(iter->data), FALSE);
    }

    // Create the grid to replace the button
    brand_grid = gtk_grid_new();
    gtk_widget_set_halign(brand_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(brand_grid, GTK_ALIGN_CENTER);

    // Add brand logo
    GtkWidget *logo = gtk_image_new_from_file(brand_logo_path);
    gtk_widget_set_size_request(logo, 60, 60);
    gtk_grid_attach(GTK_GRID(brand_grid), logo, 0, 0, 1, 1);

    // Create the red X button
    GtkWidget *x_button = gtk_button_new_with_label("X");
    gtk_widget_set_size_request(x_button, 30, 30);

    // Apply CSS to the X button
    const char *red_x_button_css =
        "button.red-x {"
        "   background-color: #FF0000;" // Red background
        "   color: #FFFFFF;"            // White text
        "   border-radius: 5px;"        // Rounded corners
        "   padding: 5px;"
        "   font-weight: bold;"
        "}";
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, red_x_button_css, -1);
    GtkStyleContext *context = gtk_widget_get_style_context(x_button);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_class(context, "red-x");

    // Connect the X button click to restore state
    g_signal_connect(x_button, "clicked", G_CALLBACK(on_x_button_clicked), NULL);

    // Add the X button to the grid
    gtk_grid_attach(GTK_GRID(brand_grid), x_button, 1, 0, 1, 1);

    // Replace the button with the grid
    gtk_widget_set_parent(brand_grid, gtk_widget_get_parent(button));
    gtk_widget_set_visible(brand_grid, TRUE);
}
// Use selected_brand_name when saving the car data
void on_confirm_add_car_with_file(GtkWidget *button, gpointer user_data) {
    GList *fields = (GList *)user_data;

    const char *model_name = gtk_editable_get_text(GTK_EDITABLE(fields->data));
    gboolean available = gtk_check_button_get_active(GTK_CHECK_BUTTON(fields->next->data));
    const char *price_str = gtk_editable_get_text(GTK_EDITABLE(fields->next->next->data));
    const char *png_path = gtk_editable_get_text(GTK_EDITABLE(fields->next->next->next->data));

    if (strlen(selected_brand_name) == 0 || strlen(model_name) == 0 || strlen(price_str) == 0 || strlen(png_path) == 0) {
        show_error_dialog("All fields are required.");
        return;
    }

    if (does_file_exist(CAR_DATABASE_PATH, model_name, ".bin")) {
        show_error_dialog("A model with this name already exists. Please choose a different name.");
        return;
    }

    double price = atof(price_str);
    if (price <= 0) {
        show_error_dialog("Price must be greater than zero.");
        return;
    }

    char final_image_path[512] = {0};
    copy_png_to_destination(png_path, model_name, final_image_path, sizeof(final_image_path));

    if (strlen(final_image_path) == 0) {
        show_error_dialog("Could not copy the PNG file.");
        return;
    }

    // Save the car data
    Car car;
    memset(&car, 0, sizeof(Car));
    strncpy(car.model, model_name, sizeof(car.model) - 1);
    strncpy(car.marque, selected_brand_name, sizeof(car.marque) - 1);
    car.available = available ? 1 : 0;
    car.price_per_day = price;
    strncpy(car.image_path, final_image_path, sizeof(car.image_path) - 1);

    char car_file_path[512];
    snprintf(car_file_path, sizeof(car_file_path), "%s/%s.bin", CAR_DATABASE_PATH, model_name);

    FILE *file = fopen(car_file_path, "wb");
    if (file) {
        fwrite(&car, sizeof(Car), 1, file);
        fclose(file);
        g_print("Car added successfully with image: %s\n", final_image_path);
    } else {
        show_error_dialog("Unable to save car to the database.");
        return;
    }

    g_list_free(fields);
    on_voitures_clicked(NULL, NULL);
}
void load_brand_buttons(GtkWidget *container) {
    // Create a scrolled window for horizontal scrolling
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_widget_set_size_request(scrolled_window, 500, 60); // Fixed width and height for the scrollable container

    // Create a horizontal box to hold brand buttons
    GtkWidget *brand_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), brand_box);

    // Open the brand directory
    DIR *dir = opendir(BRAND_DATABASE_PATH);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!g_str_has_suffix(entry->d_name, ".bin")) continue;

        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/%s", BRAND_DATABASE_PATH, entry->d_name);

        FILE *file = fopen(file_path, "rb");
        if (!file) continue;

        BrandData brand;
        fread(&brand, sizeof(BrandData), 1, file);
        fclose(file);

        // Create a button with consistent size
        GtkWidget *button = gtk_button_new();
        gtk_widget_set_size_request(button, 60, 60); // Button size

        // Create and scale the image to fill the button
        GtkWidget *image = gtk_image_new_from_file(brand.logo_path);
        gtk_image_set_pixel_size(GTK_IMAGE(image), 50); // Adjust image to fit within the button
        gtk_widget_set_hexpand(image, TRUE);
        gtk_widget_set_vexpand(image, TRUE);

        gtk_button_set_child(GTK_BUTTON(button), image);

        // Track buttons globally
        brand_buttons = g_list_append(brand_buttons, button);

        // Connect the click event
        g_signal_connect(button, "clicked", G_CALLBACK(on_brand_button_clicked), g_strdup(brand.logo_path));

        // Add the button to the horizontal box
        gtk_box_append(GTK_BOX(brand_box), button);
    }
    closedir(dir);

    // Add the scrollable window to the container
    gtk_box_append(GTK_BOX(container), scrolled_window);
}


void on_add_car_clicked(GtkWidget *widget, gpointer user_data) {
    static int add_car_window_counter = 0;
    char window_name[32];
    snprintf(window_name, sizeof(window_name), "add_car_window_%d", add_car_window_counter++);

    if (gtk_stack_get_child_by_name(GTK_STACK(stack), window_name)) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
        return;
    }

    GtkWidget *parent_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(parent_container, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(parent_container, GTK_ALIGN_CENTER);

    GtkWidget *title_label = gtk_label_new("<b>Add New Car</b>");
    gtk_label_set_use_markup(GTK_LABEL(title_label), TRUE);
    gtk_box_append(GTK_BOX(parent_container), title_label);

    GtkWidget *model_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(model_entry), "Model Name");
    gtk_box_append(GTK_BOX(parent_container), model_entry);

    GtkWidget *brand_title = gtk_label_new("<b>Select Brand</b>");
    gtk_label_set_use_markup(GTK_LABEL(brand_title), TRUE);
    gtk_box_append(GTK_BOX(parent_container), brand_title);

    GtkWidget *brand_buttons_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(parent_container), brand_buttons_container);
    load_brand_buttons(brand_buttons_container);

    GtkWidget *availability_checkbox = gtk_check_button_new_with_label("Available");
    gtk_box_append(GTK_BOX(parent_container), availability_checkbox);

    GtkWidget *price_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(price_entry), "Price Per Day");
    gtk_box_append(GTK_BOX(parent_container), price_entry);

    GtkWidget *png_path_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(png_path_entry), "Path to PNG Image");
    gtk_box_append(GTK_BOX(parent_container), png_path_entry);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(parent_container), button_box);

    GtkWidget *cancel_button = gtk_button_new_with_label("Cancel");
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(on_voitures_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), cancel_button);

    GtkWidget *confirm_button = gtk_button_new_with_label("Add Car");
    GList *fields = g_list_append(NULL, model_entry);
    fields = g_list_append(fields, availability_checkbox);
    fields = g_list_append(fields, price_entry);
    fields = g_list_append(fields, png_path_entry);
    g_signal_connect(confirm_button, "clicked", G_CALLBACK(on_confirm_add_car_with_file), fields);
    gtk_box_append(GTK_BOX(button_box), confirm_button);

    gtk_stack_add_named(GTK_STACK(stack), parent_container, window_name);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
}





void load_cars_page(GtkWidget *parent_container) {
    GtkWidget *car_label = gtk_label_new("<b>List of Cars</b>");
    gtk_label_set_use_markup(GTK_LABEL(car_label), TRUE);
    gtk_widget_set_halign(car_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(parent_container), car_label);

    GtkWidget *car_scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(car_scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(car_scrolled_window, 1100, 400);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(car_scrolled_window), grid);

    DIR *dir = opendir(CAR_DATABASE_PATH);
    if (!dir) return;

    struct dirent *entry;
    int row = 0, col = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (!g_str_has_suffix(entry->d_name, ".bin")) continue;

        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/%s", CAR_DATABASE_PATH, entry->d_name);

        FILE *file = fopen(file_path, "rb");
        if (!file) continue;

        Car car;
        fread(&car, sizeof(Car), 1, file);
        fclose(file);

        GtkWidget *frame = gtk_frame_new(NULL);
        gtk_widget_set_size_request(frame, 300, 350);

        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

        GtkWidget *image = gtk_image_new_from_file(car.image_path);
        gtk_widget_set_size_request(image, 250, 150);
        gtk_box_append(GTK_BOX(box), image);

        char car_info[256];
        snprintf(car_info, sizeof(car_info), "Model: %s\nPrice: %.2f MAD/day", car.model, car.price_per_day);
        GtkWidget *label = gtk_label_new(car_info);
        gtk_label_set_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_box_append(GTK_BOX(box), label);

        GtkWidget *edit_button = gtk_button_new_with_label("Edit Price");
        g_signal_connect(edit_button, "clicked", G_CALLBACK(on_edit_price_clicked), g_strdup(file_path));
        gtk_box_append(GTK_BOX(box), edit_button);

        GtkWidget *delete_button = gtk_button_new_with_label("Delete Car");
        g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_car_clicked), g_strdup(file_path));
        gtk_box_append(GTK_BOX(box), delete_button);

        gtk_frame_set_child(GTK_FRAME(frame), box);
        gtk_grid_attach(GTK_GRID(grid), frame, col, row, 1, 1);

        col++;
        if (col >= 5) { // Limit to 5 cards per row
            col = 0;
            row++;
        }
    }
    closedir(dir);

    gtk_box_append(GTK_BOX(parent_container), car_scrolled_window);
}
void on_delete_car_clicked(GtkWidget *widget, gpointer user_data) {
    const char *file_path = (const char *)user_data;

    // Read car data to display the model name dynamically
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        g_print("Failed to open file: %s\n", file_path);
        return;
    }

    Car car;
    fread(&car, sizeof(Car), 1, file);
    fclose(file);

    // Create the confirmation dialog
    char dialog_title[256];
    snprintf(dialog_title, sizeof(dialog_title), "Delete %s", car.model);

    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), dialog_title);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    // Set transient parent for proper pop-up behavior
    GtkWidget *parent_window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
    if (GTK_IS_WINDOW(parent_window)) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent_window));
    }

    // Add content area
    GtkWidget *content_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(dialog), content_area);

    // Add confirmation text
    GtkWidget *label = gtk_label_new("Are you sure you want to delete this car?");
    gtk_box_append(GTK_BOX(content_area), label);

    // Add "Confirm" and "Cancel" buttons to the content area
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(content_area), button_box);

    // Add Confirm button
    GtkWidget *confirm_button = gtk_button_new_with_label("Confirm");
    g_signal_connect(confirm_button, "clicked", G_CALLBACK(confirm_delete_car), g_strdup(file_path));
    gtk_box_append(GTK_BOX(button_box), confirm_button);

    // Add Cancel button
    GtkWidget *cancel_button = gtk_button_new_with_label("Cancel");
    g_signal_connect_swapped(cancel_button, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(button_box), cancel_button);

    // Show the dialog
    gtk_window_present(GTK_WINDOW(dialog));
}

void confirm_delete_car(GtkWidget *button, gpointer user_data) {
    const char *file_path = (const char *)user_data;

    // Delete the car file
    if (remove(file_path) == 0) {
        g_print("Car file deleted: %s\n", file_path);

        // Extract the car model name from the file path
        char car_name[256];
        snprintf(car_name, sizeof(car_name), "%s", g_path_get_basename(file_path));
        char *dot = strrchr(car_name, '.');
        if (dot) *dot = '\0'; // Remove the .bin extension

        // Construct the image path
        char image_path[512];
        snprintf(image_path, sizeof(image_path), "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase_cars\\%s.png", car_name);

        // Delete the image file
        if (remove(image_path) == 0) {
            g_print("Car image deleted: %s\n", image_path);
        } else {
            g_print("Failed to delete car image: %s\n", image_path);
        }

        // Refresh the "voitures" view
        const char *voitures_view_name = "voitures_view_0"; // Assuming first window name
        GtkWidget *voitures_view = gtk_stack_get_child_by_name(GTK_STACK(stack), voitures_view_name);
        if (voitures_view) {
            gtk_stack_remove(GTK_STACK(stack), voitures_view); // Remove the existing view
        }

        // Reopen the voitures view
        on_voitures_clicked(NULL, NULL);
    } else {
        g_print("Failed to delete file: %s\n", file_path);
    }

    // Clean up
    g_free((void *)file_path);

    // Destroy the parent dialog window
    GtkWidget *dialog = gtk_widget_get_ancestor(button, GTK_TYPE_WINDOW);
    if (GTK_IS_WINDOW(dialog)) {
        gtk_window_destroy(GTK_WINDOW(dialog));
    }
}



void load_brand_scrollable(GtkWidget *parent_container) {
    // Brands Title
    GtkWidget *title_label = gtk_label_new("<b>List of Brands</b>");
    gtk_label_set_use_markup(GTK_LABEL(title_label), TRUE);
    gtk_widget_set_halign(title_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(parent_container), title_label);

    // Horizontal Scrollable Container for Brands
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_widget_set_size_request(scrolled_window, 1100, 180);

    GtkWidget *brand_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), brand_box);

    // Populate brand cards from the database
    DIR *dir = opendir(BRAND_DATABASE_PATH);
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (!g_str_has_suffix(entry->d_name, ".bin")) continue;

        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/%s", BRAND_DATABASE_PATH, entry->d_name);

        FILE *file = fopen(file_path, "rb");
        if (!file) continue;

        BrandData brand;
        fread(&brand, sizeof(BrandData), 1, file);
        fclose(file);

        GtkWidget *brand_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_widget_set_size_request(brand_card, 150, 150);

        // Brand Logo Image
        GtkWidget *image = gtk_image_new_from_file(brand.logo_path);
        gtk_widget_set_size_request(image, 100, 80);
        gtk_box_append(GTK_BOX(brand_card), image);

        // Delete Button
        GtkWidget *delete_button = gtk_button_new_with_label("Delete Brand");
        g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_brand_clicked), g_strdup(file_path));
        gtk_box_append(GTK_BOX(brand_card), delete_button);

        gtk_box_append(GTK_BOX(brand_box), brand_card);
    }
    closedir(dir);

    gtk_box_append(GTK_BOX(parent_container), scrolled_window);
}


void on_voitures_clicked(GtkWidget *widget, gpointer data) {
    static int voitures_view_counter = 0; // Counter for unique view names
    char view_name[32];
    snprintf(view_name, sizeof(view_name), "voitures_view_%d", voitures_view_counter++);

    if (gtk_stack_get_child_by_name(GTK_STACK(stack), view_name)) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), view_name);
        return;
    }

    GtkWidget *parent_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(parent_container, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(parent_container, GTK_ALIGN_CENTER);

    // Add a border to the brand and car containers
    GtkWidget *brand_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(brand_container, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(brand_container, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(brand_container, 10);
    gtk_widget_set_margin_bottom(brand_container, 10);
    gtk_widget_add_css_class(brand_container, "bordered");
    load_brand_scrollable(brand_container);
    gtk_box_append(GTK_BOX(parent_container), brand_container);

    GtkWidget *car_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(car_container, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(car_container, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(car_container, 10);
    gtk_widget_set_margin_bottom(car_container, 10);
    gtk_widget_add_css_class(car_container, "bordered");
    load_cars_page(car_container);
    gtk_box_append(GTK_BOX(parent_container), car_container);

    // Horizontal box for the Add buttons
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);

    // Add a Brand button
    GtkWidget *add_brand_button = gtk_button_new_with_label("Add a Brand");
    g_signal_connect(add_brand_button, "clicked", G_CALLBACK(on_add_brand_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), add_brand_button);

    // Add a Car button
    GtkWidget *add_car_button = gtk_button_new_with_label("Add a Car");
    g_signal_connect(add_car_button, "clicked", G_CALLBACK(on_add_car_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), add_car_button);

    gtk_box_append(GTK_BOX(parent_container), button_box);

    // Return to Admin View button (at the bottom)
    GtkWidget *return_button = gtk_button_new_with_label("Return to Admin View");
    g_signal_connect(return_button, "clicked", G_CALLBACK(open_admin_window), NULL);
    gtk_widget_set_margin_top(return_button, 20);
    gtk_box_append(GTK_BOX(parent_container), return_button);

    gtk_stack_add_named(GTK_STACK(stack), parent_container, view_name);
    gtk_stack_set_visible_child(GTK_STACK(stack), parent_container);
}

void display_car_page(GtkWidget *parent_container) {
    // Create a scrolled window for vertical scrolling
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_set_size_request(scrolled_window, 1300, 900); // Match "Les comptes" dimensions

    // Create the grid to display car cards
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid, GTK_ALIGN_START);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), grid);
    gtk_box_append(GTK_BOX(parent_container), scrolled_window);

    DIR *dir = opendir(CAR_DATABASE_PATH);
    if (!dir) {
        g_print("Error: Unable to open car database directory\n");
        return;
    }

    struct dirent *entry;
    int row = 0, col = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (g_str_has_suffix(entry->d_name, ".bin")) {
            char file_path[512];
            snprintf(file_path, sizeof(file_path), "%s\\%s", CAR_DATABASE_PATH, entry->d_name);

            FILE *file = fopen(file_path, "rb");
            if (!file) continue;

            Car car;
            fread(&car, sizeof(Car), 1, file);
            fclose(file);

            // Create car card
            GtkWidget *frame = gtk_frame_new(NULL);
            gtk_widget_set_size_request(frame, 250, 150); // Match card size with "Les comptes"

            GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

            // Car Image
            GtkWidget *image = gtk_image_new_from_file(car.image_path);
            gtk_widget_set_size_request(image, 200, 100); // Adjust image size
            gtk_box_append(GTK_BOX(box), image);

            // Car Information
            char car_info[256];
            snprintf(car_info, sizeof(car_info), "Model: %s\nPrice: %.2f MAD/day", car.model, car.price_per_day);
            GtkWidget *label = gtk_label_new(car_info);
            gtk_label_set_wrap(GTK_LABEL(label), TRUE);
            gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
            gtk_box_append(GTK_BOX(box), label);

            // Edit Price Button
            GtkWidget *edit_button = gtk_button_new_with_label("Edit Price");
            g_signal_connect(edit_button, "clicked", G_CALLBACK(on_edit_price_clicked), g_strdup(file_path));
            gtk_box_append(GTK_BOX(box), edit_button);

            // Delete Button
            GtkWidget *delete_button = gtk_button_new_with_label("Delete");
            g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_car_clicked), g_strdup(file_path));
            gtk_box_append(GTK_BOX(box), delete_button);

            gtk_frame_set_child(GTK_FRAME(frame), box);
            gtk_grid_attach(GTK_GRID(grid), frame, col, row, 1, 1);

            col++;
            if (col >= 5) { // Wrap after 5 cards per row
                col = 0;
                row++;
            }
        }
    }
    closedir(dir);
}



void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data) {
    if (response_id == GTK_RESPONSE_ACCEPT) {
        g_print("Dialog accepted\n");
        // Add logic for acceptance, like processing input or deletion
    }
    // Destroy the dialog after response
    gtk_window_destroy(GTK_WINDOW(dialog));
}


void on_edit_price_clicked(GtkWidget *widget, gpointer user_data) {
    const char *file_path = (const char *)user_data;

    // Create the dialog for editing the price
    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Edit Car Price");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    GtkWidget *parent_window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
    if (GTK_IS_WINDOW(parent_window)) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent_window));
    }

    // Content area of the dialog
    GtkWidget *content_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(dialog), content_area);

    // Instruction label
    GtkWidget *label = gtk_label_new("Enter the new price:");
    gtk_box_append(GTK_BOX(content_area), label);

    // Price entry widget
    GtkWidget *price_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(price_entry), "New Price");
    gtk_box_append(GTK_BOX(content_area), price_entry);

    // Confirm button
    GtkWidget *confirm_button = gtk_button_new_with_label("Confirm");
    gtk_box_append(GTK_BOX(content_area), confirm_button);

    // Pass required data through a struct
    GList *data_list = g_list_append(NULL, g_strdup(file_path));
    data_list = g_list_append(data_list, price_entry);

    // Connect signals
    g_signal_connect(confirm_button, "clicked", G_CALLBACK(on_edit_price_response), data_list);

    gtk_window_present(GTK_WINDOW(dialog));
}

void on_edit_price_response(GtkWidget *button, gpointer user_data) {
    GList *data_list = (GList *)user_data;
    const char *file_path = (const char *)data_list->data;
    GtkWidget *price_entry = GTK_WIDGET(data_list->next->data);

    const char *new_price_str = gtk_editable_get_text(GTK_EDITABLE(price_entry));

    // Validate the input
    if (!new_price_str || strlen(new_price_str) == 0) {
        g_print("Error: Price input is empty.\n");
        return;
    }

    double new_price = atof(new_price_str);
    if (new_price <= 0) {
        g_print("Error: Invalid price entered.\n");
        return;
    }

    // Open the binary file and update the price
    FILE *file = fopen(file_path, "rb+");
    if (file) {
        Car car;
        fread(&car, sizeof(Car), 1, file);
        fseek(file, 0, SEEK_SET); // Reset file pointer to the beginning
        car.price_per_day = new_price;
        fwrite(&car, sizeof(Car), 1, file);
        fclose(file);

        g_print("Price updated successfully for car: %s\n", car.model);
    } else {
        g_print("Error: Unable to open file %s for writing.\n", file_path);
    }

    // Clean up
    g_free((void *)file_path);
    g_list_free(data_list);

    // Close the parent dialog window
    GtkWidget *dialog = gtk_widget_get_parent(gtk_widget_get_parent(button));
    gtk_window_destroy(GTK_WINDOW(dialog));

    // Refresh the "voitures" view
    const char *voitures_view_name = "voitures_window_0"; // Assuming first window name
    GtkWidget *voitures_view = gtk_stack_get_child_by_name(GTK_STACK(stack), voitures_view_name);
    if (voitures_view) {
        gtk_stack_remove(GTK_STACK(stack), voitures_view); // Remove the existing view
    }

    on_voitures_clicked(NULL, NULL); // Reload the voitures view
}


GtkWidget* create_admin_button(const char *label, GCallback callback, gpointer data) {
    GtkWidget *button = gtk_button_new_with_label(label);
    g_signal_connect(button, "clicked", callback, data);
    return button;
}

static void on_button_clicked(GtkWidget *widget, gpointer data) {
    g_print("%s button clicked\n", (char *)data);
}

int get_total_user_count() {
    DIR *dir = opendir(USER_DATABASE_PATH);
    int count = 0;
    if (!dir) {
        g_print("Error: Unable to open user database directory\n");
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (g_str_has_suffix(entry->d_name, ".bin")) {
            count++;
        }
    }
    closedir(dir);
    return count;
}

void on_les_comptes_clicked(GtkWidget *widget, gpointer data) {
    static int les_comptes_counter = 0;
    char window_name[32];
    snprintf(window_name, sizeof(window_name), "les_comptes_%d", les_comptes_counter++);

    // Check if the window already exists
    if (gtk_stack_get_child_by_name(GTK_STACK(stack), window_name)) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
        return;
    }

    // Main container for the accounts
    GtkWidget *parent_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(parent_container, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(parent_container, GTK_ALIGN_CENTER);

    // Title for the window
    GtkWidget *title_label = gtk_label_new("<b>List of User Accounts</b>");
    gtk_label_set_use_markup(GTK_LABEL(title_label), TRUE);
    gtk_widget_set_halign(title_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(parent_container), title_label);

    // Load the first page of user accounts
    load_user_page(parent_container, 0);  // Start from page 0

    // Back button at the bottom
    GtkWidget *back_button = gtk_button_new_with_label("Back to Admin Menu");
    g_signal_connect(back_button, "clicked", G_CALLBACK(open_admin_window), NULL);
    gtk_box_append(GTK_BOX(parent_container), back_button);

    // Add the container to the stack with a unique name
    gtk_stack_add_named(GTK_STACK(stack), parent_container, window_name);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
}

void load_user_page(GtkWidget *parent_container, int page) {
    total_users = 0; // Reset user count

    // Clear the container first
    while (gtk_widget_get_first_child(parent_container) != NULL) {
        gtk_widget_unparent(gtk_widget_get_first_child(parent_container));
    }

    // Title for the page
    GtkWidget *title_label = gtk_label_new("<b>List of User Accounts</b>");
    gtk_label_set_use_markup(GTK_LABEL(title_label), TRUE);
    gtk_widget_set_halign(title_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(parent_container), title_label);

    // Create a scrolled window for vertical scrolling
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_set_size_request(scrolled_window, 1300, 900); // Increased size for 5 rows

    // Create the grid to display user accounts
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid, GTK_ALIGN_START);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), grid);

    // Read user files
    DIR *dir = opendir(USER_DATABASE_PATH);
    if (!dir) {
        g_print("Error: Unable to open user database directory\n");
        return;
    }

    struct dirent *entry;
    int row = 0, col = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (g_str_has_suffix(entry->d_name, ".bin")) {
            char file_path[512];
            snprintf(file_path, sizeof(file_path), "%s\\%s", USER_DATABASE_PATH, entry->d_name);

            FILE *file = fopen(file_path, "rb");
            if (!file) continue;

            User user;
            fread(&user, sizeof(User), 1, file);
            fclose(file);

            // Create user card
            GtkWidget *frame = gtk_frame_new(NULL);
            gtk_widget_set_size_request(frame, 250, 150); // Card size

            GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
            char user_info[256];
            snprintf(user_info, sizeof(user_info), "Name: %s %s\nCIN: %s\nPhone: %s\nEmail: %s\nRole: %s",
                     user.name, user.lastname, user.cin, user.phonenumber, user.email, user.role);

            GtkWidget *label = gtk_label_new(user_info);
            gtk_label_set_wrap(GTK_LABEL(label), TRUE);
            gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

            // Delete Button
            GtkWidget *delete_button = gtk_button_new_with_label("Delete");
            char *file_path_copy = g_strdup(file_path);
            g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_user_clicked), file_path_copy);

            gtk_box_append(GTK_BOX(box), label);
            gtk_box_append(GTK_BOX(box), delete_button);
            gtk_frame_set_child(GTK_FRAME(frame), box);

            // Attach card to grid
            gtk_grid_attach(GTK_GRID(grid), frame, col, row, 1, 1);

            col++;
            if (col >= 5) { // Wrap after 5 cards horizontally
                col = 0;
                row++;
            }
        }
    }
    closedir(dir);

    // Add the scrolled window to the parent container
    gtk_box_append(GTK_BOX(parent_container), scrolled_window);
}





void on_previous_page(GtkWidget *widget, gpointer data) {
    int page = GPOINTER_TO_INT(data); // Directly cast the pointer to int
    if (page > 0) {
        load_user_page(widget, page - 1);  // Decrement page to go to previous page
    }
}

void on_next_page(GtkWidget *widget, gpointer data) {
    int page = GPOINTER_TO_INT(data);
    load_user_page(widget, page + 1);  // Increment page to go to next page
}

void on_delete_user_clicked(GtkWidget *widget, gpointer user_data) {
    const char *file_path = (const char *)user_data;

    // Read the user data to display name dynamically
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        g_print("Failed to open file: %s\n", file_path);
        return;
    }

    User user;
    fread(&user, sizeof(User), 1, file);
    fclose(file);

    // Generate window title
    char title[256];
    snprintf(title, sizeof(title), "Delete %s %s", user.name, user.lastname);

    // Create confirmation dialog
    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    GtkWidget *parent_window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
    if (GTK_IS_WINDOW(parent_window)) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent_window));
    }

    GtkBox *content_area = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
    GtkWidget *label = gtk_label_new("Are you sure you want to delete this account?");
    gtk_box_append(GTK_BOX(content_area), label);

    gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", GTK_RESPONSE_REJECT);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "Confirm", GTK_RESPONSE_ACCEPT);

    // Pass file path as user data
    char *file_path_copy = g_strdup(file_path);
    g_signal_connect_data(dialog, "response", G_CALLBACK(confirm_delete_user), file_path_copy, (GClosureNotify)g_free, 0);

    gtk_window_present(GTK_WINDOW(dialog));
}

void confirm_delete_user(GtkWidget *dialog, gint response_id, gpointer user_data) {
    const char *file_path = (const char *)user_data;

    if (response_id == GTK_RESPONSE_ACCEPT) {
        // Proceed to delete the user file
        if (remove(file_path) == 0) {
            g_print("User file deleted: %s\n", file_path);

            // Find the parent window or stack
            GtkWidget *parent_window = gtk_widget_get_ancestor(GTK_WIDGET(dialog), GTK_TYPE_WINDOW);
            if (GTK_IS_WIDGET(parent_window)) {
                // Reopen the "Les comptes" window
                on_les_comptes_clicked(NULL, gtk_widget_get_parent(parent_window));
            }
        } else {
            g_print("Failed to delete file: %s\n", file_path);
        }
    }

    // Destroy the confirmation dialog
    gtk_window_destroy(GTK_WINDOW(dialog));
}

void open_create_mod_window(GtkWidget *widget, gpointer data) {
    static int mod_window_counter = 0;
    char window_name[32];
    snprintf(window_name, sizeof(window_name), "create_mod_%d", mod_window_counter++);

    // Check if the window already exists
    if (gtk_stack_get_child_by_name(GTK_STACK(stack), window_name)) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
        return;
    }

    // Main container
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    // Title
    GtkWidget *title = gtk_label_new("<b>Create Moderator Account</b>");
    gtk_label_set_use_markup(GTK_LABEL(title), TRUE);
    gtk_widget_set_halign(title, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), title);

    // Input fields
    GtkWidget *name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "Name");
    gtk_box_append(GTK_BOX(box), name_entry);

    GtkWidget *lastname_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(lastname_entry), "Last Name");
    gtk_box_append(GTK_BOX(box), lastname_entry);

    GtkWidget *cin_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(cin_entry), "CIN");
    gtk_box_append(GTK_BOX(box), cin_entry);

    // Submit button
    GtkWidget *create_button = gtk_button_new_with_label("Create Moderator");
    gtk_box_append(GTK_BOX(box), create_button);

    // Back button
    GtkWidget *back_button = gtk_button_new_with_label("Back to Admin Menu");
    g_signal_connect(back_button, "clicked", G_CALLBACK(open_admin_window), NULL);
    gtk_box_append(GTK_BOX(box), back_button);

    // Connect create button signal
    g_signal_connect(create_button, "clicked", G_CALLBACK(create_moderator_account),
                     g_list_append(g_list_append(g_list_append(NULL, name_entry), lastname_entry), cin_entry));

    // Add to stack
    gtk_stack_add_named(GTK_STACK(stack), box, window_name);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
}

char* generate_random_code() {
    static char code[7]; // 6 digits + null terminator
    srand(time(NULL));
    for (int i = 0; i < 6; i++) {
        code[i] = '0' + (rand() % 10); // Random digit between 0-9
    }
    code[6] = '\0';
    return code;
}

void create_moderator_account(GtkWidget *widget, gpointer data) {
    GList *fields = (GList *)data;
    const char *name = gtk_editable_get_text(GTK_EDITABLE(fields->data));
    const char *lastname = gtk_editable_get_text(GTK_EDITABLE(fields->next->data));
    const char *cin = gtk_editable_get_text(GTK_EDITABLE(fields->next->next->data));

    // Validate inputs
    if (!is_valid_name(name) || !is_valid_name(lastname) || strlen(cin) < 4) {
        show_error_message(GTK_WIDGET(widget), "Invalid input. Check Name, Last Name, and CIN.");
        return;
    }

    // Generate email
    char email[256];
    snprintf(email, sizeof(email), "%s.%s@svr.org", name, lastname);

    // Generate random 6-digit code
    char *random_code = generate_random_code();

    // Hash the password
    char *hashed_password = hash_password(random_code);

    // Prepare user structure
    User moderator;
    memset(&moderator, 0, sizeof(User));
    strncpy(moderator.name, name, sizeof(moderator.name) - 1);
    strncpy(moderator.lastname, lastname, sizeof(moderator.lastname) - 1);
    strncpy(moderator.cin, cin, sizeof(moderator.cin) - 1);
    strncpy(moderator.email, email, sizeof(moderator.email) - 1);
    strncpy(moderator.role, "Moderator", sizeof(moderator.role) - 1);
    strncpy(moderator.phonenumber, random_code, sizeof(moderator.phonenumber) - 1); // Store plain code
    strncpy(moderator.password, hashed_password, sizeof(moderator.password) - 1);   // Store hashed code

    // Save user data
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s\\%s.bin", USER_DATABASE_PATH, cin);

    FILE *file = fopen(file_path, "wb");
    if (file) {
        fwrite(&moderator, sizeof(User), 1, file);
        fclose(file);

        // Show confirmation dialog
        char message[512];
        snprintf(message, sizeof(message),
                 "Account created successfully!\n\nEmail: %s\nPassword: %s",
                 email, random_code);
        show_success_message(widget, message);

        g_free(hashed_password);
        g_list_free(fields);
    } else {
        show_error_message(GTK_WIDGET(widget), "Error saving moderator account.");
    }
}

void show_success_message(GtkWidget *parent, const char *message) {
    // Create a new message dialog
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(gtk_widget_get_ancestor(parent, GTK_TYPE_WINDOW)),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_CLOSE,
        "Account Created Successfully!"
    );

    // Add details to the dialog
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", message);

    // Connect response signal to close dialog
    g_signal_connect(dialog, "response", G_CALLBACK(on_close_dialog_clicked), NULL);

    // Present the dialog
    gtk_window_present(GTK_WINDOW(dialog));
}

void on_close_dialog_clicked(GtkDialog *dialog, gint response_id, gpointer user_data) {
    gtk_window_destroy(GTK_WINDOW(dialog));  // Destroy the dialog
    open_admin_window(NULL, NULL);           // Return to the admin menu
}


void open_admin_window(GtkWidget *widget, gpointer data) {
    static int admin_view_counter = 0;
    char admin_view_name[32];
    snprintf(admin_view_name, sizeof(admin_view_name), "admin_view_%d", admin_view_counter++);

    // Main vertical container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(main_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(main_box, GTK_ALIGN_CENTER);

    // Title Label
    GtkWidget *title_label = gtk_label_new("<b>Admin Menu</b>");
    gtk_label_set_use_markup(GTK_LABEL(title_label), TRUE);
    gtk_widget_set_halign(title_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_box), title_label);

    // Add buttons using a helper function
    gtk_box_append(GTK_BOX(main_box), create_admin_button("Les comptes", G_CALLBACK(on_les_comptes_clicked), main_box));
    gtk_box_append(GTK_BOX(main_box), create_admin_button("Historique + Statistique", G_CALLBACK(on_button_clicked), "Historique + Statistique"));
    gtk_box_append(GTK_BOX(main_box), create_admin_button("Voitures", G_CALLBACK(on_voitures_clicked), "Voitures"));
    gtk_box_append(GTK_BOX(main_box), create_admin_button("Creer compte mod", G_CALLBACK(open_create_mod_window), "Creer compte mod"));

    // Back Button
    GtkWidget *back_button = gtk_button_new_with_label("logout");
    gtk_box_append(GTK_BOX(main_box), back_button);
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_to_main_menu), NULL);

    // Add the main container to the stack
    gtk_stack_add_named(GTK_STACK(stack), main_box, admin_view_name);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), admin_view_name);
}