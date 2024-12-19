#include <gtk/gtk.h>
#include "client.h"
#include "user.h"
#include "car.h"
#include <stdio.h>
#include "utils.h"
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "req.h"

#define CAR_DATABASE_DIR "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase_cars\\DataBase_models"
#define REQUEST_DIR "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase_req"



typedef struct {
    Car *car;
    char model_window_name[256];
} RequestWindowData;

GtkWidget *client_info_label = NULL;  // Global or static reference

const char* get_status_icon_path(int status) {
    switch (status) {
        case 1: return "C:\\Users\\mouad\\CLionProjects\\car_rental\\utils_png\\green.png"; // Approved
        case 0: return "C:\\Users\\mouad\\CLionProjects\\car_rental\\utils_png\\yellow.png"; // Pending
        case -1: return "C:\\Users\\mouad\\CLionProjects\\car_rental\\utils_png\\red.png";   // Rejected
        default: return "";
    }
}

void delete_request_file(const char *file_path) {
    if (remove(file_path) == 0) {
        g_print("Request file %s deleted successfully.\n", file_path);
    } else {
        g_print("Error deleting file: %s\n", file_path);
    }
}


GtkWidget *duration_entry;
GtkWidget *day_entry;
GtkWidget *month_entry;
GtkWidget *year_entry;


typedef struct {
    GtkWidget *entry;
    const char *edit_type;
    GtkWidget *label;
} EditPopupData;

typedef struct {
    const char *edit_type;
    GtkWidget *entry;
    GtkWidget *label;
    GtkWidget *window;
} SaveEditData;

extern GtkWidget *stack;
extern GList *window_stack;


void show_error_message1(GtkWidget *parent, const char *message) {
    // Create a new dialog
    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Message");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

    // Add an "OK" button
    gtk_dialog_add_button(GTK_DIALOG(dialog), "OK", GTK_RESPONSE_CLOSE);

    // Get the content area
    GtkBox *content_area = GTK_BOX(gtk_window_get_child(GTK_WINDOW(dialog)));

    // Create a label with the message
    GtkWidget *label = gtk_label_new(message);
    gtk_box_append(content_area, label);

    // Connect the "response" signal to destroy the dialog
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);

    // Present the dialog
    gtk_window_present(GTK_WINDOW(dialog));
}


static void return_back1(GtkWidget *widget, gpointer data) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "client_view");
}


static void return_back2(GtkWidget *widget, gpointer data) {
    // Get the current visible child from the stack
    const gchar *current_child_name = gtk_stack_get_visible_child_name(GTK_STACK(stack));

    // Check if it's the models window, and then show the catalogue window
    if (g_str_has_prefix(current_child_name, "models_")) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), "catalogue_view");
    }
}


static void return_back3(GtkWidget *widget, gpointer data) {
    const char *brand_name = (const char *)data;

    // Build the window name for the models view
    char models_window_name[256];
    snprintf(models_window_name, sizeof(models_window_name), "models_%s", brand_name);

    // Set the visible child to the models view
    gtk_stack_set_visible_child_name(GTK_STACK(stack), models_window_name);
}


static void handle_save_or_cancel(GtkDialog *dialog, gint response_id, gpointer user_data) {
    EditPopupData *popup_data = (EditPopupData *)user_data;
    GtkWidget *entry = popup_data->entry;
    const char *edit_type = popup_data->edit_type;
    GtkWidget *label = popup_data->label;

    // Retrieve the text from the entry buffer
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
    const char *new_value = gtk_entry_buffer_get_text(buffer);

    if (response_id == GTK_RESPONSE_ACCEPT) {
        if (strcmp(edit_type, "email") == 0) {
            if (!is_valid_email(new_value)) {
                show_error_message1(GTK_WIDGET(dialog), "Invalid email address!");
                return; // Keep the dialog open
            }
            strncpy(current_user.email, new_value, sizeof(current_user.email) - 1);
            current_user.email[sizeof(current_user.email) - 1] = '\0';
            gtk_label_set_text(GTK_LABEL(label), current_user.email);
        } else if (strcmp(edit_type, "phone") == 0) {
            if (!is_valid_moroccan_phone(new_value)) {
                show_error_message1(GTK_WIDGET(dialog), "Invalid phone number! Must start with 06/07 and have 10 digits.");
                return; // Keep the dialog open
            }
            strncpy(current_user.phonenumber, new_value, sizeof(current_user.phonenumber) - 1);
            current_user.phonenumber[sizeof(current_user.phonenumber) - 1] = '\0';
            gtk_label_set_text(GTK_LABEL(label), current_user.phonenumber);
        }

        if (!save_user_data(current_user.cin)) {
            show_error_message1(GTK_WIDGET(dialog), "Failed to save changes!");
            return; // Keep the dialog open
        }

        show_error_message1(GTK_WIDGET(dialog), "Changes saved successfully!");
    }

    // Properly destroy the current dialog
    gtk_window_destroy(GTK_WINDOW(dialog));
    g_free(popup_data); // Free allocated memory for this specific dialog
}


void open_edit_popup(GtkWidget *widget, gpointer data) {
    EditPopupData *popup_data = g_new(EditPopupData, 1);
    *popup_data = *(EditPopupData *)data;  // Duplicate the original data safely

    const char *edit_type = popup_data->edit_type;

    // Create a fresh dialog instance each time
    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Edit Information");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

    // Set the dialog transient for the parent window
    GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW));
    if (parent_window) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), parent_window);
    }

    // Add Save and Cancel buttons
    gtk_dialog_add_button(GTK_DIALOG(dialog), "Save", GTK_RESPONSE_ACCEPT);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", GTK_RESPONSE_REJECT);

    // Content area
    GtkBox *content_area = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_append(content_area, grid);

    GtkWidget *field_label;
    GtkWidget *entry = gtk_entry_new();

    // Add fields based on the edit type
    if (strcmp(edit_type, "email") == 0) {
        field_label = gtk_label_new("Enter new email:");
        gtk_grid_attach(GTK_GRID(grid), field_label, 0, 0, 1, 1);
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "example@example.com");
    } else if (strcmp(edit_type, "phone") == 0) {
        field_label = gtk_label_new("Enter new phone number:");
        gtk_grid_attach(GTK_GRID(grid), field_label, 0, 0, 1, 1);
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "06XXXXXXXX");
    }

    gtk_grid_attach(GTK_GRID(grid), entry, 1, 0, 1, 1);

    popup_data->entry = entry;

    // Connect the response signal
    g_signal_connect(dialog, "response", G_CALLBACK(handle_save_or_cancel), popup_data);

    gtk_window_present(GTK_WINDOW(dialog));
}

