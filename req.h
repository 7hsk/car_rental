#ifndef REQ_H
#define REQ_H

typedef struct {
    User client;               // Client information
    Car car;                   // Car information
    char request_date[20];     // Date and time of the request in "YYYY-MM-DD HH:MM:SS" format
    char start_date[11];       // Rental start date in "YYYY-MM-DD" format
    int rental_duration;       // Duration of rental in days
    float total_price;         // Total price for the rental
    int status;                // 0 = Pending, 1 = Approved, 2 = Rejected, 3 = Canceled
    char status_message[100];  // Detailed message about the request status (e.g., reasons for rejection)
    int day;                   // Start day of rental
    int month;                 // Start month of rental
    int year;                  // Start year of rental
    int end_day;               // End day of rental
    int end_month;             // End month of rental
    int end_year;              // End year of rental
    char notification[200];    // Notification message for the client
} RentalRequest;

typedef struct {
    char cin[20];              // Client CIN
    char car_model[50];        // Car model
    char dates[365][11];       // Rental dates in "YYYY-MM-DD" format
    int status;                // Same status as the request (0 = Pending, 1 = Approved, etc.)
    int rental_duration;       // Duration of the rental
    int request_id;            // Associated request ID
} RentingDates;

// Rental Request Queue (for storing multiple rental requests)
typedef struct {
    RentalRequest *requests;  // Array of rental requests
    int count;                // Number of requests in the queue
    int capacity;             // Maximum capacity of the queue
} RentalRequestQueue;

#endif //REQ_H
