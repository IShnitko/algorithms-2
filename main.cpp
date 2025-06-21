#include "config/configuration.h"
#include "io/file_io.h"
#include "utils/path_utils.h"
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <string>

#include "utils/random.h"

#ifdef _WIN32
#include <direct.h>
#define GETCWD _getcwd
#else
#include <unistd.h>
#define GETCWD getcwd
#endif

bool file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <config_file>\n", argv[0]);
        return 1;
    }

    // Инициализация случайных чисел
    init_random();

    // Отладочная информация о путях
    char cwd[1024];
    if (GETCWD(cwd, sizeof(cwd))) {
        printf("Current working directory: %s\n", cwd);
    }

    std::string resolved = resolve_path(argv[1]);
    printf("Resolved config path: %s\n", resolved.c_str());

    // Создание конфигурации
    File_config file_cfg;
    memset(&file_cfg, 0, sizeof(File_config));

    // Чтение конфигурации
    read_config_file(resolved.c_str(), &file_cfg);
    print_config_file(&file_cfg);

    // Основная конфигурация
    Config cfg;
    memset(&cfg, 0, sizeof(Config));

    // Упрощенная логика выполнения
    if (file_cfg.file_name) {
        // Загрузка из файла
        run_config_file_load(&file_cfg, &cfg);
    } else if (file_cfg.num_v > 0 && file_cfg.density > 0) {
        // Генерация случайного графа
        run_config_file_var(&file_cfg, &cfg);
    } else {
        fprintf(stderr, "Error: insufficient configuration parameters\n");
        fprintf(stderr, "Either specify file_name or both num_v and density\n");
        return 1;
    }

    // Очистка
    free_config(&cfg);
    free_config_file(&file_cfg);

    return 0;
}