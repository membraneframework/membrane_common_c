/**
 * Membrane Common C routines: RingBuffer.
 *
 * All Rights Reserved, (c) 2016 Marcin Lewandowski
 *
 * Based on c11-queues code by:
 *
 * - Umar Farooq	<umar1.farooq1@gmail.com>
 * - Steffen Vogel <post@steffenvogel.de>
 *
 * that is by itself licensed as the following:
 *
 * All rights reserved.
 *
 *  - Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MEMBRANE_RINGBUFFER_H__
#define __MEMBRANE_RINGBUFFER_H__

#include <stdio.h>
#include <string.h>
#include <erl_nif.h>
#include "c11-queues/spsc_queue.h"

typedef struct _MembraneRingBuffer MembraneRingBuffer;
struct _MembraneRingBuffer
{
  struct spsc_queue *queue; // single producer/single consumer queue
};


typedef struct _MembraneRingBufferItem MembraneRingBufferItem;
struct _MembraneRingBufferItem
{
  void   *data; // data itself
  size_t  size; // length of the data
};


MembraneRingBuffer* membrane_ringbuffer_new(size_t size);
int membrane_ringbuffer_push(MembraneRingBuffer* ringbuffer, MembraneRingBufferItem* item);
int membrane_ringbuffer_push_from_binary(MembraneRingBuffer* ringbuffer, ErlNifBinary* binary);
size_t membrane_ringbuffer_get_capacity(MembraneRingBuffer* ringbuffer);
size_t membrane_ringbuffer_get_available(MembraneRingBuffer* ringbuffer);
MembraneRingBufferItem* membrane_ringbuffer_pull(MembraneRingBuffer* ringbuffer);
void membrane_ringbuffer_destroy(MembraneRingBuffer* ringbuffer);

MembraneRingBufferItem* membrane_ringbuffer_item_new_from_binary(ErlNifBinary* binary);
void membrane_ringbuffer_item_destroy(MembraneRingBufferItem* item);

#endif
