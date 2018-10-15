/*
 * ffsox_engine.c
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

int ffsox_engine_run(engine_t *e, node_t *node)
{
  int op;

  e->node=node;

  while (NULL!=(node=e->node)) {
//fprintf(stderr,"%s: RUN\n",node->vmt->name);
//DVWRITELN("vmt %p",node->vmt);
//DVWRITELN("vmt->run %p",node->vmt->run);
//DVWRITELN("vmt->name \"%s\"",node->vmt->name);
    op=node->vmt->run(node);
//DVWRITELN("op: %d",op);

    if (op<0) {
      DMESSAGE("running engine");
      return op;
    }

    switch (op) {
    case MACHINE_PUSH:
//fprintf(stderr,"%s: PUSH\n",node->vmt->name);
//DVWRITELN("%s PUSH",node->vmt->name);
      e->node=node->vmt->next(node);
//DMARKLN();
      break;
    case MACHINE_POP:
//fprintf(stderr,"%s: POP\n",node->vmt->name);
//DVWRITELN("%s POP",node->vmt->name);
      e->node=node->vmt->prev(node);
//DMARKLN();
      break;
    default:
      DMESSAGE("illegal instruction");
      return -1;
    }
  }

  return 0;
}
