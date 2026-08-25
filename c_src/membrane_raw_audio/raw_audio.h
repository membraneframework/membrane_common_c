#include <membrane/membrane.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_SIZE 4

union Value {
  int32_t s_val;
  uint32_t u_val;
};

struct _RawAudio {
  uint32_t channels;
  uint32_t sample_format;
  uint32_t sample_rate;
};
typedef struct _RawAudio RawAudio;

int64_t raw_audio_sample_to_value(uint8_t *sample, RawAudio *raw_audio);
void raw_audio_value_to_sample(int64_t value, uint8_t *sample,
                               RawAudio *raw_adio);
int64_t raw_audio_sample_max(RawAudio *raw_audio);
int64_t raw_audio_sample_min(RawAudio *raw_audio);
uint8_t raw_audio_sample_byte_size(RawAudio *raw_audio);
