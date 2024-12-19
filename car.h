// car.h
#ifndef CAR_H
#define CAR_H

#include <glib.h>

typedef struct {
    char model[50];          // The model name (e.g., "Civic")
    char marque[50];         // The brand name (e.g., "Honda")
    int available;           // Availability status (1 = available, 0 = not available)
    float price_per_day;     // Rental price per day
    char image_path[256];    // Image path for the car model
} Car;

// Define the structure for a Car Brand (Marque)
typedef struct {
    char marque[50];         // The brand name (e.g., "Honda")
    char logo_path[256];     // Path to the brand logo image
    GList *models;           // A list of Car* for this brand
} MarqueCatalogue;

typedef struct {
    char brand_name[50];   // Brand name (e.g., "Honda")
    char logo_path[256];   // Path to the logo (e.g., "path/to/logo.png")
} BrandData;
// Declare extern variables
extern GList *catalogue;  // Declare catalogue as extern

// Function prototypes
void add_car_to_catalogue(const char *marque, const char *model, int available, float price_per_day, const char *image_path);
void populate_catalogue_with_brands_and_models(void);
GtkWidget* create_item_button(const char *name, const char *logo_path, gboolean is_brand);

#endif // CAR_H
