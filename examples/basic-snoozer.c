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

/* Expected behavior: the primary ES will block on ABT_thread_join(), waiting
 * for the ULT (thread_fn()) to complete.  The primary ES has nothing else to
 * execute during this time.  The snoozer library is used to make sure that
 * both execution streams will sleep while idle.
 */

void thread_fn(void *_arg)
{
    ABT_eventual *eventual = _arg;
    int ret = 0;

    /* NOTE: this will block whatever ES it executes on */
    sleep(5);

    /* set eventual when done */
    ABT_eventual_set(*eventual, &ret, sizeof(ret));

    return;
}

int main(int argc, char **argv) 
{
    int ret;
    int *eret;
    ABT_thread tid;
    ABT_pool pool2;
    ABT_xstream xstream2;
    ABT_eventual eventual;
    
    ret = ABT_init(argc, argv);
    if(ret != 0)
    {
        fprintf(stderr, "Error: ABT_init()\n");
        return(-1);
    }

    /* set primary ES to use the snoozer */
    ret = ABT_snoozer_xstream_self_set();
    assert(ret == 0);

    /* create one additional ES using the snoozer */
    ret = ABT_snoozer_xstream_create(1, &pool2, &xstream2);
    assert(ret == 0);

    /* launch a ULT on the new ES that will do nothing except sleep() */
    ABT_eventual_create(sizeof(*eret), &eventual);
    ABT_thread_create(pool2, thread_fn, &eventual, ABT_THREAD_ATTR_NULL, &tid);

    /* wait on eventual */
    ABT_eventual_wait(eventual, (void**)&eret);
    assert(*eret == 0);

    /* wait on the ULT to complete */
    ABT_thread_join(tid);
    ABT_thread_free(&tid);

    /* wait on the ES to complete */
    ABT_xstream_join(xstream2);
    ABT_xstream_free(&xstream2);

    ABT_eventual_free(&eventual);

    ABT_finalize();

    return(0);
}

