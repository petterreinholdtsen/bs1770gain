/*
 * ffsox_machine.c
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

int ffsox_machine_create(machine_t *m, source_t *s)
{
  m->source=s;
  m->node=&s->node;

  return 0;
}

void ffsox_machine_cleanup(machine_t *m)
{
}

int ffsox_machine_loop(machine_t *m)
{
  m->node=&m->source->node;

  while (NULL!=m->node) {
    switch (m->node->vmt->run(m->node)) {
    case MACHINE_STAY:
      break;
    case MACHINE_PUSH:
      m->node=m->node->vmt->next(m->node);
      break;
    case MACHINE_POP:
      m->node=m->node->vmt->prev(m->node);
      break;
    default:
      MESSAGE("running node");
      return -1;
    }
  }

  return 0;
}
