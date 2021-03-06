/*
 * (C) 2015 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <abt.h>
#include <abt-snoozer.h>
#include "abt-snoozer-internal.h"

static int abt_snoozer_make_pool_and_scheds(int num_scheds, ABT_pool *pool, ABT_sched *scheds);

int ABT_snoozer_xstream_create(int num_xstreams, ABT_pool *newpool, ABT_xstream *newxstreams)
{
    int ret;
    ABT_sched *scheds;
    int i;

    scheds = malloc(num_xstreams * sizeof(*scheds));
    if(!scheds)
        return(-1);

    ret = abt_snoozer_make_pool_and_scheds(num_xstreams, newpool, scheds);
    if(ret != 0)
        return(ret);
    
    for(i=0; i<num_xstreams; i++)
    {
        ret = ABT_xstream_create(scheds[i], &newxstreams[i]);
        if(ret != 0)
            return(ret);
        /* TODO: error cleanup */
    }

    free(scheds);

    return(0);
}

/* creates a new pool and scheduler that are linked by an event loop */
static int abt_snoozer_make_pool_and_scheds(int num_scheds, ABT_pool *pool, ABT_sched *scheds)
{
    int ret;
    ABT_pool_def pool_def;

    ret = abt_snoozer_pool_get_def(ABT_POOL_ACCESS_MPMC, &pool_def);
    if(ret != 0)
        return(ret);

    ret = ABT_pool_create(&pool_def, ABT_POOL_CONFIG_NULL, pool);
    if(ret != 0)
        return(ret);

    ret = abt_snoozer_create_scheds(pool, num_scheds, scheds);
    if(ret != 0)
    {
        ABT_pool_free(pool);
        return(ret);
    }
   
    return(0);
}

int ABT_snoozer_xstream_self_set(void)
{
    int ret;
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_sched sched;

    ret = ABT_xstream_self(&xstream);
    if(ret != 0)
        return(ret);

    ret = abt_snoozer_make_pool_and_scheds(1, &pool, &sched);
    if(ret != 0)
        return(ret);
    
    ret = ABT_xstream_set_main_sched(xstream, sched);
    if(ret != 0)
    {
        ABT_sched_free(&sched);
        ABT_pool_free(&pool);
        return(ret);
    }

    return(0);
} 

