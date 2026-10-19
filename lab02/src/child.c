#include "io.h"
#include "sum.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    if (ignore_sigpipe() == -1) {
        perror("Дочерний процесс: не удалось настроить SIGPIPE");
        return EXIT_FAILURE;
    }
    if (fprintf(stderr, "[Дочерний PID=%ld] Обработка файла\n",
                (long)getpid()) < 0) {
        perror("Дочерний процесс: не удалось вывести сообщение");
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t capacity = 0;
    size_t line_number = 0;
    int result = EXIT_SUCCESS;

    for (;;) {
        ssize_t length = getline(&line, &capacity, stdin);
        if (length < 0) {
            if (!feof(stdin)) {
                perror("Дочерний процесс: не удалось прочитать строку");
                result = EXIT_FAILURE;
            }
            break;
        }
        ++line_number;

        float total = 0.0f;
        enum sum_status status = sum_line(line, (size_t)length, &total);
        if (status == SUM_EMPTY) {
            continue;
        }
        if (status != SUM_OK) {
            fprintf(stderr, "[Дочерний PID=%ld] Строка %zu: %s\n",
                    (long)getpid(), line_number, sum_error(status));
            result = EXIT_FAILURE;
            break;
        }

        if (printf("%.*g\n", FLT_DECIMAL_DIG, (double)total) < 0 ||
            fflush(stdout) == EOF) {
            perror("Дочерний процесс: не удалось записать результат в канал");
            result = EXIT_FAILURE;
            break;
        }
    }

    free(line);
    return result;
}
