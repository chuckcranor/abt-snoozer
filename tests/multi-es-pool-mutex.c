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

struct thread_arg
{
    ABT_mutex *mutex;
    int index;
};

void thread_fn(void *_arg)
{
    struct thread_arg *arg = _arg;
    
    ABT_mutex_lock(*arg->mutex);
    fprintf(stderr, "ULT %d acquired lock.\n", arg->index);
    /* NOTE: this will block whatever ES it executes on */
    usleep(10);
    ABT_mutex_unlock(*arg->mutex);
    fprintf(stderr, "ULT %d released lock.\n", arg->index);

    return;
}

#define N_ES 7
#define N_ULT 10

int main(int argc, char **argv) 
{
    int ret;
    ABT_thread tid_array[N_ULT];
    struct thread_arg arg_array[N_ULT];
    ABT_pool pool2;
    ABT_xstream xstream2_array[N_ES];
    ABT_mutex mutex;
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

    /* create seven additional ES's in one pool with snoozer */
    ret = ABT_snoozer_xstream_create(N_ES, &pool2, xstream2_array);
    assert(ret == 0);

    ABT_mutex_create(&mutex);

    for(i=0; i<N_ULT; i++)
    {
        arg_array[i].mutex = &mutex;
        arg_array[i].index = i;
        ABT_thread_create(pool2, thread_fn, &arg_array[i], ABT_THREAD_ATTR_NULL, &tid_array[i]);
    }

    for(i=0; i<N_ULT; i++)
    {
        /* wait on the ULT to complete */
        ABT_thread_join(tid_array[i]);
        ABT_thread_free(&tid_array[i]);
    }

    /* wait on the ES's to complete */
    for(i=0; i<N_ES; i++)
    {
        ABT_xstream_join(xstream2_array[i]);
        ABT_xstream_free(&xstream2_array[i]);
    }

    ABT_finalize();

    return(0);
}

