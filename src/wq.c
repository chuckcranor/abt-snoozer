/*
 * (C) 2015 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */

#include <stdlib.h>
#include <abt.h>
#include <ev.h>
#include "abt-snoozer-internal.h"

/* TODO: document unusual semantics here.  Probably should pick a different
 * name than waitqueue because of connotations.  See "pending" flag.  
 */

/* TODO: non-ev version (using pthreads or similar) as fallback?  Not sure
 * what sleep/wake/timeout construct to use in that case... 
 */

struct abt_snoozer_wq_element
{
    ev_async *sched_eloop_breaker;
    ev_timer *sched_eloop_timer;
    struct ev_loop *sched_eloop;
    struct abt_snoozer_wq_element *next;
    struct abt_snoozer_wq_element *prev;
    struct abt_snoozer_wq *wq;
};

struct abt_snoozer_wq
{
    struct abt_snoozer_wq_element *head;
    int pending;
    ABT_mutex mutex;
};

static void sched_eloop_breaker_cb(EV_P_ ev_async *w, int revents);
static void sched_eloop_timer_cb(EV_P_ ev_timer *w, int revents);

void abt_snoozer_wq_init(struct abt_snoozer_wq *queue)
{
    queue->head = NULL;
    queue->pending = 0;
    ABT_mutex_create(&queue->mutex);
    
    return;
}

int abt_snoozer_wq_element_init(struct abt_snoozer_wq_element *element)
{
    element->sched_eloop_breaker = malloc(sizeof(*element->sched_eloop_breaker));
    if(!element->sched_eloop_breaker)
        return(-1);
    
    element->sched_eloop_timer = malloc(sizeof(*element->sched_eloop_timer));
    if(!element->sched_eloop_timer)
    {
        free(element->sched_eloop_breaker);
        return(-1);
    }

    element->sched_eloop = ev_loop_new(EVFLAG_AUTO);
    if(!element->sched_eloop)
    {
        free(element->sched_eloop_timer);
        free(element->sched_eloop_breaker);
        return(-1);
    }

    ev_async_init(element->sched_eloop_breaker, sched_eloop_breaker_cb);
    ev_async_start(element->sched_eloop, element->sched_eloop_breaker);

    return(0);
}

void abt_snoozer_wq_wait(struct abt_snoozer_wq *queue, 
    struct abt_snoozer_wq_element *element, double timeout_seconds)
{

    ABT_mutex_spinlock(queue->mutex);

    if(queue->pending)
    {
        /* work may have become available; don't sleep yet */ 
        queue->pending = 0;
        ABT_mutex_unlock(queue->mutex);
        return;
    }
    else
    {
        /* add self to queue */
        if(queue->head)
            queue->head->prev = element;
        element->next = queue->head;
        element->prev = NULL;
        element->wq = queue;
        /* arm timer */
        ev_timer_init(element->sched_eloop_timer, sched_eloop_timer_cb, 
            timeout_seconds, 0);
        ev_timer_start(element->sched_eloop, element->sched_eloop_timer);
    }

    ABT_mutex_unlock(queue->mutex);

    /* sleep */
    ev_run(element->sched_eloop, 0);

    ABT_mutex_spinlock(queue->mutex);
    if(element->wq)
    {
        /* take self off of queue */
        if(element->wq->head == element)
            element->wq->head = element->next;
        if(element->prev)
            element->prev->next = element->next;
        if(element->next)
            element->next->prev = element->prev;
        element->prev = NULL;
        element->next = NULL;
        element->wq = NULL;
    }
    else
    {
        /* already off queue; stop timer */
        ev_timer_stop(element->sched_eloop, element->sched_eloop_timer);
    }
    ABT_mutex_unlock(queue->mutex);

    return;
}

void abt_snoozer_wq_wake(struct abt_snoozer_wq *queue)
{
    struct abt_snoozer_wq_element *element = NULL;

    ABT_mutex_spinlock(queue->mutex);
    if(queue->head)
    {
        element = queue->head;
        queue->head = element->next;
        if(queue->head)
            queue->head->prev = NULL;
        element->next = NULL;
        element->prev = NULL;
        element->wq = NULL;
    }
    else
    {
        queue->pending = 1;
    }
        
    ABT_mutex_unlock(queue->mutex);

    if(element)
    {
        ev_async_send(element->sched_eloop, element->sched_eloop_breaker);
    }
    
    return;
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