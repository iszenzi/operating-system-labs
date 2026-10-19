#include "sum.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

enum sum_status sum_line(const char *line, size_t length, float *result)
{
    if (memchr(line, '\0', length) != NULL)
    {
        return SUM_INVALID_NUMBER;
    }

    const char *cursor = line;
    float total = 0.0f;
    int has_numbers = 0;

    for (;;)
    {
        while (isspace((unsigned char)*cursor))
        {
            ++cursor;
        }
        if (*cursor == '\0')
        {
            break;
        }

        char *end = NULL;
        errno = 0;
        float value = strtof(cursor, &end);
        if (end == cursor || (*end != '\0' && !isspace((unsigned char)*end)))
        {
            return SUM_INVALID_NUMBER;
        }
        if (errno == ERANGE)
        {
            return SUM_NUMBER_RANGE;
        }
        if (!isfinite(value))
        {
            return SUM_INVALID_NUMBER;
        }

        total += value;
        if (!isfinite(total))
        {
            return SUM_OVERFLOW;
        }
        has_numbers = 1;
        cursor = end;
    }

    *result = total;
    return has_numbers ? SUM_OK : SUM_EMPTY;
}

const char *sum_error(enum sum_status status)
{
    switch (status)
    {
    case SUM_INVALID_NUMBER:
        return "ожидаются конечные числа float, разделённые пробелами";
    case SUM_NUMBER_RANGE:
        return "число выходит за допустимый диапазон float";
    case SUM_OVERFLOW:
        return "переполнение суммы float";
    case SUM_OK:
    case SUM_EMPTY:
        return "ошибки нет";
    }
    return "неизвестная ошибка входных данных";
}
