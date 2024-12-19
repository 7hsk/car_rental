#ifndef USER_H
#define USER_H

#include <glib.h>
typedef struct {
    char name[50];
    char lastname[50];
    char phonenumber[20];
    char email[50];
    char cin[20];
    int day;
    int month;
    int year;
    char password[65]; // SHA-256 hash length is 64 characters + null terminator
    char role[10];
} User;

// Declare extern variables and functions
extern User current_user;   // Declaring the current_user variable
gboolean save_user_data(const char *cin);
void update_user_data(const User *user);

#endif // USER_H
