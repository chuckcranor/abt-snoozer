/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 * See COPYRIGHT in top-level directory.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <abt.h>

#include <abt-snoozer.h>
#include "abt-snoozer-internal.h"

/* derived from argobots/src/sched/basic.c */
/* TODO: licensing */

/******************************************************************************/
/* Scheduler data structure and functions                                     */
/******************************************************************************/
typedef struct abt_snoozer_sched_data sched_data_t;

static int sched_init(ABT_sched sched, ABT_sched_config config)
{
    sched_data_t *p_data = (sched_data_t *)calloc(1, sizeof(sched_data_t));

    ABT_sched_config_read(config, 1, &p_data->event_freq);
    ABT_sched_set_data(sched, (void *)p_data);

    return ABT_SUCCESS;
}

static void sched_run(ABT_sched sched)
{
    uint32_t work_count = 0;
    sched_data_t *p_data;
    int num_pools;
    ABT_pool *pools;
    ABT_unit unit;
    int target;
    ABT_bool stop;
    unsigned seed = time(NULL);
    int loop_total;

    ABT_sched_get_data(sched, (void **)&p_data);
    ABT_sched_get_num_pools(sched, &num_pools);
    pools = (ABT_pool *)malloc(num_pools * sizeof(ABT_pool));
    ABT_sched_get_pools(sched, num_pools, 0, pools);

    while (1) {
        loop_total = 0;
        /* Execute one work unit from the scheduler's pool */
        ABT_pool_pop(pools[0], &unit);
        if (unit != ABT_UNIT_NULL) {
            ABT_xstream_run_unit(unit, pools[0]);
            loop_total++;
        } else if (num_pools > 1) {
            /* Steal a work unit from other pools */
            target = (num_pools == 2) ? 1 : (rand_r(&seed) % (num_pools-1) + 1);
            ABT_pool_pop(pools[target], &unit);
            if (unit != ABT_UNIT_NULL) {
                ABT_xstream_run_unit(unit, pools[target]);
                loop_total++;
            }
        }

        if (++work_count >= p_data->event_freq) {
            work_count = 0;
            ABT_xstream_check_events(sched);
            ABT_sched_has_to_stop(sched, &stop);
            if (stop == ABT_TRUE) break;
            if(loop_total == 0)
            {
                /* nothing to do; sleep unless signaled by a pool */
                ev_run(p_data->ev.sched_eloop, 0);
            }
        }
    }

    free(pools);
}

static int sched_free(ABT_sched sched)
{
    sched_data_t *p_data;

    ABT_sched_get_data(sched, (void **)&p_data);
    free(p_data);

    return ABT_SUCCESS;
}

int abt_snoozer_create_scheds(int num, ABT_pool *pools, ABT_sched *scheds)
{
    ABT_sched_config config;
    ABT_pool *my_pools;
    int i, k;
    int ret;

    ABT_sched_config_var cv_event_freq = {
        .idx = 0,
        .type = ABT_SCHED_CONFIG_INT
    };

    ABT_sched_def sched_def = {
        .type = ABT_SCHED_TYPE_ULT,
        .init = sched_init,
        .run = sched_run,
        .free = sched_free,
        .get_migr_pool = NULL
    };

    /* Create a scheduler config */
    ret = ABT_sched_config_create(&config, cv_event_freq, 10,
                            ABT_sched_config_var_end);
    if(ret != 0)
        return(ret);

    my_pools = (ABT_pool *)malloc(num * sizeof(ABT_pool));
    if(!my_pools)
    {
        ABT_sched_config_free(&config);
        return(-1);
    }
    for (i = 0; i < num; i++) {
        for (k = 0; k < num; k++) {
            my_pools[k] = pools[(i + k) % num];
        }

        ret = ABT_sched_create(&sched_def, num, my_pools, config, &scheds[i]);
        if(ret != 0)
        {
            free(my_pools);
            ABT_sched_config_free(&config);
            return(ret);
        }
    }
    free(my_pools);

    ABT_sched_config_free(&config);

    return(0);
}

