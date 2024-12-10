cmake_minimum_required(VERSION 3.14)

project(mortal-engine)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(mortal-engine)
add_subdirectory(mortal-engine-test)
