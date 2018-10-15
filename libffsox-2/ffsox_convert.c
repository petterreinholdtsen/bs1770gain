/*
 * ffsox_convert.c
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */
#include <ffsox_priv.h>

#if defined (FFSOX_FIX_881132_CHANNEL_OVERFLOW) // [
int ffsox_convert_setup(convert_t *convert, frame_t *fr, frame_t *fw,
    double q, intercept_t *intercept)
#else // ] [
void ffsox_convert_setup(convert_t *convert, frame_t *fr, frame_t *fw,
    double q, intercept_t *intercept)
#endif // ]
{
  int nb_samples1,nb_samples2;

  convert->fr=fr;
  convert->fw=fw;
  convert->q=q;
  convert->intercept=intercept;
  convert->channels=av_frame_get_channels(fr->frame);

#if defined (FFSOX_FIX_881132_CHANNEL_OVERFLOW) // [
  if (AV_NUM_DATA_POINTERS<=convert->channels) {
		DVMESSAGE("#channels (%d) not in range (%d at maximum)",
				convert->channels,AV_NUM_DATA_POINTERS);
		return -1;
	}
#endif // ]

  nb_samples1=fr->frame->nb_samples-fr->nb_samples.frame;
  nb_samples2=fw->frame->nb_samples-fw->nb_samples.frame;
  convert->nb_samples=PBU_MIN(nb_samples1,nb_samples2);

#if defined (FFSOX_FIX_881132_CHANNEL_OVERFLOW) // [
  return 0;
#endif // ]
}
