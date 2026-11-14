#include "io.h"
#include "mapping.h"
#include "sum.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    char *line = NULL;
    size_t capacity = 0;
    size_t line_number = 0;
    off_t output_length = 0;
    int result = EXIT_SUCCESS;

    if (fprintf(stderr, "[Дочерний PID=%ld] Суммирование строк через mmap\n",
                (long)getpid()) < 0)
    {
        perror("Не удалось вывести сообщение дочернего процесса");
        return EXIT_FAILURE;
    }

    for (;;)
    {
        ssize_t length = getline(&line, &capacity, stdin);
        if (length < 0)
        {
            if (!feof(stdin))
            {
                perror("Дочерний процесс: не удалось прочитать строку");
                result = EXIT_FAILURE;
            }
            break;
        }
        ++line_number;

        float total = 0.0f;
        enum sum_status status = sum_line(line, (size_t)length, &total);
        if (status == SUM_EMPTY)
        {
            continue;
        }
        if (status != SUM_OK)
        {
            fprintf(stderr, "[Дочерний PID=%ld] Строка %zu: %s\n",
                    (long)getpid(), line_number, sum_error(status));
            result = EXIT_FAILURE;
            break;
        }
        if (append_sum(STDOUT_FILENO, &output_length, total) == -1)
        {
            result = EXIT_FAILURE;
            break;
        }
    }

    if (ftruncate(STDOUT_FILENO, output_length) == -1)
    {
        perror("Дочерний процесс: не удалось установить итоговый размер");
        result = EXIT_FAILURE;
    }
    free(line);
    int output = STDOUT_FILENO;
    if (close_fd(&output) == -1)
    {
        result = EXIT_FAILURE;
    }
    if (fclose(stdin) == EOF)
    {
        perror("Дочерний процесс: не удалось закрыть входной файл");
        result = EXIT_FAILURE;
    }
    return result;
}
