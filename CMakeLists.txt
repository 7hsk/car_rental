cmake_minimum_required(VERSION 3.10)
project(car_rental C)

# Set C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

# Find GTK4 using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)

# Check if GTK4 was found
if (NOT GTK_FOUND)
    message(FATAL_ERROR "GTK4 not found. Please install it.")
endif()

# Find and link cJSON library
find_package(cJSON REQUIRED)

# Add your source files
set(SOURCE_FILES
        main.c
        signup.c
        utils.c
        utils.h
        login.c
        signup.h
        login.h
        user.h
        fewrfre.c
        fewrfre.h
        admin.c
        admin.h
        client.c
        client.h
        car.h
        car.c
        user.c
        req.h
        mod.c
        mod.h
)

# Define the executable and link it with the necessary libraries
add_executable(car_rental ${SOURCE_FILES})

# Link libraries
target_link_libraries(car_rental PRIVATE cJSON ${GTK_LIBRARIES})

# Include GTK include directories
include_directories(${GTK_INCLUDE_DIRS})

# Link additional GTK flags for compatibility
target_compile_options(car_rental PRIVATE ${GTK_CFLAGS_OTHER})
target_link_options(car_rental PRIVATE ${GTK_LDFLAGS_OTHER})

# Additional settings for UCRT
set_target_properties(car_rental PROPERTIES
        LINK_FLAGS "-Wl,--enable-auto-import"
)