#include "mapping.h"
#include "io.h"

#include <errno.h>
#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int append_sum(int fd, off_t *length, float value)
{
    char text[64];
    int count = snprintf(text, sizeof(text), "%.*g\n",
                         FLT_DECIMAL_DIG, (double)value);
    if (count < 0 || (size_t)count >= sizeof(text))
    {
        fprintf(stderr, "Не удалось подготовить сумму для записи\n");
        return -1;
    }
    if ((uintmax_t)*length + (unsigned int)count > INT64_MAX ||
        (uintmax_t)*length + (unsigned int)count > SIZE_MAX)
    {
        fprintf(stderr, "Файл результатов слишком большой\n");
        return -1;
    }

    off_t new_length = *length + count;
    if (ftruncate(fd, new_length) == -1)
    {
        perror("Не удалось увеличить файл результатов");
        return -1;
    }
    char *data = mmap(NULL, (size_t)new_length, PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        perror("Не удалось отобразить файл для записи");
        return -1;
    }

    memcpy(data + (size_t)*length, text, (size_t)count);
    *length = new_length;
    if (munmap(data, (size_t)new_length) == -1)
    {
        perror("Не удалось снять отображение после записи");
        return -1;
    }
    return 0;
}

int print_results(int fd)
{
    struct stat info;
    if (fstat(fd, &info) == -1)
    {
        perror("Не удалось узнать размер файла результатов");
        return -1;
    }
    if (info.st_size == 0)
    {
        return 0;
    }
    if (info.st_size < 0 || (uintmax_t)info.st_size > SIZE_MAX)
    {
        fprintf(stderr, "Недопустимый размер файла результатов\n");
        return -1;
    }

    size_t length = (size_t)info.st_size;
    const char *data = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        perror("Не удалось отобразить файл для чтения");
        return -1;
    }
    int result = write_all(STDOUT_FILENO, data, length);
    if (result == -1)
    {
        perror("Родитель: не удалось вывести результаты");
    }
    if (munmap((void *)data, length) == -1)
    {
        perror("Не удалось снять отображение после чтения");
        result = -1;
    }
    return result;
}
