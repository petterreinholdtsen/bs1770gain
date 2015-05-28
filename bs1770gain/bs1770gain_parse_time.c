/*
 * bs1770gain_parse_time.c
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
#include <bs1770gain.h>
#include <ctype.h>

// parse time in microseconds.
int64_t bs1770gain_parse_time(const char *s)
{
  enum { N=1000000 };
  int64_t t1=0,t2=0,tt=0;
  int64_t n=N;
  const char *mp=s+strlen(s);
state1:
  if (mp==s) {
    t1*=60;
    t1+=tt;
    tt=0;
    goto final;
  }
  else if ('.'==*s) {
    t1*=60;
    t1+=tt;
    tt=0;
    ++s;
    goto state2;
  }
  else if (':'==*s) {
    t1*=60;
    t1+=tt;
    tt=0;
    ++s;
    goto state1;
  }
  else if (isdigit(*s)) {
    tt*=10;
    tt+=*s++-'0';
    goto state1;
  }
  else
    goto error;
state2:
  if (mp==s)
    goto final;
  else if (isdigit(*s)) {
    if (0<n) {
      n/=10;
      t2+=n*(*s++-'0');
    }

    goto state2;
  }
  else
    goto error;
final:
  return t1*N+t2;
error:
  return 0;
}

