cmake_minimum_required(VERSION 3.10)

# Создаем исполняемый файл для тестов
add_executable(lab4_tests test_ipc.cpp)

if(MINGW)
    target_link_options(lab4_tests PRIVATE "-static")
endif()
