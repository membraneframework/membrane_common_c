#include "raw_audio.h"

bool architecture_le()
{
  short int word = 0x0001;
  char *b = (char *)&word;
  return (b[0] ? true : false);
}

void *reverse_memcpy(void *dest, const void *src, size_t len)
{
  char *curr_dest = dest;
  const char *curr_src = src + len;
  while (len--)
    *curr_dest++ = *curr_src--;
  return dest;
}

/**
 * Converts one raw sample into its numeric value, interpreting it for given
 * format.
 */
int64_t raw_audio_sample_to_value(uint8_t *sample, RawAudio *raw_audio)
{

  bool is_architecture_le = architecture_le();
  bool is_format_le =
      (raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_ENDIANITY) ==
      MEMBRANE_SAMPLE_FORMAT_ENDIANITY_LE;
  uint8_t size = raw_audio_sample_byte_size(raw_audio);
  union Value ret;
  ret.u_val = 0;

  if (is_architecture_le && is_format_le)
  {
    memcpy(&(ret.u_val), sample, size);
  }
  else if (!is_architecture_le && !is_format_le)
  {
    reverse_memcpy(&(ret.u_val), sample, size);
  }
  else if (!is_architecture_le && is_format_le)
  {
    for (int8_t i = size - 1; i >= 0; --i)
    {
      ret.u_val <<= 8;
      ret.u_val += sample[i];
    }
  }
  else
  {
    for (int8_t i = 0; i < size; ++i)
    {
      ret.u_val <<= 8;
      ret.u_val += sample[i];
    }
  }

  uint32_t pad_left = MAX_SIZE - size;
  ret.u_val <<= 8 * pad_left;

  bool is_signed = raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_TYPE;
  if (is_signed)
  {
    return (int64_t)(ret.s_val >> 8 * pad_left);
  }
  else
  {
    return (int64_t)(ret.u_val >> 8 * pad_left);
  }
}

/**
 * Converts value into one raw sample, encoding it in given format.
 */
void raw_audio_value_to_sample(int64_t value, uint8_t *sample,
                               RawAudio *raw_audio)
{
  bool is_signed = raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_TYPE;
  union Value ret;

  if (is_signed)
  {
    ret.s_val = (int32_t)value;
  }
  else
  {
    ret.u_val = (uint32_t)value;
  }

  bool is_format_le =
      (raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_ENDIANITY) ==
      MEMBRANE_SAMPLE_FORMAT_ENDIANITY_LE;
  bool is_architecture_le = architecture_le();
  uint8_t size = raw_audio_sample_byte_size(raw_audio);

  if (is_architecture_le && is_format_le)
  {
    memcpy(sample, &(ret.u_val), size);
  }
  else if (!is_architecture_le && !is_format_le)
  {
    reverse_memcpy(sample, &(ret.u_val), size);
  }
  else if (!is_architecture_le && is_format_le)
  {
    for (uint8_t i = 0; i < size; ++i)
    {
      sample[i] = ret.u_val & 0xFF;
      ret.u_val >>= 8;
    }
  }
  else
  {
    for (int8_t i = size - 1; i >= 0; --i)
    {
      sample[i] = ret.u_val & 0xFF;
      ret.u_val >>= 8;
    }
  }
}

/**
 * Returns maximum sample value for given format.
 */
int64_t raw_audio_sample_max(RawAudio *raw_audio)
{
  bool is_signed = raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_TYPE;
  uint32_t size = raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_SIZE;
  if (is_signed)
  {
    return (1 << (size - 1)) - 1;
  }
  else
  {
    return (1 << size) - 1;
  }
}

/**
 * Returns minimum sample value for given format.
 */
int64_t raw_audio_sample_min(RawAudio *raw_audio)
{
  bool is_signed = raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_TYPE;

  if (is_signed)
  {
    uint32_t size = raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_SIZE;
    return -(1 << (size - 1));
  }
  else
  {
    return 0;
  }
}

/**
 * Returns byte size for given format.
 */
uint8_t raw_audio_sample_byte_size(RawAudio *raw_audio)
{
  return (uint8_t)((raw_audio->sample_format & MEMBRANE_SAMPLE_FORMAT_SIZE) / 8);
}
