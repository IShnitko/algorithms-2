cmake_minimum_required(VERSION 3.10)
project(GraphProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Все исходные файлы
file(GLOB SOURCES
        "*.cpp"
        "graph/*.cpp"
        "algorithms/*/*.cpp"
        "io/*.cpp"
        "utils/*.cpp"
)

# Создаем исполняемый файл
add_executable(GraphProject ${SOURCES})

# Для Windows
if(WIN32)
    target_link_libraries(GraphProject -lole32)
endif()