const char *find_brand_logo(const char *brand_name) {
    GList *brands = g_list_first(catalogue);
    while (brands != NULL) {
        MarqueCatalogue *marque = (MarqueCatalogue *)brands->data;
        if (strcmp(marque->marque, brand_name) == 0) {
            return marque->logo_path;  // Return the brand's logo path
        }
        brands = g_list_next(brands);
    }
    return NULL;  // Return NULL if not found
}


void show_brand_models(GtkWidget *catalogue_grid, MarqueCatalogue *marque) {
    GtkWidget *scroll_window, *models_box, *back_button;

    // Create a scrolling window for the models
    scroll_window = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scroll_window, 400, 300);  // Set scroll window size

    // Add the scroll window to the catalogue grid (which is a GtkBox)
    gtk_box_append(GTK_BOX(catalogue_grid), scroll_window);

    // Create a vertical box for the models (this is the container where model buttons will go)
    models_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);  // Create a new vertical box for models
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), models_box);  // Set the box inside the scroll window

    // Iterate over the models of the brand
    GList *models = g_list_first(marque->models);
    while (models != NULL) {
        Car *car = (Car *)models->data;

        // Create a button for each model
        GtkWidget *model_button = create_item_button(car->model, marque->logo_path, FALSE);  // False because it's a model

        // Connect the model button to the model details window
        g_signal_connect(model_button, "clicked", G_CALLBACK(open_model_details_window), car);

        // Add the model button to the models box (which is a GtkBox)
        gtk_box_append(GTK_BOX(models_box), model_button);
        models = g_list_next(models);
    }

    // Add a back button to return to the previous screen (catalogue view)
    back_button = gtk_button_new_with_label("Back to Catalogue");
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_back2), NULL);
    gtk_box_append(GTK_BOX(models_box), back_button);  // Add back button to the models box
}


static void on_button_clicked(GtkWidget *widget, gpointer data) {
    g_print("%s button clicked\n", (char *)data);
}


static void return_to_main_view(GtkWidget *widget, gpointer data) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "main_view");
}


void open_client_window(GtkApplication *app, gpointer user_data) {
    GtkWidget *main_grid, *top_grid, *button_grid;
    GtkWidget *label, *logout_button;
    GtkWidget *buttons[5];
    const char *button_labels[] = {
        "Catalogue",
        "List des voitures dispo",
        "Profile + Modify",
        "Etat Demande",
        "Historique"
    };


    // Create main grid
    main_grid = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(main_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(main_grid, GTK_ALIGN_CENTER);

    // Create top grid
    top_grid = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    // Create label and set dynamic user info
    char user_info[200];
    snprintf(user_info, sizeof(user_info), "Client Information: Name: %s %s   ID: %s",
             current_user.name, current_user.lastname, current_user.cin);

    if (!client_info_label) {
        // Create the label if it doesn't already exist
        client_info_label = gtk_label_new(user_info);
    } else {
        // Update the label text dynamically
        gtk_label_set_text(GTK_LABEL(client_info_label), user_info);
    }

    gtk_box_append(GTK_BOX(top_grid), client_info_label);

    // Logout button
    logout_button = gtk_button_new_with_label("Log Out");
    g_signal_connect(logout_button, "clicked", G_CALLBACK(return_to_main_view), NULL);
    gtk_box_append(GTK_BOX(top_grid), logout_button);

    gtk_widget_set_halign(logout_button, GTK_ALIGN_END);
    gtk_widget_set_hexpand(logout_button, TRUE);

    gtk_box_append(GTK_BOX(main_grid), top_grid);

    // Button grid
    button_grid = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(button_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(button_grid, GTK_ALIGN_CENTER);

    for (int i = 0; i < 5; i++) {
        buttons[i] = gtk_button_new_with_label(button_labels[i]);
        // Add signal handler for Catalogue button to open the catalogue window
        if (i == 0) {
            g_signal_connect(buttons[i], "clicked", G_CALLBACK(open_catalogue_window), NULL);  // Connect the Catalogue button to open_catalogue_window
        } else if (i == 1) {
            g_signal_connect(buttons[i], "clicked", G_CALLBACK(open_car_dispo_window), NULL);
        } else if (i == 2) {
            g_signal_connect(buttons[i], "clicked", G_CALLBACK(open_profile_window), NULL);
        } else if (i == 3) {
            g_signal_connect(buttons[i], "clicked", G_CALLBACK(on_etat_demande_button_clicked), NULL);
        } else {
            g_signal_connect(buttons[i], "clicked", G_CALLBACK(on_button_clicked), (gpointer)button_labels[i]);
        }
        gtk_box_append(GTK_BOX(button_grid), buttons[i]);
    }

    gtk_box_append(GTK_BOX(main_grid), button_grid);

    // Add the client view to the stack
    gtk_stack_add_named(GTK_STACK(stack), main_grid, "client_view");

    // Show the client view
    gtk_stack_set_visible_child(GTK_STACK(stack), main_grid);
}



void open_catalogue_window(GtkApplication *app, gpointer user_data) {
    GtkWidget *catalogue_grid;
    GtkWidget *back_button;
    GList *brands;

    // Create the catalogue window grid (GtkGrid)
    catalogue_grid = gtk_grid_new();  // Use GtkGrid to make a grid layout
    gtk_widget_set_halign(catalogue_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(catalogue_grid, GTK_ALIGN_CENTER);

    int row = 0, col = 0;

    // Add brand buttons dynamically
    brands = g_list_first(catalogue);  // Iterate through the catalogue list
    while (brands != NULL) {
        MarqueCatalogue *marque = (MarqueCatalogue *)brands->data;

        // Create a button with a vertical layout: logo on top, text below
        GtkWidget *brand_button = gtk_button_new();
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5); // Vertical box for content
        gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);

        // Create the logo image
        GtkWidget *logo_image = gtk_image_new_from_file(marque->logo_path);
        gtk_widget_set_size_request(logo_image, 64, 64);  // Set a larger size for the logo

        // Create the label for the brand name
        GtkWidget *label = gtk_label_new(marque->marque);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);

        // Add the logo and label to the vertical box
        gtk_box_append(GTK_BOX(vbox), logo_image);
        gtk_box_append(GTK_BOX(vbox), label);

        // Add the vertical box to the button
        gtk_button_set_child(GTK_BUTTON(brand_button), vbox);

        // Connect the brand button to show its models when clicked
        g_signal_connect(brand_button, "clicked", G_CALLBACK(open_models_window), marque);  // Pass the brand data (marque)

        // Add the brand button to the grid (creating a grid layout)
        gtk_grid_attach(GTK_GRID(catalogue_grid), brand_button, col, row, 1, 1);

        // Update row and column for grid layout
        col++;
        if (col > 3) {  // 3 columns in a grid
            col = 0;
            row++;
        }

        brands = g_list_next(brands);
    }

    // Back button to return to client view
    back_button = gtk_button_new_with_label("Back");
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_back1), NULL);
    gtk_grid_attach(GTK_GRID(catalogue_grid), back_button, 0, row + 1, 4, 1);  // Span 4 columns

    // Add the catalogue view to the stack
    gtk_stack_add_named(GTK_STACK(stack), catalogue_grid, "catalogue_view");

    // Show the catalogue view and hide the client view
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "catalogue_view");
}



