/*
 * ffsox_list.c
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

///////////////////////////////////////////////////////////////////////////////
void *ffsox_list_create(void *node)
{
  list_t *n=node;

  n->prev=node;
  n->next=node;

  return node;
}

int ffsox_list_append(void *head, void *node, size_t size)
{
  list_t **hp=head;
  list_t *h=*hp;
  list_t *n;

  if (NULL==(n=malloc(size)))
    goto malloc;

  memcpy(n,node,size);

  if (NULL==h) {
    n->prev=n;
    n->next=n;
    h=n;
  }
  else {
    n->next=h;
    n->prev=h->prev;
    h->prev->next=n;
    h->prev=n;
  }

  *hp=h;

  return 0;
malloc:
  return -1;
}

void *ffsox_list_remove_link(void *head, void *node)
{
  list_t *h=head;
  list_t *n=node;

  if (n!=h)
    ;
  else if (n->next!=h)
    h=n->next;
  else
    h=NULL;

  n->prev->next=n->next;
  n->next->prev=n->prev;
  n->prev=NULL;
  n->next=NULL;

  return h;
}

void ffsox_list_free_full(void *head, void *free_func)
{
  list_t *h=head;
  list_t *n;

  while (NULL!=(n=h)) {
    h=ffsox_list_remove_link(n,n);
    ((void (*)(list_t *))free_func)(n);
  }
}

void ffsox_list_free(void *head)
{
  ffsox_list_free_full(head,free);
}
