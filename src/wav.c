#include "wav.h"
#include <stdio.h>
#include <stdlib.h>

wav_error wav_metadata_read(wav_metadata *mp, FILE *fp) {
    if (mp == NULL || fp == NULL)
        return WAV_INVALID_INPUT;

    // Обращаю внимание на то, что считывается не вся информация из файла,
    // поэтому считывает сдвигась только к нужной информации
    if (fseek(fp, 4, SEEK_SET) != 0 ||
        fread(&mp->file_size, sizeof(mp->file_size), 1, fp) != 1 ||

        // Тут читаются сразу два значения, поскольку находятся рядом
        fseek(fp, 22, SEEK_SET) != 0 ||
        fread(&mp->channels_count, sizeof(mp->channels_count), 1, fp) != 1 ||
        fread(&mp->sample_rate, sizeof(mp->sample_rate), 1, fp) != 1 ||

        fseek(fp, 34, SEEK_SET) != 0 ||
        fread(&mp->bit_depth, sizeof(mp->bit_depth), 1, fp) != 1 ||

        fseek(fp, 40, SEEK_SET) != 0 ||
        fread(&mp->data_size, sizeof(mp->data_size), 1, fp) != 1)
        return WAV_READ_FILE_ERR;
    return WAV_OK;
}

wav_error wav_raw_data_read(wav_raw_data *rp, const wav_metadata *const mp, FILE *fp) {
    if (rp == NULL || mp == NULL || fp == NULL)
        return WAV_INVALID_INPUT;

    rp->byte_depth = mp->bit_depth / 8;
    rp->size       = mp->data_size / rp->byte_depth;

    rp->bytes = calloc(mp->data_size, sizeof(uint8_t));
    if (rp->bytes == NULL)
        return WAV_MEMORY_ERR;
    if (fseek(fp, 44, SEEK_SET) != 0 || fread(rp->bytes, sizeof(uint8_t) * mp->data_size, 1, fp) != 1)
        return WAV_READ_FILE_ERR;
    return WAV_OK;
}

wav_error wav_read(wav_metadata *mp, wav_raw_data *rp, FILE *fp) {
    wav_error err;

    if (rp == NULL || mp == NULL || fp == NULL)
        return WAV_INVALID_INPUT;

    if ((err = wav_metadata_read(mp, fp)) != WAV_OK)
        return err;
    if ((err = wav_raw_data_read(rp, mp, fp)) != WAV_OK)
        return err;
    return WAV_OK;
}


// Преобразование байтов в аудиосемпл
wav_error wav_bytes_to_sample(float *samplep, const uint8_t *const bytes, size_t byte_depth) {
    uint32_t sample, max;

    // Поскольку большинство WAV файлов имеют битовую глубину меньше 32-х, то
    // я выбрал uint32_t для преобразования байтов в числовое значение.
    if (samplep == NULL || bytes == NULL || byte_depth > 4 || byte_depth == 0)
        return WAV_INVALID_INPUT;

    // Максимальное число, которое можно получить из такого количества байт
    max = (1 << (byte_depth * 8)) - 1;

    // Конвертация в порядке little-endian
    sample = 0;
    for (int i = byte_depth - 1; i >= 0; --i)
        sample = (sample << 8) | bytes[i];

    // Представление аудиосемпла в диапазоне от -1 до 1
    *samplep = 2 * ((float) sample / max) - 1;
    return WAV_OK;
}

wav_error wav_data_parse(wav_data *dp, const wav_raw_data *const rp, const wav_metadata *const mp) {
    wav_error err;
    int i, j;

    if (dp == NULL || rp == NULL || mp == NULL)
        return WAV_INVALID_INPUT;

    dp->len     = rp->size;
    dp->samples = calloc(dp->len, sizeof(float));
    if (dp->samples == NULL)
        return WAV_MEMORY_ERR;
    
    i = 0;
    for (j = 0; j < mp->data_size; j += rp->byte_depth) {
        err = wav_bytes_to_sample(dp->samples + i, rp->bytes + j, rp->byte_depth);
        if (err != WAV_OK) {
            free(dp->samples);
            return err;
        }
        i++;
    }
    return WAV_OK;
}

