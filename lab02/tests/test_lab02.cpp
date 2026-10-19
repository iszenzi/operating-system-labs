#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/wait.h>

extern "C"
{
#include "sum.h"
}

TEST(SumTest, AddsFloatNumbers)
{
    const char line[] = "1.5 2.25 -0.75";
    float result = 0.0f;

    ASSERT_EQ(sum_line(line, std::strlen(line), &result), SUM_OK);
    EXPECT_FLOAT_EQ(result, 3.0f);
}

TEST(SumTest, HandlesNegativeNumbersAndExponent)
{
    const char line[] = "-1e2 2.5e1 .125";
    float result = 0.0f;

    ASSERT_EQ(sum_line(line, std::strlen(line), &result), SUM_OK);
    EXPECT_FLOAT_EQ(result, -74.875f);
}

TEST(SumTest, HandlesWhitespace)
{
    const char line[] = " \t 2.5  -1\n";
    float result = 0.0f;

    ASSERT_EQ(sum_line(line, std::strlen(line), &result), SUM_OK);
    EXPECT_FLOAT_EQ(result, 1.5f);
}

TEST(SumTest, SkipsEmptyLine)
{
    const char line[] = " \t\n";
    float result = 0.0f;

    EXPECT_EQ(sum_line(line, std::strlen(line), &result), SUM_EMPTY);
}

TEST(SumTest, RejectsInvalidNumber)
{
    const char line[] = "1.5 error";
    float result = 0.0f;

    EXPECT_EQ(sum_line(line, std::strlen(line), &result), SUM_INVALID_NUMBER);
}

TEST(SumTest, RejectsNumberOutsideFloatRange)
{
    const char line[] = "1e100";
    float result = 0.0f;

    EXPECT_EQ(sum_line(line, std::strlen(line), &result), SUM_NUMBER_RANGE);
}

TEST(SumTest, DetectsSumOverflow)
{
    const char line[] = "3e38 3e38";
    float result = 0.0f;

    EXPECT_EQ(sum_line(line, std::strlen(line), &result), SUM_OVERFLOW);
}

TEST(ProcessTest, TransfersResultsThroughPipe)
{
    FILE *process = popen(
        "printf '%s\\n' examples/numbers.txt | timeout 5 ./build/parent 2>/dev/null",
        "r");
    ASSERT_NE(process, nullptr);

    char buffer[128];
    std::string output;
    while (fgets(buffer, sizeof(buffer), process) != nullptr)
    {
        output += buffer;
    }
    int read_error = ferror(process);
    int status = pclose(process);

    EXPECT_EQ(read_error, 0);
    ASSERT_NE(status, -1);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0);
    EXPECT_EQ(output,
              "3\n8\n75.125\n0\n-4\n42.5\n0.875\n0\n55\n1.00100005\n");
}

TEST(ProcessTest, ReportsMissingFile)
{
    FILE *process = popen(
        "printf '%s\\n' /proc/self/lab02_missing_file | timeout 5 ./build/parent 2>/dev/null",
        "r");
    ASSERT_NE(process, nullptr);

    int output = fgetc(process);
    int read_error = ferror(process);
    int status = pclose(process);

    EXPECT_EQ(output, EOF);
    EXPECT_EQ(read_error, 0);
    ASSERT_NE(status, -1);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), EXIT_FAILURE);
}
