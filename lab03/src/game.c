#include "game.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

struct worker_data
{
    const struct game_options *options;
    uint64_t first;
    uint64_t count;
    struct game_result result;
    int error;
};

static uint64_t random_number(uint64_t *state)
{
    *state += UINT64_C(0x9e3779b97f4a7c15);
    uint64_t value = *state;
    value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
    return value ^ (value >> 31);
}

static uint64_t roll_die(uint64_t *state)
{
    uint64_t value;
    do
    {
        value = random_number(state);
    } while (value < 4);
    return value % 6 + 1;
}

static void *run_worker(void *argument)
{
    struct worker_data *worker = argument;
    const struct game_options *options = worker->options;
    if (!options->quiet)
    {
        long tid = syscall(SYS_gettid);
        if (tid == -1)
        {
            perror("Не удалось получить TID");
            worker->error = 1;
            return NULL;
        }
        if (fprintf(stderr, "[PID=%ld TID=%ld] Начало: опытов=%llu, первый=%llu\n",
                    (long)getpid(), tid, (unsigned long long)worker->count,
                    (unsigned long long)worker->first) < 0)
        {
            worker->error = 1;
            return NULL;
        }
    }

    for (uint64_t i = 0; i < worker->count; ++i)
    {
        uint64_t index = worker->first + i;
        uint64_t state = options->seed + index * UINT64_C(0x9e3779b97f4a7c15);
        state = random_number(&state);
        uint64_t score1 = options->score1;
        uint64_t score2 = options->score2;
        for (uint64_t round = options->completed; round < options->rounds; ++round)
        {
            score1 += roll_die(&state);
            score1 += roll_die(&state);
            score2 += roll_die(&state);
            score2 += roll_die(&state);
        }
        if (score1 > score2)
        {
            ++worker->result.wins1;
        }
        else if (score2 > score1)
        {
            ++worker->result.wins2;
        }
        else
        {
            ++worker->result.draws;
        }
    }
    return NULL;
}

int simulate_game(const struct game_options *options, struct game_result *result)
{
    unsigned int count = options->threads;
    if (options->experiments < count)
    {
        count = (unsigned int)options->experiments;
    }
    struct worker_data *workers = calloc(count, sizeof(*workers));
    pthread_t *threads = calloc(count, sizeof(*threads));
    if (workers == NULL || threads == NULL)
    {
        perror("Не удалось выделить память для потоков");
        free(workers);
        free(threads);
        return -1;
    }

    uint64_t first = 0;
    for (unsigned int i = 0; i < count; ++i)
    {
        workers[i].options = options;
        workers[i].first = first;
        workers[i].count = options->experiments / count;
        if (i < options->experiments % count)
        {
            ++workers[i].count;
        }
        first += workers[i].count;
    }

    unsigned int created = 0;
    int failed = 0;
    for (unsigned int i = 0; i + 1 < count; ++i)
    {
        int error = pthread_create(&threads[i], NULL, run_worker, &workers[i]);
        if (error != 0)
        {
            fprintf(stderr, "Не удалось создать поток: %s\n", strerror(error));
            failed = 1;
            break;
        }
        ++created;
    }
    if (!failed)
    {
        run_worker(&workers[count - 1]);
    }
    for (unsigned int i = 0; i < created; ++i)
    {
        int error = pthread_join(threads[i], NULL);
        if (error != 0)
        {
            fprintf(stderr, "Не удалось дождаться потока: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
    }

    *result = (struct game_result){0};
    result->threads_used = count;
    for (unsigned int i = 0; i < count; ++i)
    {
        result->wins1 += workers[i].result.wins1;
        result->wins2 += workers[i].result.wins2;
        result->draws += workers[i].result.draws;
        if (workers[i].error)
        {
            failed = 1;
        }
    }
    free(threads);
    free(workers);
    if (failed)
    {
        return -1;
    }
    return 0;
}
