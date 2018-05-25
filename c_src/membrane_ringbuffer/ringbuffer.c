/**
 * Membrane Common C routines: RingBuffer.
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
MembraneRingBuffer* membrane_ringbuffer_new(size_t max_elements, size_t element_size) {
  // Make sure if max_elements is a power of 2 before continuing
  if(((max_elements-1) & max_elements) != 0) {
    return NULL;
  }
  MembraneRingBuffer *ringbuffer = enif_alloc(sizeof(MembraneRingBuffer));
  ringbuffer->buffer = enif_alloc(max_elements * element_size);
  /*
   * write_index and read_index will be kept modulo (max_elements * 2)
   * while actual indices for read/write will be calculated using modulo (max_elements)
   * When max_elements is a power of 2 this will not break modulo (max_elements) arithmetic
   * while allowing to distinguish full ringbuffer from empty.
   */
  ringbuffer->read_index = 0;
  ringbuffer->write_index = 0;
  ringbuffer->element_size = element_size;
  ringbuffer->max_elements = max_elements;

  return ringbuffer;
}


/**
 * Writes at most `cnt` elements to the ringbuffer.
 *
 * Returns the actual number of elements copied.
 */
size_t membrane_ringbuffer_write(MembraneRingBuffer* ringbuffer, void *src, size_t cnt) {
  size_t available = membrane_ringbuffer_get_write_available(ringbuffer);
  cnt = cnt > available ? available : cnt;

  size_t index = atomic_load(&ringbuffer->write_index) % ringbuffer->max_elements;
  void * dest = ringbuffer->buffer + (index * ringbuffer->element_size);
  // if the write will not be contiguous
  if (index + cnt > ringbuffer->max_elements) {
    size_t tail_items = ringbuffer->max_elements - index;

    size_t copy_size = tail_items * ringbuffer->element_size;
    memcpy(dest, src, copy_size);

    dest = ringbuffer->buffer;
    src  = src + (copy_size * ringbuffer->element_size);
    copy_size = (cnt - tail_items) * ringbuffer->element_size;
    memcpy(dest, src, copy_size);
  } else {
    size_t copy_size = cnt * ringbuffer->element_size;
    memcpy(dest, src, copy_size);
  }

  ringbuffer->write_index = (atomic_load(&ringbuffer->write_index) + cnt) % (ringbuffer->max_elements * 2);
  return cnt;
}


/**
 * Returns the number of ringbuffer's available elements for read.
 */
size_t membrane_ringbuffer_get_read_available(MembraneRingBuffer* ringbuffer) {
  return (atomic_load(&ringbuffer->write_index) - atomic_load(&ringbuffer->read_index)) % (ringbuffer->max_elements * 2);
}


/**
 * Returns the number of ringbuffer's available elements for write.
 */
size_t membrane_ringbuffer_get_write_available(MembraneRingBuffer* ringbuffer) {
  return ringbuffer->max_elements - membrane_ringbuffer_get_read_available(ringbuffer);
}


/**
 * Reads at most `cnt` elements from the ringbuffer.
 *
 * Returns the actual number of elements copied.
 */
size_t membrane_ringbuffer_read(MembraneRingBuffer* ringbuffer, void *dest, size_t cnt) {
  size_t available = membrane_ringbuffer_get_read_available(ringbuffer);
  cnt = cnt > available ? available : cnt;

  size_t index = atomic_load(&ringbuffer->read_index) % ringbuffer->max_elements;
  void * src = ringbuffer->buffer + (index * ringbuffer->element_size);
  // if the read will not be contiguous
  if (index + cnt > ringbuffer->max_elements) {
    size_t tail_items = ringbuffer->max_elements - index;

    size_t copy_size = tail_items * ringbuffer->element_size;
    memcpy(dest, src, copy_size);

    dest = ringbuffer->buffer;
    src  = src + (copy_size * ringbuffer->element_size);
    copy_size = (cnt - tail_items) * ringbuffer->element_size;
    memcpy(dest, src, copy_size);
  } else {
    size_t copy_size = cnt * ringbuffer->element_size;
    memcpy(dest, src, copy_size);
  }

  ringbuffer->read_index = (atomic_load(&ringbuffer->read_index) + cnt) % (ringbuffer->max_elements * 2);
  return cnt;
}


/**
 * Reset the read and write pointers to zero. This is not thread safe.
 */
void membrane_ringbuffer_cleanup(MembraneRingBuffer* ringbuffer) {
  ringbuffer->write_index = 0;
  ringbuffer->read_index = 0;
}


/**
 * Destroys given ring buffer.
 *
 */
void membrane_ringbuffer_destroy(MembraneRingBuffer* ringbuffer) {
  enif_free(ringbuffer->buffer);
  enif_free(ringbuffer);
}
