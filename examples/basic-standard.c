/*
 * (C) 2015 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <abt.h>

/* Expected behavior: the primary ES will block on ABT_thread_join(), waiting
 * for the ULT (thread_fn()) to complete.  The primary ES has nothing else to
 * execute during this time, but it will busy spin anyway because that is the
 * default ABT scheduler behavior.
 */

void thread_fn(void *arg)
{
    /* NOTE: this will block whatever ES it executes on */
    sleep(5);
    return;
}

int main(int argc, char **argv) 
{
    int ret;
    ABT_thread tid;
    ABT_pool pool2;
    ABT_xstream xstream2;
    
    ret = ABT_init(argc, argv);
    if(ret != 0)
    {
        fprintf(stderr, "Error: ABT_init()\n");
        return(-1);
    }

    /* create one additional ES */
    ABT_xstream_create(ABT_SCHED_NULL, &xstream2);
    ABT_xstream_get_main_pools(xstream2, 1, &pool2);

    /* launch a ULT on the new ES that will do nothing except sleep() */
    ABT_thread_create(pool2, thread_fn, NULL, ABT_THREAD_ATTR_NULL, &tid);

    /* wait on the ULT to complete */
    ABT_thread_join(tid);
    ABT_thread_free(&tid);

    /* wait on the ES to complete */
    ABT_xstream_join(xstream2);
    ABT_xstream_free(&xstream2);

    ABT_finalize();

    return(0);
}

