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

#define ABT_SNOOZER_POOL_DATA_PADDING 256
struct abt_snoozer_pool_data {
    /* allow space for underlying pool data */
    char data[ABT_SNOOZER_POOL_DATA_PADDING]; 
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
