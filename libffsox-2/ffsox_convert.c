/*
 * ffsox_convert.c
 * Copyright (C) 2014 Peter Belkner <pbelkner@snafu.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */
#include <ffsox_priv.h>

void ffsox_convert_setup(convert_t *convert, frame_t *fr, frame_t *fw,
    double q, intercept_t *intercept)
{
  int nb_samples1,nb_samples2;

  convert->fr=fr;
  convert->fw=fw;
  convert->q=q;
  convert->intercept=intercept;
  convert->channels=av_frame_get_channels(fr->frame);

  nb_samples1=fr->frame->nb_samples-fr->nb_samples.frame;
  nb_samples2=fw->frame->nb_samples-fw->nb_samples.frame;
  convert->nb_samples=PBU_MIN(nb_samples1,nb_samples2);
}
