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

#include "userdelfx.h"
#include "simplelfo.hpp"
#include "biquad.hpp"
#include <stdint.h>

#define BASE_FREQ 500.0f
#define DEPTH_FREQ 7500.0f

#define CONST_SR_RECIPROCAL (1.f / 48000.f)

#define PHI0_180_INC 0x7fffffff

static float l, r;
static float fb;
static uint32_t stereo_phase; 
static dsp::SimpleLFO lfo;
static dsp::BiQuad pl1;
static dsp::BiQuad pl2;
static dsp::BiQuad pl3;
static dsp::BiQuad pl4;
static dsp::BiQuad pr1;
static dsp::BiQuad pr2;
static dsp::BiQuad pr3;
static dsp::BiQuad pr4;

void
lfo_dual_phase_cycle (float *o0, float *o90)
{
  float w0;

  lfo.cycle ();
  w0 = lfo.phi0;
  *o0 = lfo.triangle_uni ();
  lfo.phi0 += stereo_phase;
  *o90 = lfo.triangle_uni ();
  lfo.phi0 = w0;
}

void
DELFX_INIT (uint32_t platform, uint32_t api)
{
  l = 0.0f;
  r = 0.0f;
  stereo_phase = 0.5 * PHI0_180_INC;
  lfo.setF0 (0.1f, CONST_SR_RECIPROCAL);
  lfo.reset ();
}

void
DELFX_PROCESS (float *xn, uint32_t frames)
{
  float fl, fr, lfo0, lfo90, tanpiwcl, tanpiwcr;
  for (; frames > 0; frames--)
    {
      lfo_dual_phase_cycle (&lfo0, &lfo90);
      fl = BASE_FREQ + DEPTH_FREQ * lfo0 * lfo0;
      fr = BASE_FREQ + DEPTH_FREQ * lfo90 * lfo90;
      tanpiwcl = fx_tanpif (pl1.mCoeffs.wc (fl, CONST_SR_RECIPROCAL));
      tanpiwcr = fx_tanpif (pr1.mCoeffs.wc (fr, CONST_SR_RECIPROCAL));

      pl1.mCoeffs.setFOAP (tanpiwcl);
      pl2.mCoeffs.setFOAP (tanpiwcl);
      pl3.mCoeffs.setFOAP (tanpiwcl);
      pl4.mCoeffs.setFOAP (tanpiwcl);

      pr1.mCoeffs.setFOAP (tanpiwcr);
      pr2.mCoeffs.setFOAP (tanpiwcr);
      pr3.mCoeffs.setFOAP (tanpiwcr);
      pr4.mCoeffs.setFOAP (tanpiwcr);

      l = pl1.process_fo (*xn + l * fb);
      l = pl2.process_fo (l);
      l = pl3.process_fo (l);
      l = pl4.process_fo (l);

      r = pr1.process_fo (*(xn + 1) + r * fb);
      r = pr2.process_fo (r);
      r = pr3.process_fo (r);
      r = pr4.process_fo (r);

      *xn = l + *xn;
      xn++;
      *xn = r + *xn;
      xn++;
    }
}

void
DELFX_PARAM (uint8_t index, int32_t value)
{
  float v = q31_to_f32 (value);
  switch (index)
    {
    case k_user_delfx_param_time:
      lfo.setF0 (0.05 + 9.95 * v, CONST_SR_RECIPROCAL);
      break;
    case k_user_delfx_param_depth:
      fb = 0.9f * v;
      break;
    case k_user_delfx_param_shift_depth:
      stereo_phase = v * PHI0_180_INC;
      break;      
    }
}
