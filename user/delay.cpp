/*
Copyright 2019 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include "userdelfx.h"
#include "buffer_ops.h"
#include "LCWDelayBuffer.h"
#include "LCWPow2.h"

// サンプリングバッファが2^18もあれば、BPM=30, time=x1.0に対応できる
// 最低でも96k個（= 48000 x 2）は欲しいが、130k個もあるので大丈夫
#define LCW_DELAY_SAMPLING_RATE (48000)
#define LCW_DELAY_BPM_MIN (30)
#define LCW_DELAY_BPM_MAX (480)

#define LCW_DELAY_BUFFER_MAX (2)
#define LCW_DELAY_SAMPLING_BUFFER_SIZE (1<<18)
#define LCW_DELAY_SAMPLING_BUFFER_TOTAL (LCW_DELAY_SAMPLING_BUFFER_SIZE * LCW_DELAY_BUFFER_MAX)

#define LCW_DELAY_TIME_PARAMS (10)

static __sdram int32_t s_delay_ram_sampling[LCW_DELAY_SAMPLING_BUFFER_TOTAL];
static LCWDelayBuffer delayBuffers[LCW_DELAY_BUFFER_MAX];

static float s_inputGain;
static float s_mix;
static float s_depth;
static float s_time;

static const float delayTimeParams[LCW_DELAY_TIME_PARAMS] = {
    1.f/8,  // 0.125
    .5f/3 , // 0.1666
    3.f/16, // 0.1875
    // ---------------
    1.f/4 , // 0.25
    1.f/3 , // 0.3333
    3.f/8 , // 0.375
    // ---------------
    1.f/2 , // 0.5
    2.f/3 , // 0.6666
    3.f/4 , // 0.75
    // ---------------
    1.f
};

__fast_inline float softlimiter(float c, float x)
{
  float xf = si_fabsf(x);
  if ( xf < c ) {
    return x;
  }
  else {
    return si_copysignf( c + fx_softclipf(c, xf - c), x );
  }
}

void DELFX_INIT(uint32_t platform, uint32_t api)
{
  for (int32_t i=0; i<LCW_DELAY_BUFFER_MAX; i++) {
    LCWDelayBuffer *buf = &(delayBuffers[i]);
    buf->buffer = &(s_delay_ram_sampling[LCW_DELAY_SAMPLING_BUFFER_SIZE * i]);
    buf->size = LCW_DELAY_SAMPLING_BUFFER_SIZE;
    buf->mask = LCW_DELAY_SAMPLING_BUFFER_SIZE - 1;
    buf->pointer = 0;
    buf->fbGain = 0; // あとで
    buf->offset = 0; // あとで
    buf->out = 0;
  }

  s_mix = 0.5f;
  s_depth = 0.f;
  s_time = 0.f;
  s_inputGain = 0.f;
}

__fast_inline uint32_t calcDelaySamples(float bpm, float delayTime)
{
  // bps = bpm / 60
  // n = bps / delayTime
  // delaySamples = samplingRate / n
  //              = (samplingRate * delayTime) / bps
  //              = (samplingRate * delayTime * 60) / bpm
  return (uint32_t)((LCW_DELAY_SAMPLING_RATE * delayTime * 60.f) / bpm);
}

__fast_inline int32_t calcFbGain(float bpm, float delayTime)
{
  // samples = samplingRate / ((bpm / 60) / delayTime)
  // n = (samplingRate * time) / samples
  //   = (time * (bpm / 60)) / param
  // gain = 2 ^ (log2(0.001) / n)
  const float log2_0_001 = -9.965784284662087;
  const float time = 3.f; // time(sec)で十分に小さくなる設定
  const float n = (time * bpm) / (delayTime * 60.f);
  return LCWPow2( LCW_SQ7_24(log2_0_001 / n) );
}

__fast_inline int32_t delayMain(LCWDelayBuffer *p, int32_t in)
{
  p->pointer = LCW_DELAY_BUFFER_DEC(p);
  const int32_t zn = LCW_DELAY_BUFFER_LUT(p, p->offset);
#if 1
  p->buffer[p->pointer] =
    in + (int32_t)(((int64_t)zn * p->fbGain) >> 24);
#else
  p->buffer[p->pointer] = in;
#endif

  return zn;
}

void DELFX_PROCESS(float *xn, uint32_t frames)
{
  float bpm = fx_get_bpmf();
  bpm = clampfsel( (float)LCW_DELAY_BPM_MIN, bpm, (float)LCW_DELAY_BPM_MAX );

  const float params[] = { s_time, s_depth };
  for (int32_t i=0; i<LCW_DELAY_BUFFER_MAX; i++) {
    // 0.f-1.f -> 0..(n-1)
    const uint32_t j = (uint32_t)((params[i] * (LCW_DELAY_TIME_PARAMS - 1)) + .5f);
    const float delayTime = delayTimeParams[j];
    delayBuffers[i].offset = calcDelaySamples( bpm, delayTime );
    delayBuffers[i].fbGain = calcFbGain( bpm, delayTime );
  }

  float * __restrict x = xn;
  const float * x_e = x + 2*frames;

  const float dry = 1.f - s_mix;
  const float wet = s_mix;

  LCWDelayBuffer *buffer = &(delayBuffers[0]);
  for (; x != x_e; ) {
    float xL = *x;
    const int32_t inL = (int32_t)( s_inputGain * xL * (1 << 24) );
    int32_t outL = 0;
    outL += delayMain( buffer + 0, inL );
    outL += delayMain( buffer + 1, inL );

    float wL = outL / (float)(1 << 24);
    wL = softlimiter( 0.1f, wL ); // -> -1.0 .. +1.0

    float yL = (dry * xL) + (wet * wL);

    *(x++) = yL;
    *(x++) = yL;

    if ( s_inputGain < 0.99998f ) {
      s_inputGain += ( (1.f - s_inputGain) * 0.0625f );
    }
    else { s_inputGain = 1.f; }
  }
}

void DELFX_RESUME(void)
{
  buf_clr_u32(
    (uint32_t * __restrict__)s_delay_ram_sampling,
    LCW_DELAY_SAMPLING_BUFFER_TOTAL );
  s_inputGain = 0.f;
}

void DELFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) {
  case k_user_delfx_param_time:
    s_time = clip01f(valf);
    break;
  case k_user_delfx_param_depth:
    s_depth = clip01f(valf);
    break;
  case k_user_delfx_param_shift_depth:
    // Rescale to add notch around 0.5f
    s_mix = (valf <= 0.49f) ? 1.02040816326530612244f * valf : (valf >= 0.51f) ? 0.5f + 1.02f * (valf-0.51f) : 0.5f;
    break;
  default:
    break;
  }
}
