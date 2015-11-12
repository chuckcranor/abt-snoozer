/*
 * (C) 2015 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */

#include <stdlib.h>
#include <abt.h>
#include <abt-snoozer.h>
#include "abt-snoozer-internal.h"

static int abt_snoozer_setup_ev(struct abt_snoozer_ev *ev);
static void sched_eloop_breaker_cb(EV_P_ ev_async *w, int revents);

int ABT_snoozer_xstream_create(ABT_pool *newpool, ABT_xstream *newxstream)
{
    return(-1);
}

int ABT_snoozer_xstream_self_set(void)
{
    int ret;
    ABT_xstream xstream;
    ABT_pool pool;
    ABT_sched sched;
    ABT_pool_def pool_def;
    struct abt_snoozer_sched_data *sched_data;
    struct abt_snoozer_pool_data *pool_data;

    ret = ABT_xstream_self(&xstream);
    if(ret != 0)
        return(ret);

    /* create new pool and scheduler */

    ret = abt_snoozer_pool_get_def(ABT_POOL_ACCESS_MPMC, &pool_def);
    if(ret != 0)
        return(ret);

    ret = ABT_pool_create(&pool_def, ABT_POOL_CONFIG_NULL, &pool);
    if(ret != 0)
        return(ret);

    ret = abt_snoozer_create_scheds(1, &pool, &sched);
    if(ret != 0)
    {
        ABT_pool_free(&pool);
        return(ret);
    }

    /* set up event loop that will like the pool to the scheduler */

    ABT_sched_get_data(sched, (void**)(&sched_data));
    ABT_pool_get_data(pool, (void**)(&pool_data));

    ret = abt_snoozer_setup_ev(&sched_data->ev);
    if(ret != 0)
    {
        ABT_sched_free(&sched);
        ABT_pool_free(&pool);
        return(ret);
    }
    pool_data->ev = sched_data->ev;

    ABT_sched_set_data(sched, sched_data);
    ABT_pool_set_data(pool, pool_data);
    
    /* override existing scheduler */

    ret = ABT_xstream_set_main_sched(xstream, sched);
    if(ret != 0)
    {
        ABT_sched_free(&sched);
        ABT_pool_free(&pool);
        return(ret);
    }

    return(0);
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

    ev->sched_eloop = ev_default_loop(EVFLAG_AUTO);
    ev_async_init(ev->sched_eloop_breaker, sched_eloop_breaker_cb);
    ev_async_start(ev->sched_eloop, ev->sched_eloop_breaker);

    return(0);
}
