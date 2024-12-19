#include <gtk/gtk.h>
#include "utils.h"
#include <ctype.h>
#include <time.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include "client.h"
#include "login.h"
#include "signup.h"

extern GtkWidget *stack;
extern GList *window_stack;

const char *month_names[] = {
    "",        // Index 0 (not used, to match 1-based indexing)
    "January",  // Index 1
    "February", // Index 2
    "March",    // Index 3
    "April",    // Index 4
    "May",      // Index 5
    "June",     // Index 6
    "July",     // Index 7
    "August",   // Index 8
    "September",// Index 9
    "October",  // Index 10
    "November", // Index 11
    "December"  // Index 12
};

void on_quit_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *window = GTK_WIDGET(data);
    gtk_window_close(GTK_WINDOW(window));
}

gboolean is_valid_name(const char *name) {
    int length = strlen(name);
    if (length < 3 || length > 20) {
        return FALSE;
    }
    for (int i = 0; name[i] != '\0'; i++) {
        if (!isalpha(name[i])) {
            return FALSE;
        }
    }
    return TRUE;
}

gboolean is_valid_email(const char *email) {
    int length = strlen(email);
    gboolean has_at = FALSE, has_dot = FALSE;
    int at_index = -1, dot_index = -1;

    for (int i = 0; i < length; i++) {
        if (email[i] == '@') {
            if (has_at) return FALSE; // Multiple '@' characters
            has_at = TRUE;
            at_index = i;
        }
        if (email[i] == '.') {
            has_dot = TRUE;
            dot_index = i;
        }
    }

    // Ensure '@' comes before '.' and there are characters before and after both
    return has_at && has_dot && at_index < dot_index && at_index > 0 && dot_index < length - 1;
}

gboolean is_strong_password(const char *password) {
    int length = strlen(password);
    gboolean has_upper = FALSE, has_lower = FALSE, has_digit = FALSE, has_special = FALSE;

    if (length < 8) return FALSE;

    for (int i = 0; i < length; i++) {
        if (isupper(password[i])) has_upper = TRUE;
        if (islower(password[i])) has_lower = TRUE;
        if (isdigit(password[i])) has_digit = TRUE;
        if (ispunct(password[i])) has_special = TRUE;
    }

    return has_upper && has_lower && has_digit && has_special;
}

gboolean is_older_than_21(int year, int month, int day) {
    time_t now = time(NULL);
    struct tm current = *localtime(&now);

    int age = current.tm_year + 1900 - year;
    if (current.tm_mon + 1 < month || (current.tm_mon + 1 == month && current.tm_mday < day)) {
        age--;
    }

    return age >= 21;
}

// Function to return to the previous window in the stack
void return_back(GtkWidget *widget, gpointer data) {
    if (window_stack != NULL) {
        // Remove the current window from the stack
        window_stack = g_list_remove(window_stack, g_list_last(window_stack)->data);

        // If there is a previous window in the stack, show it
        if (window_stack != NULL) {
            GtkWidget *previous_view = GTK_WIDGET(g_list_last(window_stack)->data);
            gtk_stack_set_visible_child(GTK_STACK(stack), previous_view);
        } else {
            // If no more windows, go back to the main view
            gtk_stack_set_visible_child_name(GTK_STACK(stack), "main_view");
        }
    }
}


void return_to_main_menu(GtkWidget *widget, gpointer data) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "main_view");
}

#include "admin.h"  // Add this include for the admin window function

void proceed_to_next_window(const char *role) {
    if (strcmp(role, "Client") == 0) {
        GtkWidget *client_view = gtk_stack_get_child_by_name(GTK_STACK(stack), "client_view");
        if (client_view == NULL) {
            open_client_window(gtk_window_get_application(GTK_WINDOW(gtk_widget_get_native(stack))), NULL);
        } else {
            gtk_stack_set_visible_child(GTK_STACK(stack), client_view);
        }
    } else if (strcmp(role, "Admin") == 0) {
        // Open the Admin window
        open_admin_window(NULL, NULL);
    } else {
        g_print("Unknown role: %s\n", role);
    }
}

