#include "io.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void run_child(int input, int pipe_read, int pipe_write)
{
    if (close_fd(&pipe_read) == -1) {
        _exit(EXIT_FAILURE);
    }
    if (dup2(input, STDIN_FILENO) == -1) {
        perror("Дочерний процесс: не удалось перенаправить ввод");
        _exit(EXIT_FAILURE);
    }
    if (input != STDIN_FILENO && close_fd(&input) == -1) {
        _exit(EXIT_FAILURE);
    }
    if (dup2(pipe_write, STDOUT_FILENO) == -1) {
        perror("Дочерний процесс: не удалось перенаправить вывод");
        _exit(EXIT_FAILURE);
    }
    if (pipe_write != STDOUT_FILENO && close_fd(&pipe_write) == -1) {
        _exit(EXIT_FAILURE);
    }

    execl("./build/child", "child", (char *)NULL);
    perror("Дочерний процесс: не удалось запустить build/child");
    _exit(EXIT_FAILURE);
}

static int wait_for_child(pid_t child_pid)
{
    int status;
    pid_t waited;

    do {
        waited = waitpid(child_pid, &status, 0);
    } while (waited == -1 && errno == EINTR);

    if (waited == -1) {
        perror("Родитель: не удалось дождаться дочернего процесса");
        return EXIT_FAILURE;
    }
    if (WIFSIGNALED(status)) {
        fprintf(stderr, "Родитель: дочерний процесс завершён сигналом %d\n",
                WTERMSIG(status));
        return EXIT_FAILURE;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
        fprintf(stderr, "Родитель: дочерний процесс завершился с ошибкой\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(void)
{
    int input = -1;
    int channel[2] = {-1, -1};
    pid_t child_pid = -1;
    char *filename = NULL;
    size_t capacity = 0;
    int result = EXIT_FAILURE;

    if (ignore_sigpipe() == -1) {
        perror("Родитель: не удалось настроить SIGPIPE");
        goto cleanup;
    }
    if (fprintf(stderr, "[Родитель PID=%ld] Введите путь к файлу:\n",
                (long)getpid()) < 0) {
        perror("Родитель: не удалось вывести приглашение");
        goto cleanup;
    }

    ssize_t length = getline(&filename, &capacity, stdin);
    if (length < 0) {
        if (feof(stdin)) {
            fprintf(stderr, "Родитель: имя файла не введено\n");
        } else {
            perror("Родитель: не удалось прочитать имя файла");
        }
        goto cleanup;
    }
    if (memchr(filename, '\0', (size_t)length) != NULL) {
        fprintf(stderr, "Родитель: имя файла содержит нулевой байт\n");
        goto cleanup;
    }
    while (length > 0 &&
           (filename[(size_t)length - 1] == '\n' ||
            filename[(size_t)length - 1] == '\r')) {
        filename[(size_t)--length] = '\0';
    }
    if (length == 0) {
        fprintf(stderr, "Родитель: имя файла не должно быть пустым\n");
        goto cleanup;
    }

    input = open(filename, O_RDONLY);
    if (input == -1) {
        perror("Родитель: не удалось открыть файл");
        goto cleanup;
    }
    if (pipe(channel) == -1) {
        perror("Родитель: не удалось создать канал");
        goto cleanup;
    }
    child_pid = fork();
    if (child_pid == -1) {
        perror("Родитель: не удалось создать дочерний процесс");
        goto cleanup;
    }
    if (child_pid == 0) {
        run_child(input, channel[0], channel[1]);
    }

    int close_error = close_fd(&input);
    if (close_fd(&channel[1]) == -1) {
        close_error = -1;
    }
    if (close_error == -1) {
        goto cleanup;
    }
    if (fprintf(stderr, "[Родитель PID=%ld] Запущен дочерний процесс PID=%ld\n",
                (long)getpid(), (long)child_pid) < 0) {
        perror("Родитель: не удалось вывести сообщение");
        goto cleanup;
    }

    result = EXIT_SUCCESS;
    char buffer[4096];
    for (;;) {
        ssize_t received = read(channel[0], buffer, sizeof(buffer));
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Родитель: не удалось прочитать данные из канала");
            result = EXIT_FAILURE;
            break;
        }
        if (received == 0) {
            break;
        }
        if (write_all(STDOUT_FILENO, buffer, (size_t)received) == -1) {
            perror("Родитель: не удалось вывести результат");
            result = EXIT_FAILURE;
            break;
        }
    }

cleanup:
    free(filename);
    if (close_fd(&input) == -1) {
        result = EXIT_FAILURE;
    }
    if (close_fd(&channel[0]) == -1) {
        result = EXIT_FAILURE;
    }
    if (close_fd(&channel[1]) == -1) {
        result = EXIT_FAILURE;
    }
    if (child_pid > 0 && wait_for_child(child_pid) != EXIT_SUCCESS) {
        result = EXIT_FAILURE;
    }
    return result;
}
