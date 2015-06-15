/*
 * ffsox_packet_consumer_list.c
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

void ffsox_packet_consumer_list_free(packet_consumer_list_t *n)
{
  ffsox_node_t *tmp1=&n->consumer->node;
  ffsox_node_t *tmp2;

  while (NULL!=tmp1) {
    tmp2=tmp1;
    tmp1=tmp2->vmt->next(tmp2);
    ffsox_node_destroy(tmp2);
  }

  FREE(n);
}
