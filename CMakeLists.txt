cmake_minimum_required(VERSION 3.10)
project(GraphProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Собираем все исходные файлы
file(GLOB SOURCES
        "*.cpp"
        "config/*.cpp"
        "graph/*.cpp"
        "io/*.cpp"
        "utils/*.cpp"
        "algorithms/sp/*.cpp"   # Добавлено
        "algorithms/mst/*.cpp"  # Добавлено
)

# Создаем исполняемый файл
add_executable(GraphProject ${SOURCES})

# Для Windows
if(WIN32)
    target_link_libraries(GraphProject -lole32)
endif()