void open_models_window(GtkWidget *widget, gpointer user_data) {
    MarqueCatalogue *marque = (MarqueCatalogue *)user_data;
    GtkWidget *models_grid;
    GtkWidget *back_button;
    GList *models;

    // Create the models window grid (GtkGrid)
    models_grid = gtk_grid_new();
    gtk_widget_set_halign(models_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(models_grid, GTK_ALIGN_CENTER);

    int row = 0, col = 0;

    // Iterate through the models of the brand
    models = g_list_first(marque->models);
    while (models != NULL) {
        Car *car = (Car *)models->data;

        // Create a button for each model
        GtkWidget *model_button = create_item_button(car->model, marque->logo_path, FALSE);  // False because it's a model

        // Connect the model button to the model details window
        g_signal_connect(model_button, "clicked", G_CALLBACK(open_model_details_window), car);

        // Add the model button to the grid
        gtk_grid_attach(GTK_GRID(models_grid), model_button, col, row, 1, 1);

        col++;
        if (col > 3) {  // 3 columns in the grid
            col = 0;
            row++;
        }

        models = g_list_next(models);
    }

    // Add a back button
    back_button = gtk_button_new_with_label("Back to Catalogue");
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_back2), NULL);
    gtk_grid_attach(GTK_GRID(models_grid), back_button, 0, row + 1, 4, 1);

    // Add the models view to the stack
    char window_name[256];
    snprintf(window_name, sizeof(window_name), "models_%s", marque->marque);
    gtk_stack_add_named(GTK_STACK(stack), models_grid, window_name);

    // Show the models view
    gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
}


void open_model_details_window(GtkWidget *widget, gpointer user_data) {
    Car *car = (Car *)user_data;

    // Create a unique name for this window
    char window_name[256];
    snprintf(window_name, sizeof(window_name), "model_%s", car->model);

    if (gtk_stack_get_child_by_name(GTK_STACK(stack), window_name)) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
        return;
    }

    // Apply inline CSS using GtkCssProvider
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css_data =
        "frame.bordered-box {"
        "    border: 3px solid #555;"
        "    border-radius: 10px;"
        "    padding: 10px;"
        "}"
        "button {"
        "    margin: 10px;"
        "}";
    gtk_css_provider_load_from_data(provider, css_data, -1);

    // Add the CSS provider to the display
    GtkStyleContext *context;
    context = gtk_widget_get_style_context(GTK_WIDGET(stack));
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    // Create a grid for content
    GtkWidget *details_grid = gtk_grid_new();
    gtk_widget_set_halign(details_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(details_grid, GTK_ALIGN_CENTER);
    gtk_grid_set_row_spacing(GTK_GRID(details_grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(details_grid), 20);

    // Wrap content in a GtkFrame (bordered container)
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_widget_add_css_class(frame, "bordered-box");  // Apply the 'bordered-box' class
    gtk_widget_set_size_request(frame, 400, 400);

    GtkWidget *frame_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(frame_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(frame_box, GTK_ALIGN_CENTER);

    // 1. Model Image
    GtkWidget *large_image = gtk_image_new_from_file(car->image_path);
    gtk_widget_set_size_request(large_image, 300, 300);
    gtk_box_append(GTK_BOX(frame_box), large_image);

    // 2. Brand Logo
    GtkWidget *brand_logo = gtk_image_new_from_file(find_brand_logo(car->marque));
    gtk_widget_set_size_request(brand_logo, 100, 100);
    gtk_box_append(GTK_BOX(frame_box), brand_logo);

    // 3. Model Name
    GtkWidget *model_name_label = gtk_label_new(NULL);
    char model_name_text[256];
    char *escaped_model = g_markup_escape_text(car->model, -1);
    snprintf(model_name_text, sizeof(model_name_text), "<b>%s</b>", escaped_model);
    gtk_label_set_markup(GTK_LABEL(model_name_label), model_name_text);
    g_free(escaped_model);
    gtk_box_append(GTK_BOX(frame_box), model_name_label);

    // 4. Price Sentence
    GtkWidget *price_label = gtk_label_new(NULL);
    char price_text[128];
    snprintf(price_text, sizeof(price_text), "Rent this car for %.2f MAD per day!", car->price_per_day);
    gtk_label_set_text(GTK_LABEL(price_label), price_text);
    gtk_box_append(GTK_BOX(frame_box), price_label);

    // 5. Back Button
    GtkWidget *back_button = gtk_button_new_with_label("Back to Models");
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_back3), (gpointer)car->marque);
    gtk_box_append(GTK_BOX(frame_box), back_button);

    // Add the box to the frame
    gtk_frame_set_child(GTK_FRAME(frame), frame_box);

    // Add the frame to the grid
    gtk_grid_attach(GTK_GRID(details_grid), frame, 0, 0, 1, 1);

    // Add the details grid to the stack
    gtk_stack_add_named(GTK_STACK(stack), details_grid, window_name);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
}


