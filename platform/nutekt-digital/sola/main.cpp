/*
 *   Copyright (C) 2024 David García Goñi <dagargo@gmail.com>
 *
 *   This file is part of logue-collection.
 *
 *   logue-collection is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   logue-collection is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with logue-collection. If not, see <http://www.gnu.org/licenses/>.
 */

#include "usermodfx.h"
#include "simplelfo.hpp"
#include "delayline.hpp"
#include "biquad.hpp"
#include <stdint.h>

#define FRAMES_MS 48
#define HALF_DELAY_MS 10
#define HALF_DELAY_FRAMES (HALF_DELAY_MS * FRAMES_MS)
#define DOUBLE_DELAY_FRAMES (HALF_DELAY_FRAMES * 4)

#define CONST_SR_RECIPROCAL (1.f / 48000.f)
#define CONST_LPF_CUTOFF 12000.0f
#define CONST_LPF_Q 0.707f
#define PHI0_120_INC ((0x7fffffff / 3) * 2)

static __sdram f32pair_t mem[DOUBLE_DELAY_FRAMES];
static float slfod, flfod;
static dsp::BiQuad lpfl;
static dsp::BiQuad lpfr;
static dsp::DualDelayLine dl;
static dsp::DelayLine dlr;
static dsp::SimpleLFO slfo;
static dsp::SimpleLFO flfo;

void
lfo_tri_phase_cycle (dsp::SimpleLFO * lfo, float *o0, float *o120, float *o240)
{
  float w0;

  lfo->cycle ();
  w0 = lfo->phi0;
  *o0 = lfo->sine_bi();
  lfo->phi0 += PHI0_120_INC;
  *o120 = lfo->sine_bi();
  lfo->phi0 += PHI0_120_INC;
  *o240 = lfo->sine_bi();
  lfo->phi0 = w0;
}

void
MODFX_INIT (uint32_t platform, uint32_t api)
{
  float wc = lpfl.mCoeffs.wc(CONST_LPF_CUTOFF, CONST_SR_RECIPROCAL);
  float tanpiwc = fx_tanpif(wc);
  lpfl.mCoeffs.setSOLP(tanpiwc, CONST_LPF_Q);
  wc = lpfr.mCoeffs.wc(CONST_LPF_CUTOFF, CONST_SR_RECIPROCAL);
  tanpiwc = fx_tanpif(wc);
  lpfr.mCoeffs.setSOLP(tanpiwc, CONST_LPF_Q);

  dl.setMemory (mem, DOUBLE_DELAY_FRAMES);

  slfo.reset ();
  flfo.reset ();
}

void
MODFX_PROCESS (const float *main_xn, float *main_yn,
	       const float *sub_xn, float *sub_yn, uint32_t frames)
{
  f32pair_t input, output, outputpos1, outputpos2, outputpos3;
  float inl, inr, outl, outr, slfo0, flfo0, slfo120, flfo120, slfo240, flfo240;
  float play_offset, play_pos1, play_pos2, play_pos3;
  for (; frames > 0; frames--)
    {
      input.a = *main_xn;
      main_xn++;
      input.b = *main_xn;
      main_xn++;

      input.a = lpfl.process_fo(input.a);
      input.b = lpfr.process_fo(input.b);
      dl.write(input);

      lfo_tri_phase_cycle (&slfo, &slfo0, &slfo120, &slfo240);
      lfo_tri_phase_cycle (&flfo, &flfo0, &flfo120, &flfo240);

      play_offset = HALF_DELAY_FRAMES + HALF_DELAY_FRAMES;

      play_pos1 = play_offset + slfo0 * slfod + flfo0 * flfod;
      play_pos2 = play_offset + slfo120 * slfod + flfo120 * flfod;
      play_pos3 = play_offset + slfo240 * slfod + flfo240 * flfod;

      outputpos1 = dl.readFrac(play_pos1);
      outputpos2 = dl.readFrac(play_pos2);
      outputpos3 = dl.readFrac(play_pos3);

      output = f32pair_add (outputpos1, outputpos2);
      output = f32pair_add (output, outputpos3);

      output = f32pair_mulscal (output, 0.333f);

      *main_yn = output.a;
      main_yn++;
      *main_yn = output.b;
      main_yn++;
    }
}

void
MODFX_PARAM (uint8_t index, int32_t value)
{
  float v = q31_to_f32 (value);
  switch (index)
    {
    case k_user_modfx_param_time:
      slfo.setF0 (0.13f + 3.89f * v, CONST_SR_RECIPROCAL);
      flfo.setF0 (1.18f + 13.27f * v, CONST_SR_RECIPROCAL);
      break;
    case k_user_modfx_param_depth:
      slfod = 0.104f * v * HALF_DELAY_FRAMES;
      flfod = 0.052f * v * HALF_DELAY_FRAMES;
      break;
    }
}
