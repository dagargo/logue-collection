/*
 *   Copyright (C) 2019 David García Goñi <dagargo@gmail.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "userosc.h"
#include "osc_api.h"

#define MAX_RATIO 16.0f
#define MIN_RATIO 0.25f

#define FM_FACTOR 10.0f

#define GET_M_VALUE(f) (f * 0.01f)
#define GET_F_VALUE(f) (f * (MAX_RATIO - MIN_RATIO) + MIN_RATIO)
#define GET_F_VALUE_OCTAVE(v,d) (v == -2 ? 0.25f : v == -1 ? 0.5f : v == 0 ? 1.0f : v == 1 ? 2.0f : v == 2 ? 4.0f : d)

/*
_____
/     \    Feedback
|     |
\-- [ O1 ] Shape             [ O3 ] O3o
     |                        |
     *    Shift + LFO         *    O3 Mod. Depth + LFO
     |                        |
   [ O2 ] O2 Octave         [ O4 ] O4 Octave
     |                        |
     \------------------------/    Balance
*/

float phase_o1;
float phase_o2;
float phase_o3;
float phase_o4;
float o1;
float o1f;
float o1md;
float o2f;
float o3f;
float o3md;
float o4f;
float balance;
float o1fb;

void
OSC_INIT (uint32_t platform, uint32_t api)
{
  o1f = 1.0f;
  o1md = 0.0f;
  o2f = 1.0f;
  o3f = 1.0f;
  o3md = 0.0f;
  o4f = 1.0f;
  balance = 0.0;
  o1fb = 0.0;
}

void
OSC_CYCLE (const user_osc_param_t * const params,
	   int32_t * yn, const uint32_t frames)
{
  uint8_t note = params->pitch >>8;
  uint8_t pitch_mod = params->pitch & 0xff;
  float shape_mod = q31_to_f32(params->shape_lfo);
  float w0 = osc_w0f_for_note(note, pitch_mod);
  float w0o1 = w0 * o1f;
  float w0o2 = w0 * o2f;
  float w0o3 = w0 * o3f;
  float w0o4 = w0 * o4f;
  float o1md_plus_shape_mod = o1md + shape_mod;
  float o3md_plus_shape_mod = o3md + shape_mod;

  for (uint32_t i = 0; i < frames; i++, yn++) {
    o1 = osc_sinf(phase_o1 + o1 * o1fb);
    float o2 = osc_sinf(phase_o2 + o1md_plus_shape_mod * o1 * FM_FACTOR);
    float o3 = osc_sinf(phase_o3);
    float o4 = osc_sinf(phase_o4 + o3md_plus_shape_mod * o3 * FM_FACTOR);
    float mix = (1.0f - balance) * o2 + balance * o4;
    *yn = f32_to_q31(mix);
    phase_o1 += w0o1;
    phase_o2 += w0o2;
    phase_o3 += w0o3;
    phase_o4 += w0o4;
    phase_o1 -= (uint32_t) phase_o1;
    phase_o2 -= (uint32_t) phase_o2;
    phase_o3 -= (uint32_t) phase_o3;
    phase_o4 -= (uint32_t) phase_o4;
  }
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
  o1 = 0.0f;
  phase_o1 = 0.0f;
  phase_o2 = 0.0f;
  phase_o3 = 0.0f;
  phase_o4 = 0.0f;
}

void
OSC_NOTEOFF (const user_osc_param_t * const params)
{
}

void
OSC_PARAM (uint16_t param, uint16_t value)
{
  const float valf = param_val_to_f32(value);
  switch (param)
    {
    case k_user_osc_param_shape:
      o1f = GET_F_VALUE(valf);
      break;
    case k_user_osc_param_shiftshape:
      o1md = valf;
      break;
    case k_user_osc_param_id1:
      o1fb = 0.17f * GET_M_VALUE(value);
      break;
    case k_user_osc_param_id2:
      o2f = GET_F_VALUE_OCTAVE((int16_t) value, o2f);
      break;
    case k_user_osc_param_id3:
      o3f = GET_F_VALUE_OCTAVE((int16_t) value, o3f);
      break;
    case k_user_osc_param_id4:
      o3md = GET_M_VALUE(value);
      break;
    case k_user_osc_param_id5:
      o4f = GET_F_VALUE_OCTAVE((int16_t) value, o4f);
      break;
    case k_user_osc_param_id6:
      balance = GET_M_VALUE(value);
      break;
    default:
      break;
    }
}