void open_profile_window(GtkWidget *widget, gpointer user_data) {
    static int profile_count = 0;  // Track the number of profile windows
    char window_name[50];
    snprintf(window_name, sizeof(window_name), "profile_%d", ++profile_count);

    // Check if the window already exists
    if (gtk_stack_get_child_by_name(GTK_STACK(stack), window_name)) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
        return;
    }

    // Create main profile grid
    GtkWidget *profile_grid = gtk_grid_new();
    gtk_widget_set_halign(profile_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(profile_grid, GTK_ALIGN_CENTER);
    gtk_grid_set_row_spacing(GTK_GRID(profile_grid), 20);

    // Apply CSS directly for styling
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css_data =
        "frame.profile-box {"
        "    border: 3px solid #555;"
        "    border-radius: 10px;"
        "    padding: 20px;"
        "} "
        "button.mini-btn {"
        "    font-size: 12px;"
        "    padding: 5px;"
        "    margin-left: 10px;"
        "}";
    gtk_css_provider_load_from_data(provider, css_data, -1);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    // Create the frame for the profile box
    GtkWidget *profile_frame = gtk_frame_new(NULL);
    gtk_widget_add_css_class(profile_frame, "profile-box");

    // Container inside the frame
    GtkWidget *profile_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);

    // Title
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span font='20' weight='bold'>PROFILE</span>");
    gtk_box_append(GTK_BOX(profile_box), title);

    // Personal Information Section
    GtkWidget *personal_info_label = gtk_label_new("<b>Personal Information</b>");
    gtk_label_set_use_markup(GTK_LABEL(personal_info_label), TRUE);
    gtk_box_append(GTK_BOX(profile_box), personal_info_label);

    // Name - Use the current user's name
    GtkWidget *name_label = gtk_label_new(NULL);
    char name_text[100];
    snprintf(name_text, sizeof(name_text), "Name: %s %s", current_user.name, current_user.lastname);
    gtk_label_set_text(GTK_LABEL(name_label), name_text);
    gtk_box_append(GTK_BOX(profile_box), name_label);

    // CIN - Use the current user's CIN
    GtkWidget *cin_label = gtk_label_new(NULL);
    char cin_text[100];
    snprintf(cin_text, sizeof(cin_text), "CIN: %s", current_user.cin);
    gtk_label_set_text(GTK_LABEL(cin_label), cin_text);
    gtk_box_append(GTK_BOX(profile_box), cin_label);

    // Birthday - Use the current user's birthday
    GtkWidget *birthday_label = gtk_label_new(NULL);
    char birthday_text[100];
    snprintf(birthday_text, sizeof(birthday_text), "Birthday: %02d/%02d/%04d", current_user.day, current_user.month, current_user.year);
    gtk_label_set_text(GTK_LABEL(birthday_label), birthday_text);
    gtk_box_append(GTK_BOX(profile_box), birthday_label);

    // Contact Information Section
    GtkWidget *contact_info_label = gtk_label_new("<b>Contact Information</b>");
    gtk_label_set_use_markup(GTK_LABEL(contact_info_label), TRUE);
    gtk_box_append(GTK_BOX(profile_box), contact_info_label);

    // Email - Use the current user's email
    GtkWidget *email_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *email_label_widget = gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(email_label_widget), current_user.email);
    GtkWidget *email_edit_button = gtk_button_new_with_label("Edit");
    gtk_widget_add_css_class(email_edit_button, "mini-btn");

    // Create and connect the Edit button for email
    EditPopupData *email_popup_data = g_new(EditPopupData, 1);
    email_popup_data->edit_type = "email";
    email_popup_data->label = email_label_widget;
    g_signal_connect(email_edit_button, "clicked", G_CALLBACK(open_edit_popup), email_popup_data);

    gtk_box_append(GTK_BOX(email_row), email_label_widget);
    gtk_box_append(GTK_BOX(email_row), email_edit_button);
    gtk_box_append(GTK_BOX(profile_box), email_row);

    // Phone Number - Use the current user's phone number
    GtkWidget *phone_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *phone_label_widget = gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(phone_label_widget), current_user.phonenumber);
    GtkWidget *phone_edit_button = gtk_button_new_with_label("Edit");
    gtk_widget_add_css_class(phone_edit_button, "mini-btn");

    // Create and connect the Edit button for phone
    EditPopupData *phone_popup_data = g_new(EditPopupData, 1);
    phone_popup_data->edit_type = "phone";
    phone_popup_data->label = phone_label_widget;
    g_signal_connect(phone_edit_button, "clicked", G_CALLBACK(open_edit_popup), phone_popup_data);

    gtk_box_append(GTK_BOX(phone_row), phone_label_widget);
    gtk_box_append(GTK_BOX(phone_row), phone_edit_button);
    gtk_box_append(GTK_BOX(profile_box), phone_row);

    // Back Button
    GtkWidget *back_button = gtk_button_new_with_label("Back to Client Menu");
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_back1), NULL);
    gtk_box_append(GTK_BOX(profile_box), back_button);

    // Add box to the frame and grid
    gtk_frame_set_child(GTK_FRAME(profile_frame), profile_box);
    gtk_grid_attach(GTK_GRID(profile_grid), profile_frame, 0, 0, 1, 1);

    // Add profile grid to the stack
    gtk_stack_add_named(GTK_STACK(stack), profile_grid, window_name);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
}


