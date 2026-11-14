#include "io.h"
#include "mapping.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void run_child(int input, int output)
{
    if (dup2(input, STDIN_FILENO) == -1)
    {
        perror("Дочерний процесс: не удалось перенаправить потоки");
        _exit(EXIT_FAILURE);
    }
    if (input != STDIN_FILENO && close_fd(&input) == -1)
    {
        _exit(EXIT_FAILURE);
    }
    if (dup2(output, STDOUT_FILENO) == -1)
    {
        perror("Дочерний процесс: не удалось перенаправить вывод");
        _exit(EXIT_FAILURE);
    }
    if (output != STDOUT_FILENO && close_fd(&output) == -1)
    {
        _exit(EXIT_FAILURE);
    }
    execl("./build/child", "child", (char *)NULL);
    perror("Дочерний процесс: не удалось запустить build/child");
    _exit(EXIT_FAILURE);
}

static int wait_for_child(pid_t child_pid, int *status)
{
    pid_t waited;
    do
    {
        waited = waitpid(child_pid, status, 0);
    } while (waited == -1 && errno == EINTR);
    if (waited == -1)
    {
        perror("Родитель: не удалось дождаться дочернего процесса");
        return -1;
    }
    return 0;
}

int main(void)
{
    int input = -1;
    int output = -1;
    pid_t child_pid = -1;
    char *filename = NULL;
    size_t capacity = 0;
    int result = EXIT_FAILURE;
    char temporary[] = "/tmp/lab04-XXXXXX";
    int temporary_exists = 0;

    if (ignore_sigpipe() == -1)
    {
        perror("Родитель: не удалось настроить SIGPIPE");
        goto cleanup;
    }
    if (fprintf(stderr, "[Родитель PID=%ld] Введите путь к файлу:\n",
                (long)getpid()) < 0)
    {
        perror("Родитель: не удалось вывести приглашение");
        goto cleanup;
    }
    ssize_t length = getline(&filename, &capacity, stdin);
    if (length < 0)
    {
        if (feof(stdin))
        {
            fprintf(stderr, "Родитель: имя файла не введено\n");
        }
        else
        {
            perror("Родитель: не удалось прочитать имя файла");
        }
        goto cleanup;
    }
    if (memchr(filename, '\0', (size_t)length) != NULL)
    {
        fprintf(stderr, "Родитель: имя файла содержит нулевой байт\n");
        goto cleanup;
    }
    while (length > 0 &&
           (filename[(size_t)length - 1] == '\n' ||
            filename[(size_t)length - 1] == '\r'))
    {
        filename[(size_t)--length] = '\0';
    }
    if (length == 0)
    {
        fprintf(stderr, "Родитель: имя файла не должно быть пустым\n");
        goto cleanup;
    }
    input = open(filename, O_RDONLY);
    if (input == -1)
    {
        perror("Родитель: не удалось открыть файл");
        goto cleanup;
    }
    output = mkstemp(temporary);
    if (output == -1)
    {
        perror("Родитель: не удалось создать временный файл");
        goto cleanup;
    }
    temporary_exists = 1;

    if (unlink(temporary) == -1)
    {
        perror("Родитель: не удалось удалить имя временного файла");
        goto cleanup;
    }
    temporary_exists = 0;

    child_pid = fork();
    if (child_pid == -1)
    {
        perror("Родитель: не удалось создать дочерний процесс");
        goto cleanup;
    }
    if (child_pid == 0)
    {
        run_child(input, output);
    }
    if (close_fd(&input) == -1)
    {
        goto cleanup;
    }
    if (fprintf(stderr, "[Родитель PID=%ld] Запущен дочерний процесс PID=%ld\n",
                (long)getpid(), (long)child_pid) < 0)
    {
        perror("Родитель: не удалось вывести сообщение");
        goto cleanup;
    }

    int status;
    if (wait_for_child(child_pid, &status) == -1)
    {
        goto cleanup;
    }
    child_pid = -1;
    result = EXIT_SUCCESS;
    if (print_results(output) == -1)
    {
        result = EXIT_FAILURE;
    }
    if (WIFSIGNALED(status))
    {
        fprintf(stderr, "Родитель: дочерний процесс завершён сигналом %d\n",
                WTERMSIG(status));
        result = EXIT_FAILURE;
    }
    else if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS)
    {
        fprintf(stderr, "Родитель: дочерний процесс завершился с ошибкой\n");
        result = EXIT_FAILURE;
    }

cleanup:
    free(filename);
    if (close_fd(&input) == -1)
    {
        result = EXIT_FAILURE;
    }
    if (close_fd(&output) == -1)
    {
        result = EXIT_FAILURE;
    }
    if (temporary_exists && unlink(temporary) == -1)
    {
        perror("Родитель: не удалось удалить временный файл");
        result = EXIT_FAILURE;
    }
    if (child_pid > 0)
    {
        int cleanup_status;
        if (wait_for_child(child_pid, &cleanup_status) == -1)
        {
            result = EXIT_FAILURE;
        }
    }
    return result;
}
