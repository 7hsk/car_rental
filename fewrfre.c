#include <gtk/gtk.h>
#include <glib.h>
#include "user.h"

User* read_all_user_data(int *num_users) {
    GDir *dir = g_dir_open("C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase", 0, NULL);
    if (dir == NULL) {
        g_print("Error opening directory!\n");
        *num_users = 0;
        return NULL;
    }

    const gchar *filename;
    User *users = NULL;
    *num_users = 0;

    while ((filename = g_dir_read_name(dir)) != NULL) {
        if (g_str_has_suffix(filename, ".bin")) {
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase\\%s", filename);

            FILE *file = fopen(file_path, "rb");
            if (file == NULL) {
                g_print("Error opening file at %s: %s\n", file_path, strerror(errno));
                continue;
            }

            users = g_realloc(users, (*num_users + 1) * sizeof(User));
            fread(&users[*num_users], sizeof(User), 1, file);
            fclose(file);
            (*num_users)++;
        }
    }

    g_dir_close(dir);

    // Count the number of existing .txt files in the DataBase_txt directory
    GDir *txt_dir = g_dir_open("C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase_txt", 0, NULL);
    if (txt_dir == NULL) {
        g_print("Error opening directory!\n");
        return users;
    }

    int txt_file_count = 0;
    while ((filename = g_dir_read_name(txt_dir)) != NULL) {
        if (g_str_has_suffix(filename, ".txt")) {
            txt_file_count++;
        }
    }
    g_dir_close(txt_dir);

    // Create a new .txt file with the name user-data%number_of_files.txt
    char txt_file_path[256];
    snprintf(txt_file_path, sizeof(txt_file_path), "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase_txt\\user-data%d.txt", txt_file_count);

    FILE *txt_file = fopen(txt_file_path, "w");
    if (txt_file == NULL) {
        g_print("Error creating file at %s: %s\n", txt_file_path, strerror(errno));
        return users;
    }

    // Write user data to the .txt file
    for (int i = 0; i < *num_users; i++) {
        fprintf(txt_file, "Name: %s\n", users[i].name);
        fprintf(txt_file, "Last Name: %s\n", users[i].lastname);
        fprintf(txt_file, "Phone Number: %s\n", users[i].phonenumber);
        fprintf(txt_file, "Email: %s\n", users[i].email);
        fprintf(txt_file, "CIN: %s\n", users[i].cin);
        fprintf(txt_file, "Birthday: %02d/%02d/%04d\n", users[i].day, users[i].month, users[i].year);
        fprintf(txt_file, "Role: %s\n", users[i].role);
        fprintf(txt_file, "\n");
    }

    fclose(txt_file);
    return users;
}