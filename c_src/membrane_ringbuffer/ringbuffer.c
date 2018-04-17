/**
 * Membrane Common C routines: RingBuffer.
 *
 * All Rights Reserved, (c) 2016 Marcin Lewandowski
 */

#include "ringbuffer.h"

/**
 * Initializes new ring buffer of given size.
 *
 * Size must be power of 2.
 *
 * It allows lock-free concurrent operation by one consumer and one producer.
 *
 * It should be freed using membrane_ringbuffer_destroy after usage.
 */
MembraneRingBuffer* membrane_ringbuffer_new(size_t element_count, size_t element_size) {
  MembraneRingBuffer *ringbuffer = enif_alloc(sizeof(MembraneRingBuffer));
  ringbuffer->data = enif_alloc(element_count * element_size);
  ringbuffer->ringbuffer = enif_alloc(sizeof(PaUtilRingBuffer));
  if(PaUtil_InitializeRingBuffer(ringbuffer->ringbuffer, element_size, element_count, ringbuffer->data)) {
      return NULL;
  }

  return ringbuffer;
}


/**
 * Writes at most `cnt` elements to the ringbuffer.
 *
 * Returns the actual number of elements copied.
 */
size_t membrane_ringbuffer_write(MembraneRingBuffer* ringbuffer, void *src, size_t cnt) {
  return PaUtil_WriteRingBuffer(ringbuffer->ringbuffer, src, cnt);
}


/**
 * Returns the number of ringbuffer's available elements for read.
 */
size_t membrane_ringbuffer_get_read_available(MembraneRingBuffer* ringbuffer) {
  return PaUtil_GetRingBufferReadAvailable(ringbuffer->ringbuffer);
}


/**
 * Returns the number of ringbuffer's available elements for write.
 */
size_t membrane_ringbuffer_get_write_available(MembraneRingBuffer* ringbuffer) {
  return PaUtil_GetRingBufferWriteAvailable(ringbuffer->ringbuffer);
}


/**
 * Reads at most `cnt` elements from the ringbuffer.
 *
 * Returns the actual number of elements copied.
 */
size_t membrane_ringbuffer_read(MembraneRingBuffer* ringbuffer, void *dest, size_t cnt) {
  return PaUtil_ReadRingBuffer(ringbuffer->ringbuffer, dest, cnt);
}


/**
 * Reset the read and write pointers to zero. This is not thread safe.
 */
void membrane_ringbuffer_cleanup(MembraneRingBuffer* ringbuffer) {
  PaUtil_FlushRingBuffer(ringbuffer->ringbuffer);
}


/**
 * Destroys given ring buffer.
 *
 */
void membrane_ringbuffer_destroy(MembraneRingBuffer* ringbuffer) {
  enif_free(ringbuffer->ringbuffer);
  enif_free(ringbuffer->data);
  enif_free(ringbuffer);
}
