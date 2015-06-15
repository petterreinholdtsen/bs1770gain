/*
 * ffsox_basename.c
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

static node_vmt_t vmt;

int ffsox_node_create(node_t *n)
{
  n->vmt=ffsox_node_get_vmt();
  n->state=STATE_RUN;

  return 0;
}

void ffsox_node_destroy(node_t *n)
{
  n->vmt->cleanup(n);
  FREE(n);
}

////////
static void node_cleanup(node_t *n)
{
  (void)n;
}

static node_t *node_prev(node_t *n)
{
  (void)n;

  return NULL;
}

static node_t *node_next(node_t *n)
{
  (void)n;

  return NULL;
}

static int node_run(node_t *n)
{
  DMESSAGE("running node");
  (void)n;

  return -1;
}

const node_vmt_t *ffsox_node_get_vmt(void)
{
  static int initialized;

  if (0==initialized) {
    vmt.name="node";
    vmt.cleanup=node_cleanup;
    vmt.prev=node_prev;
    vmt.next=node_next;
    vmt.run=node_run;
    initialized=1;
  }

  return &vmt;
}
