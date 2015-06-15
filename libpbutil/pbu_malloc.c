/*
 * pbu_malloc.c
 * Copyright (C) 2015 Peter Belkner <pbelkner@users.sf.net>
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

#if defined (PBU_MALLOC_DEBUG) // {
typedef struct head head_t;

struct head {
  const char *func;
  const char *file;
  int line;
  int free;
  size_t size;
  head_t *next;
};

enum { PBU_MALLOC_SIZE=4096*4096 };

static char buf[PBU_MALLOC_SIZE];
static char *rp=buf;
static char *mp=buf+PBU_MALLOC_SIZE;

static head_t *pbu_lookup(const void *ptr)
{
  head_t *head=(head_t *)buf;

  while (head<(head_t *)rp) {
    if (ptr==(char *)head+sizeof *head)
      return head;

    head=head->next;
  }

  return NULL;
}

void pbu_heap_print(void)
{
  head_t *head=(head_t *)buf;

  while (head<(head_t *)rp) {
    if (!head->free)
      fprintf(stderr,"%s(), %s (%d): %p\n",head->func,head->file,head->line,
          head+1);

    head=head->next;
  }
}

void *pbu_malloc(const char *func, const char *file, int line, size_t size)
{
  head_t *head=(head_t *)rp;
  char *ptr;

  if (mp<(ptr=rp+sizeof *head)) {
    pbu_trace_indent();
    fprintf(stderr,"malloc: %s(), %s (%d) ERROR 1\n",func,file,line);
    return NULL;
  }

  head->func=func;
  head->file=file;
  head->line=line;
  head->free=0;

  while (0!=size%8)
    ++size;

  if (mp<(ptr+size)) {
    pbu_trace_indent();
    fprintf(stderr,"malloc: %s(), %s (%d) ERROR 2\n",func,file,line);
    return NULL;
  }

  head->size=size;
  rp=ptr+size;
  head->next=(head_t *)rp;
  pbu_trace_indent();
  fprintf(stderr,"malloc: %s(), %s (%d): %p\n",func,file,line,ptr);

  return ptr;
}

void *pbu_calloc(const char *func, const char *file, int line, size_t num,
    size_t size)
{
  char *bp;

  pbu_trace_indent();
  fprintf(stderr,"calloc: %s(), %s (%d)\n",func,file,line);

  if (NULL==(bp=pbu_malloc(func,file,line,num*size)))
    return NULL;

  return calloc(num,size);
}

void *pbu_realloc(const char *func, const char *file, int line, void *ptr,
    size_t size)
{
  (void)ptr;
  (void)size;
  pbu_trace_indent();
  fprintf(stderr,"realloc: %s(), %s (%d) ERROR\n",func,file,line);
  exit(1);

  return NULL;
}

char *pbu_strdup(const char *func, const char *file, int line,
    const char *str)
{
  void *ptr;

  if (NULL==(ptr=pbu_malloc(func,file,line,strlen(str)+1)))
    return NULL;

  return strcpy(ptr,str);
}

#if defined (_WIN32) // {
wchar_t *_pbu_wcsdup(const char *func, const char *file, int line,
    const wchar_t *str)
{
  void *ptr;

  if (NULL==(ptr=pbu_malloc(func,file,line,(wcslen(str)+1)*sizeof *str)))
    return NULL;

  return wcscpy(ptr,str);
}
#endif // }

void pbu_free1(void *ptr)
{
  head_t *head=pbu_lookup(ptr);


  if (NULL==head) {
    pbu_trace_indent();
    fprintf(stderr,"free: ERROR 1: %p\n",ptr);
    exit(1);
  }
  else if (head->free) {
    pbu_trace_indent();
    fprintf(stderr,"free: ERROR 2: %p %s(), %s (%d)\n",
        ptr,head->func,head->file,head->line);
    exit(1);
  }

  pbu_trace_indent();
  fprintf(stderr,"free: %p\n",ptr);
  head->free=1;
}

void pbu_free(const char *func, const char *file, int line, void *ptr)
{
  head_t *head=pbu_lookup(ptr);


  if (NULL==head) {
    pbu_trace_indent();
    fprintf(stderr,"free: %s(), %s (%d) ERROR 1: %p\n",func,file,line,ptr);
    exit(1);
  }
  else if (head->free) {
    pbu_trace_indent();
    fprintf(stderr,"free: %s(), %s (%d) ERROR 2: %p %s(), %s (%d)\n",
        func,file,line,ptr,head->func,head->file,head->line);
    exit(1);
  }

  pbu_trace_indent();
  fprintf(stderr,"free: %s(), %s (%d): %p\n",func,file,line,ptr);
  head->free=1;
}
#endif // }
