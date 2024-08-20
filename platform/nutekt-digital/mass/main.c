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

#include "userosc.h"
#include "osc_api.h"
#include "buffer_ops.h"

#define MAX_VOICES 7

float phase_voice[MAX_VOICES];
float phase_sub;
float sub;
float detuning;
float beating;
uint8_t voices;
float voices_inv;

void
OSC_INIT (uint32_t platform, uint32_t api)
{
  for (uint32_t i = 0; i < MAX_VOICES; i++)
    {
      phase_voice[i] = 0.0f;
    }
}

static inline float
get_sawf (float phase, uint8_t note)
{
  return osc_bl2_sawf (phase, _osc_bl_saw_idx (note));
}

void
OSC_CYCLE (const user_osc_param_t * const params,
	   int32_t * yn, const uint32_t frames)
{
  uint8_t note = (params->pitch) >> 8;
  uint8_t pitch_mod = params->pitch & 0xFF;
  float w0 = osc_w0f_for_note (note, pitch_mod);
  float inc_detuning = detuning * voices_inv;
  float inc_beating = beating * voices_inv;

  for (uint32_t i = 0; i < frames; i++, yn++)
    {
      float mix = 0.0f, detuning;
      int s = 1, k;
      for (uint32_t i = 0; i < voices; i++)
	{
	  mix += get_sawf (phase_voice[i], note) * voices_inv;
	  k = s * i;
	  s = -1 * s;
	  detuning = 1.0f + inc_detuning * k;
	  phase_voice[i] += w0 * detuning + inc_beating * k;
	  phase_voice[i] -= (uint32_t) phase_voice[i];
	}
      mix = (1.0f - sub) * mix + sub * get_sawf (phase_sub, note - 12);
      *yn = f32_to_q31 (mix);

      phase_sub += w0 * 0.5f;
      phase_sub -= (uint32_t) phase_sub;
    }
}

void
OSC_NOTEOFF (const user_osc_param_t * const params)
{
  (void) params;
}

void
OSC_PARAM (uint16_t param, uint16_t value)
{
  switch (param)
    {
    case k_user_osc_param_shape:
      detuning = param_val_to_f32 (value) * 0.05f;
      break;
    case k_user_osc_param_shiftshape:
      sub = param_val_to_f32 (value) * 0.5;
      break;
    case k_user_osc_param_id1:
      voices = value + 1;
      voices_inv = 1.0f / voices;
      break;
    case k_user_osc_param_id2:
      beating = value * 0.00001f;
      break;
    case k_user_osc_param_id3:
    case k_user_osc_param_id4:
    case k_user_osc_param_id5:
    case k_user_osc_param_id6:
      break;
    default:
      break;
    }
}
