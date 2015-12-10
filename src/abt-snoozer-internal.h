/*
 * (C) 2015 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */

#ifndef __ABT_SNOOZER_INTERNAL
#define __ABT_SNOOZER_INTERNAL

#include <abt-snoozer.h>

struct abt_snoozer_wq;
struct abt_snoozer_wq_element;

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
    struct abt_snoozer_wq* wq;
};

int abt_snoozer_pool_get_def(ABT_pool_access access, ABT_pool_def *p_def);
int abt_snoozer_create_scheds(ABT_pool *pool, int num_scheds, ABT_sched *scheds);

struct abt_snoozer_wq* abt_snoozer_wq_alloc(void);
void abt_snoozer_wq_free(struct abt_snoozer_wq *wq);

struct abt_snoozer_wq_element* abt_snoozer_wq_element_alloc(void);
void abt_snoozer_wq_element_free(struct abt_snoozer_wq_element *element);

void abt_snoozer_wq_wait(struct abt_snoozer_wq *queue, 
    struct abt_snoozer_wq_element *element, double timeout_seconds);
void abt_snoozer_wq_wake(struct abt_snoozer_wq *queue);

#endif /* __ABT_SNOOZER_INTERNAL */
