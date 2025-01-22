/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2023 Impinj, Inc. All rights reserved.                      *
 *                                                                           *
 *****************************************************************************/


#include <assert.h>
#include "board/ex10_osal.h"
#include <zephyr/kernel.h>

// Max mutexes and conditional variables that are available.
#define MAX_RTOS_RESOURCE_ID 8

static int mutexes_allocated = 0;
static struct k_mutex mutexes[MAX_RTOS_RESOURCE_ID];

static int condvars_allocated = 0;
static struct k_condvar condvars[MAX_RTOS_RESOURCE_ID];

static struct k_mutex* get_mutex(ex10_mutex_t* mutex) {
    // Alloc and init mutex if not done yet
    if (!*mutex) {
        // Out of resources?
        assert(mutexes_allocated < MAX_RTOS_RESOURCE_ID);
        *mutex = ++mutexes_allocated;
        k_mutex_init(&mutexes[*mutex - 1]);
    }
    return &mutexes[*mutex - 1];
}

static struct k_condvar* get_condvar(ex10_cond_t* cond) {
    // Alloc and init condvar if not done yet
    if (!*cond) {
        // Out of resources?
        assert(condvars_allocated < MAX_RTOS_RESOURCE_ID);
        *cond = ++condvars_allocated;
        k_condvar_init(&condvars[*cond - 1]);
    }
    return &condvars[*cond - 1];
}

int ex10_mutex_lock(ex10_mutex_t* mutex) 
{
    return k_mutex_lock(get_mutex(mutex), K_FOREVER);
}

int ex10_mutex_unlock(ex10_mutex_t* mutex) 
{
    return k_mutex_unlock(get_mutex(mutex));
}

int ex10_cond_signal(ex10_cond_t* cond) 
{
    return k_condvar_broadcast(get_condvar(cond));
}

int ex10_cond_wait(ex10_cond_t* cond, ex10_mutex_t* mutex) 
{
    return k_condvar_wait(get_condvar(cond), get_mutex(mutex), K_FOREVER);
}

int ex10_cond_timed_wait_us(ex10_cond_t*  cond,
                            ex10_mutex_t* mutex,
                            uint32_t      timeout_us) 
{
    return k_condvar_wait(get_condvar(cond), get_mutex(mutex), K_USEC(timeout_us));
}

int ex10_memcpy(void* dst_ptr, size_t dst_size, const void* src_ptr, size_t src_size) {
        if (src_size <= dst_size)
    {
        uint8_t*       dst_byte_ptr = (uint8_t*)dst_ptr;
        uint8_t const* src_byte_ptr = (uint8_t const*)src_ptr;
        for (size_t index = 0u; index < src_size; ++index)
        {
            dst_byte_ptr[index] = src_byte_ptr[index];
        }
        return 0;
    }
    else
    {
        ex10_memzero(dst_ptr, dst_size);
        return EINVAL;
    }
}

int  ex10_memset(void* dst_ptr, size_t dst_size, int value, size_t count) {
    if (count <= dst_size)
    {
        uint8_t* dst_byte_ptr = (uint8_t*)dst_ptr;
        for (size_t index = 0u; index < count; ++index)
        {
            dst_byte_ptr[index] = (uint8_t)value;
        }
        return 0;
    }
    else
    {
        ex10_memzero(dst_ptr, dst_size);
        return EINVAL;
    }
}

void ex10_memzero(void* dst_ptr, size_t dst_size) {
    uint8_t* dst_byte_ptr = (uint8_t*)dst_ptr;
    for (size_t index = 0u; index < dst_size; ++index)
    {
        dst_byte_ptr[index] = 0u;
    }
}

