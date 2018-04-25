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
#include "portaudio/pa_ringbuffer.h"


typedef struct _MembraneRingBuffer MembraneRingBuffer;
struct _MembraneRingBuffer
{
  struct PaUtilRingBuffer  *ringbuffer;
  void                     *data;
  size_t                    element_size;
};

MembraneRingBuffer* membrane_ringbuffer_new(size_t element_count, size_t element_size);
size_t membrane_ringbuffer_write(MembraneRingBuffer* ringbuffer, void *src, size_t cnt);
size_t membrane_ringbuffer_get_write_available(MembraneRingBuffer* ringbuffer);
size_t membrane_ringbuffer_get_read_available(MembraneRingBuffer* ringbuffer);
size_t membrane_ringbuffer_read(MembraneRingBuffer* ringbuffer, void *dest, size_t cnt);
void membrane_ringbuffer_cleanup(MembraneRingBuffer* ringbuffer);
void membrane_ringbuffer_destroy(MembraneRingBuffer* ringbuffer);



#endif
