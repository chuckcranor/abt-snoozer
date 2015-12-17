/*
 * (C) 2015 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */

#ifndef __ABT_SNOOZER
#define __ABT_SNOOZER

#ifdef __cplusplus
extern "C" {
#endif

#include <abt.h>

/**
 * Creates a new argobots xstream with a pool and scheduler defined to sleep
 * when idle. 
 * @param [in] num_xstreams number of xstreams to create
 * @param [out] newpool single pool associated with new xstreams
 * @param [out] newxstreams newly created xstreams
 * @returns 0 on success, -1 upon error
 */
int ABT_snoozer_xstream_create(int num_xstreams, ABT_pool *newpool, ABT_xstream *newxstreams);

/**
 * Replaces pool and scheduler in xstream of caller with pool and scheduler
 * that will sleep when idle.
 * 
 * @returns 0 on success, -1 upon error
 */
int ABT_snoozer_xstream_self_set(void);

#ifdef __cplusplus
}
#endif

#endif /* __ABT_SNOOZER */