void show_info_message(GtkWidget *parent, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(parent),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s", message
    );

    gtk_window_set_title(GTK_WINDOW(dialog), "Information");
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_window_present(GTK_WINDOW(dialog));
}



void show_error_message(GtkWidget *parent, const char *message) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Error",
        GTK_WINDOW(parent),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Close", GTK_RESPONSE_CLOSE
    );

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), g_strdup_printf("<b>Error:</b> %s", message));
    gtk_box_append(GTK_BOX(content_area), label);

    gtk_widget_show(dialog);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), dialog);
}

char* hash_password(const char *password) {
    GChecksum *checksum = g_checksum_new(G_CHECKSUM_SHA256);
    g_checksum_update(checksum, (const guchar *)password, strlen(password));
    const gchar *hash = g_checksum_get_string(checksum);
    char *hashed_password = g_strdup(hash);
    g_checksum_free(checksum);
    return hashed_password;
}

void show_main_menu(GtkApplication *app) {
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *label;
    GtkWidget *login_button, *sign_up_button, *quit_button;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "CAR RENTAL SVR");

    // Apply Fullscreen-Windowed Behavior Globally
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE); // Disable resizing
    gtk_window_fullscreen(GTK_WINDOW(window));           // Set to fullscreen

    // Stack and layout setup
    stack = gtk_stack_new();
    gtk_window_set_child(GTK_WINDOW(window), stack);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    label = gtk_label_new("CAR RENTAL SVR");
    gtk_box_append(GTK_BOX(box), label);

    login_button = gtk_button_new_with_label("LOGIN");
    g_signal_connect(login_button, "clicked", G_CALLBACK(open_login_window), NULL);
    gtk_box_append(GTK_BOX(box), login_button);

    sign_up_button = gtk_button_new_with_label("SIGN UP");
    g_signal_connect(sign_up_button, "clicked", G_CALLBACK(open_signup_window), NULL);
    gtk_box_append(GTK_BOX(box), sign_up_button);

    quit_button = gtk_button_new_with_label("QUIT");
    g_signal_connect(quit_button, "clicked", G_CALLBACK(on_quit_clicked), window);
    gtk_box_append(GTK_BOX(box), quit_button);

    gtk_stack_add_named(GTK_STACK(stack), box, "main_view");
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "main_view");

    gtk_window_present(GTK_WINDOW(window));
}




gboolean is_valid_moroccan_phone(const char *phone) {
    int length = strlen(phone);

    // Check if it starts with 06 or 07 and is exactly 10 digits
    if (length == 10 && (strncmp(phone, "06", 2) == 0 || strncmp(phone, "07", 2) == 0)) {
        for (int i = 0; i < length; i++) {
            if (!isdigit(phone[i])) {
                return FALSE; // Non-digit character found
            }
        }
        return TRUE; // Valid phone number
    }
    return FALSE; // Invalid phone number
}

int get_days_in_month(int month, int year) {
    // List of days for each month (Jan - Dec), assuming non-leap year for February
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Adjust February days for leap years
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }

    return days_in_month[month - 1]; // Month is 1-indexed
}

int get_month_from_name(const char *month_name) {
    for (int i = 1; i <= 12; i++) {
        if (g_strcmp0(month_names[i], month_name) == 0) {
            return i;  // Return the month number
        }
    }
    return -1;  // Return -1 if no match found
}


int is_leap_year(int year) {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        return 1; // It's a leap year
    }
    return 0; // Not a leap year
}


const char* get_month_name(int month) {
    const char *month_names[] = {
        "",         // 0 (not used)
        "January",   // 1
        "February",  // 2
        "March",     // 3
        "April",     // 4
        "May",       // 5
        "June",      // 6
        "July",      // 7
        "August",    // 8
        "September", // 9
        "October",   // 10
        "November",  // 11
        "December"   // 12
    };
    return (month >= 1 && month <= 12) ? month_names[month] : "Invalid Month";
}
