/**
 * Membrane Common C routines: RingBuffer.
 *
 * All Rights Reserved, (c) 2016 Marcin Lewandowski
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
