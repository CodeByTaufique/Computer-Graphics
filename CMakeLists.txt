cmake_minimum_required(VERSION 3.20)
project(ComputerGraphicsLab VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 1. Mac-specific paths for Homebrew
if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif()

# 2. Find necessary packages
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

# 3. Define the library list based on the platform
# This saves you from typing all the frameworks over and over again
if(APPLE)
    set(COMMON_LIBS
            ${OPENGL_LIBRARIES}
            ${GLUT_LIBRARIES}
            GLEW::GLEW
            glfw
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
    )
else()
    set(COMMON_LIBS
            ${OPENGL_LIBRARIES}
            ${GLUT_LIBRARIES}
            GLEW::GLEW
            glfw
            winmm
    )
endif()

# ---------------------------------------------------------
# 4. Lab Assignments (Add each new .cpp file here)
# ---------------------------------------------------------

# Lab 1: Rotating Triangle
# Assuming you named the rotating triangle file 'triangle.cpp'
add_executable(Try1 main.cpp)
target_link_libraries(Try1 PRIVATE ${COMMON_LIBS})

# Lab 2: Cross Platform Test (Teal Screen)
# Assuming you named the teal screen file 'lab2.cpp'
add_executable(Try2 try.cpp)
target_link_libraries(Try2 PRIVATE ${COMMON_LIBS})

add_executable(Try3 try1.cpp)
target_link_libraries(Try3 PRIVATE ${COMMON_LIBS})

add_executable(Lab1 Lab1.cpp)
target_link_libraries(Lab1 PRIVATE ${COMMON_LIBS})

add_executable(House house.cpp)
target_link_libraries(House PRIVATE ${COMMON_LIBS})

add_executable(Boat boat.cpp)
target_link_libraries(Boat PRIVATE ${COMMON_LIBS})

add_executable(Stars fourStars.cpp)
target_link_libraries(Stars PRIVATE ${COMMON_LIBS})

add_executable(Lab2 Lab2.cpp)
target_link_libraries(Lab2 PRIVATE ${COMMON_LIBS})

add_executable(Algo1 Bresenham.cpp)
target_link_libraries(Algo1 PRIVATE ${COMMON_LIBS})

add_executable(Algo2 DDA.cpp)
target_link_libraries(Algo2 PRIVATE ${COMMON_LIBS})

add_executable(Lab3 Lab3.cpp)
target_link_libraries(Lab3 PRIVATE ${COMMON_LIBS})

add_executable(Circle midCircle.cpp)
target_link_libraries(Circle PRIVATE ${COMMON_LIBS})

add_executable(Lab4 Lab4.cpp)
target_link_libraries(Lab4 PRIVATE ${COMMON_LIBS})

add_executable(2d 2dTransform.cpp)
target_link_libraries(2d PRIVATE ${COMMON_LIBS})

add_executable(Lab5 Lab5.cpp)
target_link_libraries(Lab5 PRIVATE ${COMMON_LIBS})

add_executable(Flag flag.cpp)
target_link_libraries(Flag PRIVATE ${COMMON_LIBS})

add_executable(Game Game.cpp)
target_link_libraries(Game PRIVATE ${COMMON_LIBS})

add_executable(A check.cpp)
target_link_libraries(A PRIVATE ${COMMON_LIBS})

add_executable(P p.cpp)
target_link_libraries(P PRIVATE ${COMMON_LIBS})

add_executable(Car Car.cpp)
target_link_libraries(Car PRIVATE ${COMMON_LIBS})

add_executable(Flower Flower.cpp)
target_link_libraries(Flower PRIVATE ${COMMON_LIBS})

add_executable(House2.0 House2.0.cpp)
target_link_libraries(House2.0 PRIVATE ${COMMON_LIBS})

add_executable(Lab6 Lab6.cpp)
target_link_libraries(Lab6 PRIVATE ${COMMON_LIBS})

add_executable(Metro Metro.cpp)
target_link_libraries(Metro PRIVATE ${COMMON_LIBS})

add_executable(Lab7 Lab7.cpp)
target_link_libraries(Lab7 PRIVATE ${COMMON_LIBS})

add_executable(Metro2 Metro2.cpp)
target_link_libraries(Metro2 PRIVATE ${COMMON_LIBS})

add_executable(animation Animation.cpp)
target_link_libraries(animation PRIVATE ${COMMON_LIBS})

add_executable(Lab8 Lab8.cpp)
target_link_libraries(Lab8 PRIVATE ${COMMON_LIBS})

add_executable(Lab9 Lab9.cpp)
target_link_libraries(Lab9 PRIVATE ${COMMON_LIBS})