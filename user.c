#include "user.h"
#include <glib.h>
#include <stdio.h>

User current_user;  // Define current_user here

void update_user_data(const User *user) {
    char file_path[100];
    snprintf(file_path, sizeof(file_path), "C:/Users/mouad/CLionProjects/car_rental/DataBase/%s.bin", user->cin);

    FILE *file = fopen(file_path, "wb");
    if (file) {
        fwrite(user, sizeof(User), 1, file);
        fclose(file);
    } else {
        g_print("Error: Unable to update user data\n");
    }
}


gboolean save_user_data(const char *cin) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "C:\\Users\\mouad\\CLionProjects\\car_rental\\DataBase\\%s.bin", cin);

    FILE *file = fopen(file_path, "wb");
    if (!file) {
        g_print("Error: Unable to open file %s for writing\n", file_path);
        return FALSE;
    }

    fwrite(&current_user, sizeof(User), 1, file);
    fclose(file);
    return TRUE;
}
