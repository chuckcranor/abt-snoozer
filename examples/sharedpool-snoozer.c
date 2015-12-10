/*
 * (C) 2015 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <abt.h>
#include <abt-snoozer.h>

/* Expected behavior: the primary ES will block on 4 eventual constructs that
 * depend on ULTs that were all submitted to the same pool.  The pool will be
 * associated with 4 separate ESs/schedulers
 */

void thread_fn(void *_arg)
{
    ABT_eventual *eventual = _arg;
    int ret = 0;
    printf("starting ULT.\n");

    /* NOTE: this will block whatever ES it executes on */
    sleep(5);
    
    /* set eventual when done */
    ABT_eventual_set(*eventual, &ret, sizeof(ret));

    printf("finishing ULT.\n");
    return;
}

int main(int argc, char **argv) 
{
    int ret;
    ABT_thread tids[4];
    ABT_pool shared_pool;
    ABT_xstream xstreams[4];
    ABT_eventual eventuals[4];
    int *eret;
    int i;
    
    ret = ABT_init(argc, argv);
    if(ret != 0)
    {
        fprintf(stderr, "Error: ABT_init()\n");
        return(-1);
    }

    /* set primary ES to use the snoozer */
    ret = ABT_snoozer_xstream_self_set();
    assert(ret == 0);

    /* create a set of 4 xstreams using a shared pool */
    ret = ABT_snoozer_xstream_create(4, &shared_pool, xstreams);
    assert(ret == 0);

    /* launch 4 ULTs on shared pool that will do nothing except sleep() */
    for(i=0; i<4; i++)
    {
        ABT_eventual_create(sizeof(*eret), &eventuals[i]);
        ABT_thread_create(shared_pool, thread_fn, &eventuals[i], ABT_THREAD_ATTR_NULL, &tids[i]);
    }

    /* wait on eventuals */
    for(i=0; i<4; i++)
    {
        ABT_eventual_wait(eventuals[i], (void**)&eret);
        assert(*eret == 0);
    }

    /* wait on the ULTs to complete */
    for(i=0; i<4; i++)
    {
        ABT_thread_join(tids[i]);
        ABT_thread_free(&tids[i]);
    }

    /* wait on the ESs to complete */
    for(i=0; i<4; i++)
    {
        ABT_xstream_join(xstreams[i]);
        ABT_xstream_free(&xstreams[i]);
    }

    ABT_finalize();

    return(0);
}

