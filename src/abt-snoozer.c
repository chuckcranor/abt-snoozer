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

static int abt_snoozer_setup_ev(struct abt_snoozer_ev *ev);
static void sched_eloop_breaker_cb(EV_P_ ev_async *w, int revents);
static void sched_eloop_timer_cb(EV_P_ ev_timer *w, int revents);
static int abt_snoozer_make_pool_and_sched(ABT_pool *pool, ABT_sched *sched);

int ABT_snoozer_xstream_create(ABT_pool *newpool, ABT_xstream *newxstream)
{
    int ret;
    ABT_sched sched;

    ret = abt_snoozer_make_pool_and_sched(newpool, &sched);
    if(ret != 0)
        return(ret);
    
    ret = ABT_xstream_create(sched, newxstream);
    if(ret != 0)
        return(ret);

    return(0);
}

/* creates a new pool and scheduler that are linked by an event loop */
static int abt_snoozer_make_pool_and_sched(ABT_pool *pool, ABT_sched *sched)
{
    int ret;
    ABT_pool_def pool_def;
    struct abt_snoozer_sched_data *sched_data;
    struct abt_snoozer_pool_data *pool_data;

    ret = abt_snoozer_pool_get_def(ABT_POOL_ACCESS_MPMC, &pool_def);
    if(ret != 0)
        return(ret);

    ret = ABT_pool_create(&pool_def, ABT_POOL_CONFIG_NULL, pool);
    if(ret != 0)
        return(ret);

    /* TODO: start changing things here */
    /* - get rid of the get_data stuff
     * - intead have helper function that will go from pool->wq (using
     *   get_data and knowledge of pool data format)
     *   - use that helper when sleeping to wait on proper queue
     *   - no need to expose any scheduler data
     */

    ret = abt_snoozer_create_scheds(pool, 1, sched);
    if(ret != 0)
    {
        ABT_pool_free(pool);
        return(ret);
    }

    /* set up event loop that will like the pool to the scheduler */

    ABT_sched_get_data(*sched, (void**)(&sched_data));
    ABT_pool_get_data(*pool, (void**)(&pool_data));

    ret = abt_snoozer_setup_ev(&sched_data->ev);
    if(ret != 0)
    {
        ABT_sched_free(sched);
        ABT_pool_free(pool);
        return(ret);
    }
    pool_data->ev = sched_data->ev;

    ABT_sched_set_data(*sched, sched_data);
    ABT_pool_set_data(*pool, pool_data);
   
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

    ret = abt_snoozer_make_pool_and_sched(&pool, &sched);
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

static void sched_eloop_timer_cb(EV_P_ ev_timer *w, int revents)
{
    /* do nothing except break out of the event loop */
    ev_break(EV_A_ EVBREAK_ONE);
    return;
}

static void sched_eloop_breaker_cb(EV_P_ ev_async *w, int revents)
{
    /* do nothing except break out of the event loop */
    ev_break(EV_A_ EVBREAK_ONE);
    return;
}

static int abt_snoozer_setup_ev(struct abt_snoozer_ev *ev)
{
    ev->sched_eloop_breaker = malloc(sizeof(*ev->sched_eloop_breaker));
    if(!ev->sched_eloop_breaker)
        return(-1);
    
    ev->sched_eloop_timer = malloc(sizeof(*ev->sched_eloop_timer));
    if(!ev->sched_eloop_timer)
    {
        free(ev->sched_eloop_breaker);
        return(-1);
    }

    ev->sched_eloop = ev_loop_new(EVFLAG_AUTO);
    ev_async_init(ev->sched_eloop_breaker, sched_eloop_breaker_cb);
    ev_timer_init(ev->sched_eloop_timer, sched_eloop_timer_cb, 0.1, 0);
    ev_async_start(ev->sched_eloop, ev->sched_eloop_breaker);

    return(0);
}
