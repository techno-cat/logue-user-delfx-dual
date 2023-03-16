/*
Copyright 2023 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include <stdint.h>
#include "LCWCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LCW_DELAY_BUFFER_DEC(p) ( ((p)->pointer - 1) & (p)->mask )
#define LCW_DELAY_BUFFER_LUT(p, i) ( (p)->buffer[((p)->pointer + (i)) & (p)->mask] )

// Combフィルタ
typedef struct {
    int32_t *buffer;
    uint32_t size;
    uint32_t mask;
    int32_t pointer;
    int32_t fbGain;
    int32_t offset;
    int32_t out;
} LCWDelayBuffer;

// オールパスフィルタ
typedef struct {
    int32_t *buffer;
    uint32_t size;
    uint32_t mask;
    int32_t pointer;
    int32_t gain;
    int32_t offset;
    int32_t out;
} LCWAPFBuffer;

#ifdef __cplusplus
}
#endif
