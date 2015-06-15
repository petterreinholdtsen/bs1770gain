/*
 * pbu_trace.c
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
enum { PBU_STACK_SIZE=1024 };

static const char *stack[PBU_STACK_SIZE];
static const char **wp=stack;
static const char **mp=stack+PBU_STACK_SIZE;
static int count=0;

void pbu_trace_indent(void)
{
  int n=wp-stack;

  while (0<n) {
    --n;
    fputs("  ",stderr);
  }
}

void pbu_trace_puts(const char *str)
{
  pbu_trace_indent();
  fputs(str,stderr);
  fputc('\n',stderr);
}

void pbu_trace_push(const char *func)
{
  if (wp==mp)
    ++count;
  else {
    pbu_trace_indent();
    fprintf(stderr,"{ %s\n",func);
    *wp++=func;
  }
}

void pbu_trace_pop(void)
{
  if (0<count)
    --count;
  else {
    --wp;
    pbu_trace_indent();
    fprintf(stderr,"} %s\n",wp[0]);
  }
}
#endif // }
