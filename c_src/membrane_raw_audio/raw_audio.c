#include "raw_audio.h"

bool architecture_le() {
  int16_t word = 0x0001;
  char *b = (char *)&word;
  return b[0] == 0x01;
}

void *reverse_memcpy(void *dest, const void *src, size_t len) {
  char *curr_dest = dest;
  const char *curr_src = src + len - 1;
  while (len--)
    *curr_dest++ = *curr_src--;
  return dest;
}

/**
 * Converts one raw sample into its numeric value, interpreting it for given
 * format.
 */
int64_t raw_audio_sample_to_value(uint8_t *sample, RawAudio *raw_audio) {
  bool is_architecture_le = architecture_le();
  bool is_format_le =
      (raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_ENDIANITY) ==
      MEMBRANE_SAMPLE_FORMAT_ENDIANITY_LE;
  uint8_t size = raw_audio_sample_byte_size(raw_audio);
  union Value ret;
  ret.u_val = 0;

  if (is_architecture_le == is_format_le) {
    memcpy(&(ret.u_val), sample, size);
  } else {
    reverse_memcpy(&(ret.u_val), sample, size);
  }

  bool is_signed = raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_TYPE;

  if (is_signed) {

    /**
     * As the size of a sample might be smaller than the size of a ret.s_val
     * we must bit shift to have a sign bit as a first bit.
     */
    uint32_t pad_left = MAX_SIZE - size;
    ret.u_val <<= 8 * pad_left;
    return (int64_t)(ret.s_val >> 8 * pad_left);
  } else {
    return (int64_t)ret.u_val;
  }
}

/**
 * Converts value into one raw sample, encoding it in given format.
 */
void raw_audio_value_to_sample(int64_t value, uint8_t *sample,
                               RawAudio *raw_audio) {
  bool is_signed = raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_TYPE;
  union Value ret;

  if (is_signed) {
    ret.s_val = (int32_t)value;
  } else {
    ret.u_val = (uint32_t)value;
  }

  bool is_format_le =
      (raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_ENDIANITY) ==
      MEMBRANE_SAMPLE_FORMAT_ENDIANITY_LE;
  bool is_architecture_le = architecture_le();
  uint8_t size = raw_audio_sample_byte_size(raw_audio);

  if (is_architecture_le == is_format_le) {
    memcpy(sample, &(ret.u_val), size);
  } else {
    reverse_memcpy(sample, &(ret.u_val), size);
  }
}

/**
 * Returns maximum sample value for given format.
 */
int64_t raw_audio_sample_max(RawAudio *raw_audio) {
  bool is_signed = raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_TYPE;
  uint32_t size = raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_SIZE;
  if (is_signed) {
    return (1 << (size - 1)) - 1;
  } else {
    return (1 << size) - 1;
  }
}

/**
 * Returns minimum sample value for given format.
 */
int64_t raw_audio_sample_min(RawAudio *raw_audio) {
  bool is_signed = raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_TYPE;

  if (is_signed) {
    uint32_t size = raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_SIZE;
    return -(1 << (size - 1));
  } else {
    return 0;
  }
}

/**
 * Returns byte size for given format.
 */
uint8_t raw_audio_sample_byte_size(RawAudio *raw_audio) {
  return (uint8_t)((raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_SIZE) /
                   8);
}
