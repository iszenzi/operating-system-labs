#ifndef GAME_H
#define GAME_H

#include <stdint.h>

struct game_options
{
    uint64_t rounds;
    uint64_t completed;
    uint64_t score1;
    uint64_t score2;
    uint64_t experiments;
    uint64_t seed;
    unsigned int threads;
    int quiet;
};

struct game_result
{
    uint64_t wins1;
    uint64_t wins2;
    uint64_t draws;
    unsigned int threads_used;
};

int parse_options(int argc, char **argv, struct game_options *options);
int print_usage(const char *program);
int simulate_game(const struct game_options *options, struct game_result *result);

#endif
