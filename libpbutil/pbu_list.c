/*
 * pbu_list.c
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
#include <pbutil_priv.h>

///////////////////////////////////////////////////////////////////////////////
void *pbu_list_create(void *node)
{
  list_t *n=node;

#if defined (PBU_LIST_RING) // [
  n->prev=node;
  n->next=node;
#else // ] [
  n->prev=NULL;
  n->next=NULL;
#endif // ]

  return node;
}

int pbu_list_append(void *head, void *node, size_t size)
{
  list_t **hp=head;
  list_t *h=*hp;
  list_t *n;

  if (NULL==(n=MALLOC(size)))
    goto malloc;

  memcpy(n,node,size);

  if (NULL==h) {
#if defined (PBU_LIST_RING) // [
    n->prev=n;
    n->next=n;
#else // ] [
    n->prev=NULL;
    n->next=NULL;
#endif // ]
    h=n;
  }
  else {
    n->next=h;
#if defined (PBU_LIST_RING) // [
    n->prev=h->prev;
    h->prev->next=n;
#else // ] [
    n->prev=NULL;
#endif // ]
    h->prev=n;
  }

  *hp=h;

  return 0;
malloc:
  return -1;
}

#if defined (PBU_LIST_RING) // [
void *pbu_list_remove_link(void *head, void *node)
{
  list_t *h=head;
  list_t *n=node;

  if (n!=h)
    ;
  else if (n->next!=h)
    h=n->next;
  else
    h=NULL;

  if (n->prev)
    n->prev->next=n->next;

  if (n->next)
    n->next->prev=n->prev;

  n->prev=NULL;
  n->next=NULL;

  return h;
}
#endif // ]

void pbu_list_free_full(void *head, void *free_func)
{
  list_t *h=head;
  list_t *n;

  while (NULL!=(n=h)) {
#if defined (PBU_LIST_RING) // [
    h=pbu_list_remove_link(n,n);
#else // ] [
    h=n->next;
#endif // ]
    ((void (*)(list_t *))free_func)(n);
  }
}

void pbu_list_free(void *head)
{
  pbu_list_free_full(head,PBU_FREE_PTR);
}
