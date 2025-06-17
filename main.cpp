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

    // Отладочная информация о путях
    char cwd[1024];
    if (GETCWD(cwd, sizeof(cwd))) {
        printf("Current working directory: %s\n", cwd);
    }

    std::string resolved = resolve_path(argv[1]);
    printf("Resolved config path: %s\n", resolved.c_str());

    // Инициализация случайных чисел
    init_random();

    // Создание конфигурации
    File_config file_cfg;
    memset(&file_cfg, 0, sizeof(File_config));

    // Чтение конфигурации
    read_config_file(resolved.c_str(), &file_cfg);
    print_config_file(&file_cfg);

    // Основная конфигурация
    Config cfg;
    memset(&cfg, 0, sizeof(Config));
    cfg.start_vertex = file_cfg.start_vertex;
    cfg.alg_type = file_cfg.alg_type;

    // Упрощенная логика выполнения
    if (file_cfg.file_name) {
        run_config_file_load(&file_cfg, &cfg);
    } else {
        fprintf(stderr, "Random graph generation not implemented\n");
    }

    // Очистка
    free_config(&cfg);
    free_config_file(&file_cfg);

    return 0;
}