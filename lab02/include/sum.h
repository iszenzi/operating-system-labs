#ifndef LAB02_SUM_H
#define LAB02_SUM_H

#include <stddef.h>

enum sum_status
{
    SUM_OK,
    SUM_EMPTY,
    SUM_INVALID_NUMBER,
    SUM_NUMBER_RANGE,
    SUM_OVERFLOW
};

enum sum_status sum_line(const char *line, size_t length, float *result);
const char *sum_error(enum sum_status status);

#endif
