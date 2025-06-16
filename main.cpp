#include "config/configuration.h"
#include "io/file_io.h"
#include "utils/random.h"
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <string>

#ifdef _WIN32
#include <direct.h>
#define GETCWD _getcwd
#else
#include <unistd.h>
#define GETCWD getcwd
#endif

// Функция для проверки существования файла
bool file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        printf("Usage: %s <config_file>\n", argv[0]);
        return 1;
    }

    // Получаем текущую рабочую директорию
    char cwd[1024];
    if (!GETCWD(cwd, sizeof(cwd))) {
        perror("getcwd() error");
        return 1;
    }
    printf("Current working directory: %s\n", cwd);

    std::string config_path = argv[1];
    printf("Trying to open config file: %s\n", config_path.c_str());

    // Проверка 1: оригинальный путь
    if (!file_exists(config_path.c_str())) {
        printf("File not found, trying ../%s\n", argv[1]);

        // Проверка 2: в родительской директории
        std::string parent_path = "../" + config_path;
        if (file_exists(parent_path.c_str())) {
            config_path = parent_path;
        }
        // Проверка 3: в корне проекта
        else {
            std::string root_path = "../../" + config_path;
            if (file_exists(root_path.c_str())) {
                config_path = root_path;
            }
            else {
                fprintf(stderr, "Error: Config file not found\n");
                fprintf(stderr, "Searched in:\n- %s\n- %s\n- %s\n",
                        argv[1], parent_path.c_str(), root_path.c_str());
                return 1;
            }
        }
    }

    printf("Using config file: %s\n", config_path.c_str());

    // Инициализация случайных чисел
    init_random();

    // Создание конфигурации
    File_config* file_cfg = (File_config*)malloc(sizeof(File_config));
    memset(file_cfg, 0, sizeof(File_config));

    // Чтение конфигурации
    read_config_file(config_path.c_str(), file_cfg);
    print_config_file(file_cfg);

    // Основная конфигурация
    Config cfg;
    memset(&cfg, 0, sizeof(Config));

    // Выполнение в зависимости от конфигурации
    if (file_cfg->file_name) {
        run_config_file_load(file_cfg, &cfg);
    } else {
        run_config_file_var(file_cfg, &cfg);
    }

    // Очистка
    free_config(&cfg);
    free_config_file(file_cfg);

    return 0;
}