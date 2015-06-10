/*
 * pbutil_priv.h
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
#ifndef __PBUTIL_PRIV_H__
#define __PBUTIL_PRIV_H__ // {
#include <pbutil.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
#define MESSAGE(m)          PBU_MESSAGE(m)

///////////////////////////////////////////////////////////////////////////////
#define MAXOF(T)            PBU_MAXOF(T)

///////////////////////////////////////////////////////////////////////////////
#define LIST_APPEND(l,n)    PBU_LIST_APPEND(l,n)
#define LIST_NEXT(n,l)      PBU_LIST_NEXT(n,l)
#define LIST_FOREACH(n,l)   PBU_LIST_FOREACH(n,l)

///////////////////////////////////////////////////////////////////////////////
typedef pbu_list_t list_t;

#ifdef __cpluplus
}
#endif
#endif // }
