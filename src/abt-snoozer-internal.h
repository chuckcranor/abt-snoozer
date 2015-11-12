/*
 * (C) 2015 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */

#ifndef __ABT_SNOOZER_INTERNAL
#define __ABT_SNOOZER_INTERNAL

#include <abt-snoozer.h>
#include <ev.h>

struct abt_snoozer_ev {
    ev_async *sched_eloop_breaker;
    struct ev_loop *sched_eloop;
};

struct abt_snoozer_unit {
    struct abt_snoozer_unit *p_prev;
    struct abt_snoozer_unit *p_next;
    ABT_pool pool;
    union {
        ABT_thread thread;
        ABT_task   task;
    };
    ABT_unit_type type;
};

struct abt_snoozer_pool_data {
    ABT_mutex mutex;
    size_t num_units;
    struct abt_snoozer_unit *p_head;
    struct abt_snoozer_unit *p_tail;
    struct abt_snoozer_ev ev;
};

struct abt_snoozer_sched_data {
    uint32_t event_freq;
    struct abt_snoozer_ev ev;
};

int abt_snoozer_pool_get_def(ABT_pool_access access, ABT_pool_def *p_def);
int abt_snoozer_create_scheds(int num, ABT_pool *pools, ABT_sched *scheds);

#endif /* __ABT_SNOOZER_INTERNAL */
