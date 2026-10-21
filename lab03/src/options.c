#include "game.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_number(const char *text, uint64_t maximum, uint64_t *result)
{
    if (*text == '\0')
    {
        return -1;
    }
    for (const char *cursor = text; *cursor != '\0'; ++cursor)
    {
        if (*cursor < '0' || *cursor > '9')
        {
            return -1;
        }
    }
    char *end = NULL;
    errno = 0;
    unsigned long long value = strtoull(text, &end, 10);
    if (errno == ERANGE || *end != '\0' || value > maximum)
    {
        return -1;
    }
    *result = (uint64_t)value;
    return 0;
}

int print_usage(const char *program)
{
    return printf("Использование: %s --threads N [--seed S] [--quiet] K ТУР ОЧКИ1 ОЧКИ2 ОПЫТЫ\n"
           "ТУР — число полностью завершённых туров (от 0 до K).\n"
           "В каждом оставшемся туре каждый игрок бросает две кости.\n"
           "N: 1..1024, включая основной поток; K: 1..1000000000.\n"
           "ОПЫТЫ: 1..1000000000000. Очки: от 2*ТУР до 12*ТУР.\n"
           "S: целое 0..18446744073709551615, по умолчанию 1.\n"
           "--quiet отключает сообщения с PID/TID в stderr.\n", program);
}

int parse_options(int argc, char **argv, struct game_options *options)
{
    uint64_t values[5] = {0};
    const uint64_t limits[5] = {1000000000, 1000000000,
                              12000000000, 12000000000, 1000000000000};
    int count = 0;
    int has_threads = 0;
    int has_seed = 0;
    *options = (struct game_options){0};
    options->seed = 1;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--quiet") == 0)
        {
            options->quiet = 1;
        }
        else if (strcmp(argv[i], "--threads") == 0 || strcmp(argv[i], "--seed") == 0)
        {
            int is_threads = strcmp(argv[i], "--threads") == 0;
            uint64_t value;
            uint64_t maximum = UINT64_MAX;
            if (is_threads)
            {
                maximum = 1024;
            }
            if (i + 1 == argc || read_number(argv[i + 1], maximum, &value) != 0)
            {
                return -1;
            }
            ++i;
            if (is_threads)
            {
                if (has_threads || value == 0)
                {
                    return -1;
                }
                has_threads = 1;
                options->threads = (unsigned int)value;
            }
            else
            {
                if (has_seed)
                {
                    return -1;
                }
                has_seed = 1;
                options->seed = value;
            }
        }
        else
        {
            if (count == 5 || read_number(argv[i], limits[count], &values[count]) != 0)
            {
                return -1;
            }
            ++count;
        }
    }
    if (!has_threads || count != 5 || values[0] == 0 || values[4] == 0)
    {
        return -1;
    }
    options->rounds = values[0];
    options->completed = values[1];
    options->score1 = values[2];
    options->score2 = values[3];
    options->experiments = values[4];
    if (options->completed > options->rounds ||
        options->score1 < 2 * options->completed ||
        options->score1 > 12 * options->completed ||
        options->score2 < 2 * options->completed ||
        options->score2 > 12 * options->completed)
    {
        return -1;
    }
    return 0;
}