void open_car_dispo_window(GtkApplication *app, gpointer user_data) {
    GtkWidget *car_dispo_grid;
    GtkWidget *back_button;
    DIR *dir;
    struct dirent *entry;
    static int model_window_count = 0;  // To track the model window count

    // Create the car availability grid (GtkGrid)
    car_dispo_grid = gtk_grid_new();  // Use GtkGrid to make a grid layout
    gtk_widget_set_halign(car_dispo_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(car_dispo_grid, GTK_ALIGN_CENTER);

    int row = 0, col = 0;

    // Read car files from the directory
    dir = opendir(CAR_DATABASE_DIR);
    if (!dir) {
        g_print("Failed to open directory: %s\n", CAR_DATABASE_DIR);
        return;
    }

    // Iterate through all files in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and .. directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build the file path
        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s\\%s", CAR_DATABASE_DIR, entry->d_name);

        // Open and read car data from the file
        FILE *file = fopen(file_path, "rb");
        if (!file) {
            g_print("Failed to open file: %s\n", file_path);
            continue;
        }

        // Read the car structure
        Car *car = g_new(Car, 1);
        if (fread(car, sizeof(Car), 1, file) == 1 && car->available == 1) {
            // Create a button with the car's image and model name
            GtkWidget *car_button = create_item_button(car->model, car->image_path, TRUE);

            // Connect the button to handle "Request" when clicked
            g_signal_connect(car_button, "clicked", G_CALLBACK(open_model_dispo_details_window), car);

            // Add the car button to the grid
            gtk_grid_attach(GTK_GRID(car_dispo_grid), car_button, col, row, 1, 1);

            // Update grid layout
            col++;
            if (col > 3) {  // 3 columns per row
                col = 0;
                row++;
            }
        } else {
            g_free(car);
        }

        fclose(file);
    }

    closedir(dir);

    // Add a "Back" button to return to the previous view
    back_button = gtk_button_new_with_label("Back");
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_back1), NULL);
    gtk_grid_attach(GTK_GRID(car_dispo_grid), back_button, 0, row + 1, 4, 1);  // Span 4 columns

    // Add the car availability view to the stack
    gtk_stack_add_named(GTK_STACK(stack), car_dispo_grid, "car_dispo_view");

    // Show the car availability view
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "car_dispo_view");
}


static void return_back4(GtkWidget *widget, gpointer data) {
    // Simply go back to the car availability window
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "car_dispo_view");
}


void open_model_dispo_details_window(GtkWidget *widget, gpointer user_data) {
    Car *car = (Car *)user_data;  // Get the car data passed in user_data

    // Create a unique name for this window based on the car model
    char window_name[256];
    snprintf(window_name, sizeof(window_name), "model_%s_1", car->model);

    // Check if the window already exists; if it does, show it instead
    if (gtk_stack_get_child_by_name(GTK_STACK(stack), window_name)) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
        return;
    }

    // Apply inline CSS for styling
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css_data =
        "frame.bordered-box {"
        "    border: 3px solid #555;"
        "    border-radius: 10px;"
        "    padding: 10px;"
        "} "
        "button {"
        "    margin: 10px;"
        "} ";
    gtk_css_provider_load_from_data(provider, css_data, -1);

    GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(stack));
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    // Create the grid for the details page
    GtkWidget *details_grid = gtk_grid_new();
    gtk_widget_set_halign(details_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(details_grid, GTK_ALIGN_CENTER);
    gtk_grid_set_row_spacing(GTK_GRID(details_grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(details_grid), 20);

    // Wrap content in a GtkFrame (bordered container)
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_widget_add_css_class(frame, "bordered-box");
    gtk_widget_set_size_request(frame, 400, 400);

    GtkWidget *frame_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(frame_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(frame_box, GTK_ALIGN_CENTER);

    // 1. Model Image
    GtkWidget *large_image = gtk_image_new_from_file(car->image_path);
    gtk_widget_set_size_request(large_image, 300, 300);
    gtk_box_append(GTK_BOX(frame_box), large_image);

    // 2. Brand Logo
    GtkWidget *brand_logo = gtk_image_new_from_file(find_brand_logo(car->marque));
    gtk_widget_set_size_request(brand_logo, 100, 100);
    gtk_box_append(GTK_BOX(frame_box), brand_logo);

    // 3. Model Name
    GtkWidget *model_name_label = gtk_label_new(NULL);
    char model_name_text[256];
    char *escaped_model = g_markup_escape_text(car->model, -1);
    snprintf(model_name_text, sizeof(model_name_text), "<b>%s</b>", escaped_model);
    gtk_label_set_markup(GTK_LABEL(model_name_label), model_name_text);
    g_free(escaped_model);
    gtk_box_append(GTK_BOX(frame_box), model_name_label);

    // 4. Price Sentence
    GtkWidget *price_label = gtk_label_new(NULL);
    char price_text[128];
    snprintf(price_text, sizeof(price_text), "Rent this car for %.2f MAD per day!", car->price_per_day);
    gtk_label_set_text(GTK_LABEL(price_label), price_text);
    gtk_box_append(GTK_BOX(frame_box), price_label);

    // 5. Request Button
    GtkWidget *request_button = gtk_button_new_with_label("Request");

    // Pass the car and the model window name to the Request Window
    char *model_window_name = g_strdup(window_name);  // Dynamically allocate memory for window name
    RequestWindowData *request_data = g_new(RequestWindowData, 1);
    request_data->car = car;
    strncpy(request_data->model_window_name, window_name, sizeof(request_data->model_window_name) - 1);
    request_data->model_window_name[sizeof(request_data->model_window_name) - 1] = '\0'; // Ensure null-termination

    g_signal_connect(request_button, "clicked", G_CALLBACK(open_request_window), request_data);
    gtk_box_append(GTK_BOX(frame_box), request_button);

    // 6. Back Button
    GtkWidget *back_button = gtk_button_new_with_label("Back to Availability");
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_back4), NULL);
    gtk_box_append(GTK_BOX(frame_box), back_button);

    gtk_frame_set_child(GTK_FRAME(frame), frame_box);
    gtk_grid_attach(GTK_GRID(details_grid), frame, 0, 0, 1, 1);

    // Add the details grid to the stack
    gtk_stack_add_named(GTK_STACK(stack), details_grid, window_name);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
}


