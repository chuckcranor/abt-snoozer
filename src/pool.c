/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 * See COPYRIGHT in top-level directory.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <abt.h>
#include <abt-snoozer.h>
#include "abt-snoozer-internal.h"

/* abt snoozer pool implementation to be used in tandem with the abt
 * snoozer scheduler.  We don't actually want to change the default Argobots
 * pool organization; we just want to be able to signal the scheduler when
 * new items are added to the pool.  This implementation just intercepts the
 * calls to the default pool to add this functionality.
 */

typedef struct abt_snoozer_pool_data data_t;

static void abt_snoozer_pool_push_shared(ABT_pool pool, ABT_unit unit);
static int abt_snoozer_pool_init(ABT_pool pool, ABT_pool_config config);
static int abt_snoozer_pool_free(ABT_pool pool);

/* Obtain the FIFO pool definition according to the access type */
int abt_snoozer_pool_get_def(ABT_pool_access access, ABT_pool_def *p_def)
{
    int abt_errno = ABT_SUCCESS;

    /* Don't support private modes for now, would need to do something to
     * track what access mode is being used when we get to pool_init()
     * otherwise
     */
    assert(access != ABT_POOL_ACCESS_PRIV);

    /* get default pool */
    abt_errno = ABT_pool_get_def_basic(ABT_POOL_FIFO, access, p_def);
    if(abt_errno != ABT_SUCCESS)
        return(abt_errno);

    /* override push, init, and free depending on access mode */
    switch (access) {
        case ABT_POOL_ACCESS_PRIV:
            /* no change */
            break;
        case ABT_POOL_ACCESS_SPSC:
        case ABT_POOL_ACCESS_MPSC:
        case ABT_POOL_ACCESS_SPMC:
        case ABT_POOL_ACCESS_MPMC:
            /* override push function */
            p_def->p_push = abt_snoozer_pool_push_shared;
            p_def->p_init = abt_snoozer_pool_init;
            p_def->p_free = abt_snoozer_pool_free;
            break;
        default:
            return(ABT_ERR_INV_POOL_ACCESS);
    }

    return ABT_SUCCESS;
}


/* Pool functions */
static int abt_snoozer_pool_init(ABT_pool pool, ABT_pool_config config)
{
    ABT_pool_def default_def;
    int abt_errno = ABT_SUCCESS;
    data_t *p_data;
    void *data;

    p_data = malloc(sizeof(data_t));
    if(!p_data)
        return(ABT_ERR_MEM);

    /* get default pool */
    abt_errno = ABT_pool_get_def_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC, &default_def);
    if(abt_errno != ABT_SUCCESS)
    {
        free(p_data);
        return(abt_errno);
    }

    /* call the default init function */
    abt_errno = default_def.p_init(pool, config);
    if(abt_errno != ABT_SUCCESS)
    {
        free(p_data);
        return(abt_errno);
    }

    /* retrieve "data" from default pool implementation, embed it in our own */
    ABT_pool_get_data(pool, &data);
    memcpy(p_data, data, ABT_SNOOZER_POOL_DATA_PADDING);

    /* add on our own wait queue structure */
    p_data->wq = abt_snoozer_wq_alloc();
    if(!p_data->wq)
    {
        default_def.p_free(pool);
        free(p_data);
        return(ABT_ERR_MEM);
    }

    /* reset data to point to our new structure, with wq piggy-backed on */
    ABT_pool_set_data(pool, p_data);
    free(data);

    return abt_errno;
}

static int abt_snoozer_pool_free(ABT_pool pool)
{
    int abt_errno = ABT_SUCCESS;
    void *data;
    ABT_pool_def default_def;

    /* get default pool */
    abt_errno = ABT_pool_get_def_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC, &default_def);
    if(abt_errno != ABT_SUCCESS)
    {
        return(abt_errno);
    }

    /* free the extra data we tacked on */
    ABT_pool_get_data(pool, &data);
    data_t *p_data = (data_t*)data;
    abt_snoozer_wq_free(p_data->wq);

    /* call the default free function */
    abt_errno = default_def.p_free(pool);
    if(abt_errno != ABT_SUCCESS)
    {
        return(abt_errno);
    }

    return abt_errno;
}

static void abt_snoozer_pool_push_shared(ABT_pool pool, ABT_unit unit)
{
    ABT_pool_def default_def;
    void *data;
    data_t *p_data;
    int abt_errno = ABT_SUCCESS;

    /* get default pool */
    abt_errno = ABT_pool_get_def_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC, &default_def);
    assert(abt_errno == ABT_SUCCESS);

    /* call the default push shared function */
    default_def.p_push(pool, unit);

    /* signal wait queue */
    ABT_pool_get_data(pool, &data);
    p_data = data;
    abt_snoozer_wq_wake(p_data->wq);

    return;
}

