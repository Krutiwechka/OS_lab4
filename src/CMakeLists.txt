cmake_minimum_required(VERSION 3.10)

add_executable(receiver receiver.cpp)
add_executable(sender sender.cpp)

if(MINGW)
    target_link_options(receiver PRIVATE "-static")
    target_link_options(sender PRIVATE "-static")
endif()
