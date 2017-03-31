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

#include "ringbuffer.h"

/**
 * Initializes new ring buffer of given size.
 *
 * It allows lock-free concurrent operation by one consumer and one producer.
 *
 * Size must be 2-exponent.
 *
 * It should be freed using membrane_ringbuffer_destroy after usage.
 */
MembraneRingBuffer* membrane_ringbuffer_new(size_t size) {
  MembraneRingBuffer *ringbuffer = enif_alloc(sizeof(MembraneRingBuffer));
  ringbuffer->queue = NULL;
  ringbuffer->queue = spsc_queue_init(ringbuffer->queue, size, &memtype_heap);

  return ringbuffer;
}


/**
 * Pushes an item to the ringbuffer.
 *
 * Returns 1 if it was written properly, 0 otherwise.
 */
int membrane_ringbuffer_push(MembraneRingBuffer* ringbuffer, MembraneRingBufferItem* item) {
  return spsc_queue_push_many(ringbuffer->queue, &item, 1);
}


/**
 * Pushes an item that is constructed upon erlang binary to the ringbuffer.
 *
 * Returns 1 if it was written properly, 0 otherwise.
 */
int membrane_ringbuffer_push_from_binary(MembraneRingBuffer* ringbuffer, ErlNifBinary* binary) {
  MembraneRingBufferItem *item = membrane_ringbuffer_item_new_from_binary(binary);
  // TODO constructor check

  return membrane_ringbuffer_push(ringbuffer, item);
}


/**
 * Returns ringbuffer's capacity.
 */
size_t membrane_ringbuffer_get_capacity(MembraneRingBuffer* ringbuffer) {
  return ringbuffer->queue->capacity;
}


/**
 * Returns ringbuffer's available slots.
 */
size_t membrane_ringbuffer_get_available(MembraneRingBuffer* ringbuffer) {
  return (size_t) spsc_queue_available(ringbuffer->queue);
}


/**
 * Pulls one item from the ringbuffer.
 *
 * Returns item if it was read properly, NULL otherwise.
 */
MembraneRingBufferItem* membrane_ringbuffer_pull(MembraneRingBuffer* ringbuffer) {
  MembraneRingBufferItem* item = NULL;

  if(spsc_queue_pull(ringbuffer->queue, &item) == 1) {
    return item;
  } else {
    return NULL;
  }
}


/**
 * Destroys given ring buffer.
 *
 * It includes destroying all items in the ring buffer.
 */
void membrane_ringbuffer_destroy(MembraneRingBuffer* ringbuffer) {
  // Destroy items in the queue
  MembraneRingBufferItem *item = membrane_ringbuffer_pull(ringbuffer);
  while(item) {
    membrane_ringbuffer_item_destroy(item);
    item = membrane_ringbuffer_pull(ringbuffer);
  }

  // Destroy the queue
  spsc_queue_destroy(ringbuffer->queue);

  // Destroy itself
  enif_free(ringbuffer);
}


/**
 * Initializes new ring buffer item from given erlang binary.
 *
 * The data is copied from the binary so it is free to be released after
 * this operation.
 *
 * It should be freed using membrane_ringbuffer_item_destroy after usage.
 */
MembraneRingBufferItem* membrane_ringbuffer_item_new_from_binary(ErlNifBinary* binary) {
  MembraneRingBufferItem *item = enif_alloc(sizeof(MembraneRingBufferItem));
  // TODO malloc error check

  item->data = enif_alloc(binary->size);
  memcpy(item->data, binary->data, binary->size);
  item->size = binary->size;

  return item;
}


/**
 * Destroys given ring buffer item.
 */
void membrane_ringbuffer_item_destroy(MembraneRingBufferItem* item) {
  // Destroy data in the item
  enif_free(item->data);

  // Destroy itself
  enif_free(item);
}
