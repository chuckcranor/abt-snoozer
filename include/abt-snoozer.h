/*
 * (C) 2015 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */

#ifndef __ABT_SNOOZER
#define __ABT_SNOOZER

#include <abt.h>

/**
 * Creates a new argobots xstream with a pool and scheduler defined to sleep
 * when idle. 
 * @param [out] pool pool associated with new xstream
 * @returns 0 on success, -1 upon error
 */
int ABT_snoozer_xstream_create(ABT_pool *newpool);

/**
 * Replaces pool and scheduler in xstream of caller with pool and scheduler
 * that will sleep when idle.
 * 
 * @param [out] pool new pool associated with caller's xstream
 * @returns 0 on success, -1 upon error
 */
int ABT_snoozer_xstream_self_set(ABT_pool *newpool);

#endif /* __ABT_SNOOZER */