void return_to_model_dispo_details(GtkWidget *widget, gpointer user_data) {
    char *model_window_name = (char *)user_data;

    if (gtk_stack_get_child_by_name(GTK_STACK(stack), model_window_name)) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), model_window_name);
    } else {
        g_print("Error: Model details window '%s' not found.\n", model_window_name);
    }

    g_free(model_window_name);
}

static gboolean restrict_to_numeric(GtkEntry *entry, const char *new_text, int new_text_length, int *position, gpointer user_data) {
    // Check if the input is numeric
    for (int i = 0; i < new_text_length; i++) {
        if (!g_ascii_isdigit(new_text[i])) {
            return TRUE; // Block non-numeric input
        }
    }
    return FALSE; // Allow numeric input
}


void open_request_window(GtkWidget *widget, gpointer user_data) {
    static int request_window_count = 0; // Counter to ensure unique names

    RequestWindowData *data = (RequestWindowData *)user_data;
    Car *car = data->car;

    // Generate a unique name for this Request Window based on the car model and counter
    char request_window_name[256];
    snprintf(request_window_name, sizeof(request_window_name), "request_%s_%d", car->model, ++request_window_count);

    // Create the grid layout for the Request Window
    GtkWidget *request_grid = gtk_grid_new();
    gtk_widget_set_halign(request_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(request_grid, GTK_ALIGN_CENTER);
    gtk_grid_set_row_spacing(GTK_GRID(request_grid), 15);
    gtk_grid_set_column_spacing(GTK_GRID(request_grid), 15);

    // Create a frame for the content
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_widget_set_size_request(frame, 400, 500);

    GtkWidget *frame_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(frame_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(frame_box, GTK_ALIGN_CENTER);

    // 1. Car Image
    GtkWidget *large_image = gtk_image_new_from_file(car->image_path);
    gtk_widget_set_size_request(large_image, 250, 250);
    gtk_box_append(GTK_BOX(frame_box), large_image);

    // 2. Car Model Name
    char model_name_text[128];
    snprintf(model_name_text, sizeof(model_name_text), "<b>%s</b>", car->model);
    GtkWidget *model_name_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(model_name_label), model_name_text);
    gtk_box_append(GTK_BOX(frame_box), model_name_label);

    // 3. Input Fields for Rental Duration and Start Date
    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    // Rental Duration Entry
    GtkWidget *duration_label = gtk_label_new("Rental Duration (days):");
    duration_entry = gtk_entry_new();
    gtk_box_append(GTK_BOX(input_box), duration_label);
    gtk_box_append(GTK_BOX(input_box), duration_entry);

    // Rental Start Date (Day, Month, Year)
    GtkWidget *date_label = gtk_label_new("Rental Start Date:");
    GtkWidget *date_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    // Day Entry
    day_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(day_entry), "DD");
    gtk_widget_set_size_request(day_entry, 40, -1);
    gtk_entry_set_max_length(GTK_ENTRY(day_entry), 2); // Limit to 2 characters

    // Month Entry
    month_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(month_entry), "MM");
    gtk_widget_set_size_request(month_entry, 40, -1);
    gtk_entry_set_max_length(GTK_ENTRY(month_entry), 2); // Limit to 2 characters

    // Year Entry
    year_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(year_entry), "YYYY");
    gtk_widget_set_size_request(year_entry, 60, -1);
    gtk_entry_set_max_length(GTK_ENTRY(year_entry), 4); // Limit to 4 characters

    gtk_box_append(GTK_BOX(date_box), day_entry);
    gtk_box_append(GTK_BOX(date_box), month_entry);
    gtk_box_append(GTK_BOX(date_box), year_entry);

    gtk_box_append(GTK_BOX(input_box), date_label);
    gtk_box_append(GTK_BOX(input_box), date_box);

    // Add input box to the frame
    gtk_box_append(GTK_BOX(frame_box), input_box);

    // 4. Confirm Button
    GtkWidget *confirm_button = gtk_button_new_with_label("Confirm Request");
    g_signal_connect(confirm_button, "clicked", G_CALLBACK(on_confirm_request), car);
    gtk_box_append(GTK_BOX(frame_box), confirm_button);

    // 5. Back Button
    GtkWidget *back_button = gtk_button_new_with_label("Back to Model Details");
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_to_model_dispo_details), g_strdup(data->model_window_name));
    gtk_box_append(GTK_BOX(frame_box), back_button);

    gtk_frame_set_child(GTK_FRAME(frame), frame_box);
    gtk_grid_attach(GTK_GRID(request_grid), frame, 0, 0, 1, 1);

    // Add the request grid to the stack with a unique name
    gtk_stack_add_named(GTK_STACK(stack), request_grid, request_window_name);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), request_window_name);
}




