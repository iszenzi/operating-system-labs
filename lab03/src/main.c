#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--help") == 0)
    {
        if (print_usage(argv[0]) < 0 || fflush(stdout) == EOF)
        {
            perror("Не удалось вывести справку");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    struct game_options options;
    if (parse_options(argc, argv, &options) != 0)
    {
        fprintf(stderr, "Некорректные аргументы. Справка: %s --help\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct timespec start;
    struct timespec finish;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
    {
        perror("Не удалось измерить время начала");
        return EXIT_FAILURE;
    }
    struct game_result result;
    if (simulate_game(&options, &result) != 0)
    {
        return EXIT_FAILURE;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &finish) == -1)
    {
        perror("Не удалось измерить время завершения");
        return EXIT_FAILURE;
    }
    double seconds = (double)(finish.tv_sec - start.tv_sec);
    seconds += (double)(finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    int printed = printf("Экспериментов: %llu\nПотоков: %u\n"
                         "Побед игрока 1: %llu\nПобед игрока 2: %llu\nНичьих: %llu\n"
                         "Шанс игрока 1: %.6f\nШанс игрока 2: %.6f\n"
                         "Шанс ничьей: %.6f\nВремя, с: %.9f\n",
                         (unsigned long long)options.experiments, result.threads_used,
                         (unsigned long long)result.wins1, (unsigned long long)result.wins2,
                         (unsigned long long)result.draws,
                         (double)result.wins1 / (double)options.experiments,
                         (double)result.wins2 / (double)options.experiments,
                         (double)result.draws / (double)options.experiments, seconds);
    if (printed < 0 || fflush(stdout) == EOF)
    {
        perror("Не удалось вывести результат");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
