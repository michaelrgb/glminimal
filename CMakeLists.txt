project(glminimal CXX C)
add_executable(glminimal glminimal.cpp)
target_link_libraries(glminimal GL X11)
