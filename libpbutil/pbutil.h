/*
 * pbu.h
 * Copyright (C) 2015 Peter Belkner <pbelkner@snafu.de>
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
#ifndef __PBUTIL_H__
#define __PBUTIL_H__ // {
#if defined (WIN32) // {
#include <windows.h>
#endif // }
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
#define PBU_MESSAGE(m) fprintf(stderr,"Error " m ": \"%s\" (%d).\n", \
    pbu_basename(__FILE__),__LINE__)
#define PBU_MIN(x,y) ((x)<(y)?(x):(y))

#define PBU_MAXOF(T) \
  (~(~0ll<<(((sizeof (T))<<3)-1)))

#if defined (WIN32) // {
#define wcstok_r(str,delim,saveptr) pbu_wcstok_r(str,delim,saveptr)
#define strtok_r(str,delim,saveptr) pbu_strtok_r(str,delim,saveptr)
#endif // }

///////////////////////////////////////////////////////////////////////////////
typedef struct pbu_list pbu_list_t;

///////////////////////////////////////////////////////////////////////////////
#if defined (WIN32) // {
HANDLE pbu_msvcrt(void);
wchar_t *pbu_wcstok_r(wchar_t *str, const wchar_t *delim, wchar_t **saveptr);
char *pbu_strtok_r(char *str, const char *delim, char **saveptr);
int pbu_copy_file(const wchar_t *src, const wchar_t *dst);
wchar_t *pbu_s2w(const char *s);
#else // } {
int pbu_copy_file(const char *src, const char *dst);
#endif // }
int pbu_same_file(const char *path1, const char *path2);
char *pbu_extend_path(const char *dirname, const char *basename);
const char *pbu_ext(const char *path);
const char *pbu_basename(const char *path);
void pbu_mkdir(char *path);

/// list //////////////////////////////////////////////////////////////////////
#define PBU_LIST_APPEND(l,n) \
  pbu_list_append(&(l),&(n),sizeof (n))
#define PBU_LIST_NEXT(n,l) \
  (*(n)=(*(n)==NULL||(l)==(*(n))->next?NULL:(*(n))->next))
#define PBU_LIST_FOREACH(n,l) \
  for (*(n)=(l);NULL!=*(n);PBU_LIST_NEXT(n,l))

struct pbu_list {
#define PBU_LIST_MEM(T) \
  T *prev; \
  T *next;
  PBU_LIST_MEM(pbu_list_t)
};

void *pbu_list_create(void *node);
int pbu_list_append(void *head, void *node, size_t size);
void *pbu_list_remove_link(void *head, void *node);
void pbu_list_free_full(void *head, void *free_func);
void pbu_list_free(void *head);
#ifdef __cpluplus
}
#endif
#endif // }
