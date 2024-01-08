#ifndef _WAV_H
#define _WAV_H

#include <stdint.h>

typedef enum {
    WAV_OK = 0,
    WAV_INVALID_INPUT,
    WAV_MEMORY_ERR,
    WAV_READ_FILE_ERR,
} wav_error;

typedef struct {
    uint32_t file_size;      // Размер всего файла
    uint16_t channels_count; // Количество каналов
    uint32_t sample_rate;    // Частота
    uint8_t  bit_depth;      // Битовая глубина - количество бит для одного аудиосемпла
    uint32_t data_size;      // Размер секции с аудиосемплами
} wav_metadata;

wav_error wav_metadata_read(wav_metadata *mp, FILE *fp);

typedef struct {
    uint8_t *bytes;
    size_t   byte_depth;
    size_t   size;
} wav_raw_data;

wav_error wav_raw_data_read(wav_raw_data *rp, const wav_metadata *const mp, FILE *fp);

wav_error wav_read(wav_metadata *mp, wav_raw_data *rp, FILE *fp);

typedef struct {
    float *samples;
    size_t len;
} wav_data;

wav_error wav_bytes_to_sample(float *samplep, const uint8_t *const bytes, size_t byte_depth);

wav_error wav_data_parse(wav_data *dp, const wav_raw_data *const rp, const wav_metadata *const mp);

#endif