// Function to show error pop-ups
void show_error_popup(GtkWidget *widget, const char *title, const char *message) {
    // Get the parent window of the button (or widget that triggered the signal)
    GtkWidget *parent_window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);

    // Create a new window for the error pop-up
    GtkWidget *popup_window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(popup_window), title);
    gtk_window_set_resizable(GTK_WINDOW(popup_window), FALSE);
    gtk_window_set_deletable(GTK_WINDOW(popup_window), FALSE); // Disable the "X" close button

    // Set the pop-up window to appear centered relative to the parent window
    gtk_window_set_modal(GTK_WINDOW(popup_window), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(popup_window), GTK_WINDOW(parent_window));  // Set parent window here
    gtk_window_set_default_size(GTK_WINDOW(popup_window), 300, 150);

    // Add a box to contain content
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(content_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(content_box, GTK_ALIGN_CENTER);

    // Add error message
    GtkWidget *error_label = gtk_label_new(message);
    gtk_box_append(GTK_BOX(content_box), error_label);

    // Add a "Close" button
    GtkWidget *close_button = gtk_button_new_with_label("Close");

    // Signal to close the pop-up window
    g_signal_connect(close_button, "clicked", G_CALLBACK(close_popup), popup_window);
    gtk_box_append(GTK_BOX(content_box), close_button);

    // Set content to the window
    gtk_window_set_child(GTK_WINDOW(popup_window), content_box);

    // Show the pop-up window
    gtk_widget_show(popup_window);
}

// Function to close the pop-up window
void close_popup(GtkWidget *widget, gpointer user_data) {
    GtkWidget *popup_window = GTK_WIDGET(user_data);
    gtk_window_destroy(GTK_WINDOW(popup_window));
}

// Modified on_confirm_request function with error handling
void on_confirm_request(GtkWidget *widget, gpointer user_data) {
    Car *car = (Car *)user_data;

    // Retrieve input values
    const char *duration_text = gtk_editable_get_text(GTK_EDITABLE(duration_entry));
    const char *day_text = gtk_editable_get_text(GTK_EDITABLE(day_entry));
    const char *month_text = gtk_editable_get_text(GTK_EDITABLE(month_entry));
    const char *year_text = gtk_editable_get_text(GTK_EDITABLE(year_entry));

    // Validate if all fields are filled
    if (!duration_text || !*duration_text || !day_text || !month_text || !year_text) {
        show_error_popup(widget, "Error", "All fields must be filled.");
        return;
    }

    // Validate rental duration (1-30 days)
    int rental_duration = atoi(duration_text);
    if (rental_duration <= 0 || rental_duration > 30) {
        show_error_popup(widget, "Error", "Rental duration must be between 1 and 30 days.");
        return;
    }

    // Check if the entered day, month, and year are numeric
    if (!g_ascii_isdigit(day_text[0]) || !g_ascii_isdigit(month_text[0]) || !g_ascii_isdigit(year_text[0])) {
        show_error_popup(widget, "Error", "Day, month, and year must be numeric.");
        return;
    }

    // Convert the day, month, and year to integers
    int day = atoi(day_text);
    int month = atoi(month_text);
    int year = atoi(year_text);

    // Check if the month is valid (1-12)
    if (month < 1 || month > 12) {
        show_error_popup(widget, "Error", "Invalid month. Please enter a month between 1 and 12.");
        return;
    }

    // Check if the day is valid for the given month and year
    int days_in_month = get_days_in_month(month, year);
    if (day < 1 || day > days_in_month) {
        const char *month_name = get_month_name(month);
        char error_message[256];
        snprintf(error_message, sizeof(error_message), "Invalid day for %s. Please enter a valid day.", month_name);
        show_error_popup(widget, "Error", error_message);
        return;
    }

    // Validate that the date is not in the past
    time_t t = time(NULL);
    struct tm *current_time = localtime(&t);
    int current_year = current_time->tm_year + 1900;
    int current_month = current_time->tm_mon + 1;
    int current_day = current_time->tm_mday;

    if (year < current_year || (year == current_year && month < current_month) ||
        (year == current_year && month == current_month && day < current_day)) {
        show_error_popup(widget, "Error", "The rental start date cannot be in the past.");
        return;
    }

    // If all validations pass, proceed with creating the request
    char request_date[11];
    snprintf(request_date, sizeof(request_date), "%04d-%02d-%02d", current_year, current_month, current_day);

    // Calculate the total price for the rental
    float total_price = rental_duration * car->price_per_day;

    // Generate file name for the rental request
    char file_name[512];
    int file_number = 1;
    FILE *file;

    do {
        snprintf(file_name, sizeof(file_name),
                 "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase_req\\%s_request%d.bin",
                 current_user.cin, file_number++);
        file = fopen(file_name, "r");
        if (file) fclose(file);
    } while (file != NULL);

    // Create and save the RentalRequest
    RentalRequest request;
    request.client = current_user;
    request.car = *car;
    strcpy(request.request_date, request_date);
    request.rental_duration = rental_duration;
    request.total_price = total_price;
    request.status = 0;
    request.day = day;
    request.month = month;
    request.year = year;

    file = fopen(file_name, "wb");
    if (!file) {
        show_error_popup(widget, "Error", "Could not create request file.");
        return;
    }
    fwrite(&request, sizeof(RentalRequest), 1, file);
    fclose(file);

    // Show success pop-up
    show_success_popup(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW));
}



