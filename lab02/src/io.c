#include "io.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int write_all(int fd, const void *buffer, size_t length)
{
    const char *cursor = buffer;

    while (length > 0) {
        ssize_t written = write(fd, cursor, length);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (written == 0) {
            errno = EIO;
            return -1;
        }
        cursor += (size_t)written;
        length -= (size_t)written;
    }
    return 0;
}

int close_fd(int *fd)
{
    if (*fd < 0) {
        return 0;
    }

    int value = *fd;
    *fd = -1;
    if (close(value) == -1) {
        perror("Не удалось закрыть файловый дескриптор");
        return -1;
    }
    return 0;
}

int ignore_sigpipe(void)
{
    struct sigaction action = {0};
    action.sa_handler = SIG_IGN;
    if (sigemptyset(&action.sa_mask) == -1) {
        return -1;
    }
    return sigaction(SIGPIPE, &action, NULL);
}
