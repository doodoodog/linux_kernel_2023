#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cond.h"
#include "futex.h"
#include "mutex.h"

struct clock {
    mutex_t mutex;
    cond_t cond;
    int ticks;
};

static void clock_init(struct clock *clock)
{
    mutex_init(&clock->mutex);
    cond_init(&clock->cond);
    clock->ticks = 0;
}

static bool clock_wait(struct clock *clock, int ticks)
{
    mutex_lock(&clock->mutex);
    while (clock->ticks >= 0 && clock->ticks < ticks)
    {
        printf("%d %d\n", clock->ticks, ticks);
        cond_wait(&clock->cond, &clock->mutex);
    }
    // printf("wakeup clock ticks : %d, cur. ticks : %d\n", clock->ticks, ticks);
    bool ret = clock->ticks >= ticks;
    mutex_unlock(&clock->mutex);
    return ret;
}

static void clock_tick(struct clock *clock)
{
    mutex_lock(&clock->mutex);
    if (clock->ticks >= 0)
        ++clock->ticks;
    mutex_unlock(&clock->mutex);
    cond_broadcast(&clock->cond, &clock->mutex);
}

static void clock_stop(struct clock *clock)
{
    mutex_lock(&clock->mutex);
    clock->ticks = -1;
    mutex_unlock(&clock->mutex);
    cond_broadcast(&clock->cond, &clock->mutex);
}

/* A node in a computation graph */
struct node {
    struct clock *clock;
    struct node *parent;
    mutex_t mutex;
    cond_t cond;
    bool ready;
    unsigned int pid;
    unsigned int tickCnt;
    unsigned int signCnt;
};

static void node_init(struct clock *clock,
                      struct node *parent,
                      struct node *node)
{
    node->clock = clock;
    node->parent = parent;
    mutex_init(&node->mutex);
    cond_init(&node->cond);
    node->ready = false;
}

static void node_wait(struct node *node)
{
    mutex_lock(&node->mutex);
    while (!node->ready)
        cond_wait(&node->cond, &node->mutex);
    node->ready = false;
    printf("node : %d rdy 1 -> 0\n", node->pid);
    mutex_unlock(&node->mutex);
}

static void node_signal(struct node *node)
{
    mutex_lock(&node->mutex);
    node->ready = true;
    mutex_unlock(&node->mutex);
    cond_signal(&node->cond, &node->mutex);
}

static void *thread_func(void *ptr)
{
    struct node *self = ptr;
    bool bit = false;

    printf("pid : %d join\n", self->pid);

    for (int i = 1; clock_wait(self->clock, i); ++i) {
        printf("wakeup pid : %d\n", self->pid);
        if (self->parent)
        {
            printf("pid : %d, wait parent : %d\n", self->pid, self->parent->pid);
            node_wait(self->parent);
        }
        printf("pid run : %d\n", self->pid);
        if (bit) {
            self->signCnt++;
            mutex_lock(&self->clock->mutex);
            printf("signal (i : %d, clock : %d) pid : %d, cnt : %d\n", i, self->clock->ticks, self->pid, self->signCnt);
            mutex_unlock(&self->clock->mutex);
            node_signal(self);
        } else {
            self->tickCnt++;
            clock_tick(self->clock);
            mutex_lock(&self->clock->mutex);
            printf("tick (i : %d, clock : %d) pid : %d, cnt : %d\n", i, self->clock->ticks, self->pid, self->tickCnt);
            mutex_unlock(&self->clock->mutex);
        }
        bit = !bit;
    }
    printf("thread id : %u, tick count : %u\n", self->pid, self->tickCnt);
    printf("thread id : %u, sign count : %u\n", self->pid, self->signCnt);
    node_signal(self);
    return NULL;
}

int main(void)
{
    struct clock clock;
    clock_init(&clock);

#define N_NODES 16
    struct node nodes[N_NODES];
    node_init(&clock, NULL, &nodes[0]);
    nodes[0].pid = 0;
    nodes[0].tickCnt = 0;
    nodes[0].signCnt = 0;
    for (int i = 1; i < N_NODES; ++i)
    {
        nodes[i].pid = i;
        nodes[i].tickCnt = 0;
        nodes[i].signCnt = 0;
        node_init(&clock, &nodes[i - 1], &nodes[i]);
    }

    pthread_t threads[N_NODES];
    for (int i = 0; i < N_NODES; ++i) {
        if (pthread_create(&threads[i], NULL, thread_func, &nodes[i]) != 0)
            return EXIT_FAILURE;
    }

    clock_tick(&clock);
    clock_wait(&clock, 1 << N_NODES);
    clock_stop(&clock);

    for (int i = 0; i < N_NODES; ++i) {
        if (pthread_join(threads[i], NULL) != 0)
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}