void show_success_popup(GtkWidget *parent_window) {
    // Create a new window for the pop-up
    GtkWidget *popup_window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(popup_window), "Congratulations");
    gtk_window_set_resizable(GTK_WINDOW(popup_window), FALSE);
    gtk_window_set_deletable(GTK_WINDOW(popup_window), FALSE); // Disable the "X" close button

    // Set the pop-up window to appear centered relative to the parent
    gtk_window_set_modal(GTK_WINDOW(popup_window), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(popup_window), GTK_WINDOW(parent_window));
    gtk_window_set_default_size(GTK_WINDOW(popup_window), 300, 150);

    // Add a box to contain content
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(content_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(content_box, GTK_ALIGN_CENTER);

    // Add success message
    GtkWidget *success_label = gtk_label_new("Your request has been sent!");
    gtk_box_append(GTK_BOX(content_box), success_label);

    // Add a "Close" button
    GtkWidget *close_button = gtk_button_new_with_label("Close");

    // Signal to close the pop-up window and return to client view
    g_signal_connect(close_button, "clicked", G_CALLBACK(close_popup_and_return), popup_window);
    gtk_box_append(GTK_BOX(content_box), close_button);

    // Set content to the window
    gtk_window_set_child(GTK_WINDOW(popup_window), content_box);

    // Show the pop-up window
    gtk_widget_show(popup_window);
}


void close_popup_and_return(GtkWidget *widget, gpointer user_data) {
    GtkWidget *popup_window = GTK_WIDGET(user_data);

    // Close the pop-up window
    gtk_window_destroy(GTK_WINDOW(popup_window));

    // Return to client view
    return_back1(NULL, NULL);
}

void display_user_requests(GtkWidget *parent_grid, const char *directory_path, const char *user_cin) {
    DIR *dir;
    struct dirent *entry;
    char file_path[512];
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    int row = 0, col = 0;

    dir = opendir(directory_path);
    if (!dir) {
        g_print("Failed to open directory: %s\n", directory_path);
        return;
    }

    g_print("Scanning directory: %s\n", directory_path);

    while ((entry = readdir(dir)) != NULL) {
        g_print("Found file: %s\n", entry->d_name);

        // Check if the file matches user CIN and request pattern
        if (strstr(entry->d_name, user_cin) == NULL || strstr(entry->d_name, "_request") == NULL) {
            g_print("Skipping file: %s\n", entry->d_name);
            continue;
        }

        snprintf(file_path, sizeof(file_path), "%s\\%s", directory_path, entry->d_name);
        g_print("Processing file: %s\n", file_path);

        FILE *file = fopen(file_path, "rb");
        if (!file) {
            g_print("Could not open file: %s\n", file_path);
            continue;
        }

        RentalRequest request;
        if (fread(&request, sizeof(RentalRequest), 1, file) != 1) {
            g_print("Failed to read data from file: %s\n", file_path);
            fclose(file);
            continue;
        }
        fclose(file);

        // Create widgets for the request
        GtkWidget *frame = gtk_frame_new(NULL);
        gtk_widget_set_size_request(frame, 200, 200);

        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        char details[256];
        snprintf(details, sizeof(details), "Car: %s\nDate: %02d-%02d-%04d\nDuration: %d days\nPrice: %.2f MAD",
                 request.car.model, request.day, request.month, request.year,
                 request.rental_duration, request.total_price);
        GtkWidget *info_label = gtk_label_new(details);

        const char *icon_path = get_status_icon_path(request.status);
        GtkWidget *status_image = gtk_image_new_from_file(icon_path);

        GtkWidget *cancel_button = gtk_button_new_with_label("Cancel");
        char *request_path_copy = g_strdup(file_path);
        g_signal_connect(cancel_button, "clicked", G_CALLBACK(on_cancel_request_clicked), request_path_copy);

        gtk_box_append(GTK_BOX(box), status_image);
        gtk_box_append(GTK_BOX(box), info_label);
        gtk_box_append(GTK_BOX(box), cancel_button);

        gtk_frame_set_child(GTK_FRAME(frame), box);
        gtk_grid_attach(GTK_GRID(grid), frame, col, row, 1, 1);

        col++;
        if (col > 2) {
            col = 0;
            row++;
        }
    }
    closedir(dir);

    // Append the grid to the parent box
    gtk_box_append(GTK_BOX(parent_grid), grid);
    gtk_widget_show(grid);
}

void on_cancel_request_clicked(GtkWidget *widget, gpointer user_data) {
    const char *file_path = (const char *)user_data;

    // Delete the request file
    if (remove(file_path) == 0) {
        g_print("Deleted request file: %s\n", file_path);

        // Find the parent frame (the container holding the specific request)
        GtkWidget *parent_frame = gtk_widget_get_ancestor(widget, GTK_TYPE_FRAME);
        if (parent_frame) {
            // Remove the frame (grid of the selected request)
            gtk_widget_unparent(parent_frame);
        }
    } else {
        g_print("Failed to delete file: %s\n", file_path);
    }

    g_free(user_data);  // Free the duplicated file path
}


void on_etat_demande_button_clicked(GtkWidget *widget, gpointer user_data) {
    static int request_window_count = 0;

    // Generate a unique window name
    char window_name[256];
    snprintf(window_name, sizeof(window_name), "etat_demande_%d", ++request_window_count);

    if (gtk_stack_get_child_by_name(GTK_STACK(stack), window_name)) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
        return;
    }

    // Create parent grid
    GtkWidget *parent_grid = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(parent_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(parent_grid, GTK_ALIGN_CENTER);

    // Add a title
    GtkWidget *title = gtk_label_new("<b>Your Requests</b>");
    gtk_label_set_use_markup(GTK_LABEL(title), TRUE);
    gtk_box_append(GTK_BOX(parent_grid), title);

    // Display requests
    display_user_requests(parent_grid, REQUEST_DIR, current_user.cin);

    // Add back button
    GtkWidget *back_button = gtk_button_new_with_label("Back");
    g_signal_connect(back_button, "clicked", G_CALLBACK(return_back1), NULL);
    gtk_box_append(GTK_BOX(parent_grid), back_button);

    // Add to stack and show
    gtk_stack_add_named(GTK_STACK(stack), parent_grid, window_name);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), window_name);
}
