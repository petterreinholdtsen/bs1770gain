/*
 * bs1770gain_print_classic.c
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
#include <bs1770gain_priv.h>

#define DBFMT "%.2f"
#define WDBFMT L"%.2f"

#if defined (_WIN32) // [
#define W_WIN32
//#define C_WIN32
#endif // ]

///////////////////////////////////////////////////////////////////////////////
static const bs1770gain_print_vmt_t *get_vmt(void);

///////////////////////////////////////////////////////////////////////////////
void bs1770gain_print_classic(bs1770gain_print_t *p, FILE *f)
{
  p->vmt=get_vmt();
  p->f=f;
  p->a=NULL;
  p->t=NULL;
}

static int bs1770gain_print_width(int flags)
{
  int width=0;
  int len;

  ////////
  if (0!=(flags&AGGREGATE_MOMENTARY_MEAN)) {
    if (width<(len=strlen("integrated")))
      width=len;
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_MAXIMUM)) {
    if (width<(len=strlen("momentary maximum")))
      width=len;
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_RANGE)) {
    if (width<(len=strlen("momentary range")))
      width=len;
  }

  ////////
  if (0!=(flags&AGGREGATE_SHORTTERM_MEAN)) {
    if (width<(len=strlen("shortterm mean")))
      width=len;
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_MAXIMUM)) {
    if (width<(len=strlen("shortterm maximum")))
      width=len;
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_RANGE)) {
    if (width<(len=strlen("range")))
      width=len;
  }

  ////////
  if (0!=(flags&AGGREGATE_SAMPLEPEAK)) {
    if (width<(len=strlen("sample peak")))
      width=len;
  }

  if (0!=(flags&AGGREGATE_TRUEPEAK)) {
    if (width<(len=strlen("true peak")))
      width=len;
  }

  return width;
}

static void bs1770gain_print_label(const char *label, int width, FILE *f)
{
#if defined (W_WIN32) // [
  wchar_t *wlabel;

  if (NULL==(wlabel=pbu_s2w(label))) {
    DMESSAGE("converting UTF-8 to UTF-16");
    goto wlabel;
  }
#endif // ]

  width+=6;

#if defined (W_WIN32) // [
  for (width-=wcslen(wlabel);0<width;--width)
    fputwc(L' ',f);
#else // ] [
  for (width-=strlen(label);0<width;--width)
    fputc(' ',f);
#endif // ]

#if defined (W_WIN32) // [
  fwprintf(f,L"%s:  ",wlabel);
#else // ] [
  fprintf(f,"%s:  ",label);
#endif // ]

#if defined (W_WIN32) // [
  free(wlabel);
wlabel:
  return;
#endif // ]
}

///////////////////////////////////////////////////////////////////////////////
static void session_head(bs1770gain_print_t *p)
{
}

static FILE *session_file(bs1770gain_print_t *p)
{
  return stdout==p->f?p->f:NULL;
}

static void session_tail(bs1770gain_print_t *p)
{
}

////////
static void album_head(bs1770gain_print_t *p, bs1770gain_album_t *a,
    const char *ibasename)
{
#if defined (W_WIN32) // [
  wchar_t *wibasename;

  if (NULL==ibasename)
    wibasename=NULL;
  else if (NULL==(wibasename=pbu_s2w(ibasename))) {
    DMESSAGE("converting UTF-8 to UTF-16");
    goto wibasename;
  }
#endif // ]

  p->a=a;

#if defined (W_WIN32) // [
  if (NULL==wibasename||0==*wibasename)
    fwprintf(p->f,L"analyzing ...\n");
  else
    fwprintf(p->f,L"analyzing \"%s\" ...\n",wibasename);
#else // ] [
  if (NULL==ibasename||0==*ibasename)
    fprintf(p->f,"analyzing ...\n");
  else
    fprintf(p->f,"analyzing \"%s\" ...\n",ibasename);
#endif // ]

#if defined (W_WIN32) // [
  if (wibasename)
    free(wibasename);
wibasename:
  return;
#endif // ]
}

static void album_tail(bs1770gain_print_t *p)
{
#if defined (W_WIN32) // [
  fwprintf(p->f,L"done.\n");
#else // ] [
  fprintf(p->f,"done.\n");
#endif // ]
  p->a=NULL;
}

////////
static void track_head(bs1770gain_print_t *p, bs1770gain_track_t *t)
{
#if defined (W_WIN32) // [
  wchar_t *wbasename;

  if (NULL==t)
    wbasename=NULL;
  else if (NULL==(wbasename=pbu_s2w(pbu_basename(t->ipath)))) {
    DMESSAGE("converting UTF-8 to UTF-16");
    goto wbasename;
  }
#endif // ]

  p->t=t;

  if (NULL==t) {
#if defined (W_WIN32) // [
    fwprintf(p->f,L"  [ALBUM]:");
#else // ] [
    fprintf(p->f,"  [ALBUM]:");
#endif // ]
  }
  else {
#if defined (W_WIN32) // [
    fwprintf(p->f,L"  [%d/%d] \"%s\"",t->n,p->a->n,wbasename);
    fwprintf(p->f,L": ");
#else // ] [
    fprintf(p->f,"  [%d/%d] \"%s\"",t->n,p->a->n,pbu_basename(t->ipath));
    fprintf(p->f,": ");
#endif // ]
    fflush(p->f);
  }

#if defined (W_WIN32) // [
  free(wbasename);
wbasename:
  return;
#endif // ]
}

static void track_body(bs1770gain_print_t *p, aggregate_t *aggregate,
    const options_t *options)
{
  int flags=aggregate->flags;
  FILE *f=p->f;
  double norm=options->preamp+options->norm;
  int width=bs1770gain_print_width(flags);
  double q,db;
#if defined (C_WIN32) // [
  HANDLE hConsoleOutput;
  CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;

  if (p->t&&p->vmt->session.file(p)) {
    fflush(p->f);
    hConsoleOutput=GetStdHandle(_fileno(p->f));

    // get the cursor position.
    GetConsoleScreenBufferInfo(
      hConsoleOutput,
          // _In_  HANDLE                      hConsoleOutput,
      &consoleScreenBufferInfo
          // _Out_ PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo
    );

    // erase progress information, i.e. print blanks right to the cursor.
#if defined (W_WIN32) // [
    fwprintf(p->f,L"        ");
#else // ] [
    fprintf(p->f,"        ");
#endif // ]
    fflush(p->f);

    // advance line relative to the original cursor position.
    consoleScreenBufferInfo.dwCursorPosition.Y+=1;

    SetConsoleCursorPosition(
      hConsoleOutput,
          // _In_ HANDLE hConsoleOutput,
      consoleScreenBufferInfo.dwCursorPosition
          // _In_ COORD  dwCursorPosition
    );
  }
#elif defined (W_WIN32) // ] [
  fwprintf(p->f,p->t&&p->vmt->session.file(p)?L"        \n":L"\n");
#else // ] [
  fprintf(p->f,p->t&&p->vmt->session.file(p)?"        \n":"\n");
#endif // ]

  ////////
  if (0!=(flags&AGGREGATE_MOMENTARY_MEAN)) {
    db=lib1770_stats_get_mean(aggregate->momentary,
        options->momentary.mean_gate);
    //fprintf(f,"       integrated:  %.1f LUFS / %.1f LU\n",db,norm-db);
    bs1770gain_print_label("integrated",width,f);
#if defined (W_WIN32) // [
#if defined (WDBFMT) // [
    fwprintf(f,WDBFMT L" LUFS / " WDBFMT L" LU\n",db,norm-db);
#else // ] [
    fwprintf(f,L"%.1f LUFS / %.1f LU\n",db,norm-db);
#endif // ]
#else // ] [
#if defined (DBFMT) // [
    fprintf(f,DBFMT " LUFS / " DBFMT " LU\n",db,norm-db);
#else // ] [
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,norm-db);
#endif // ]
#endif // ]
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_MAXIMUM)) {
    db=lib1770_stats_get_max(aggregate->momentary);
    //fprintf(f,"        momentary:  %.1f LUFS / %.1f LU\n",db,norm-db);
    bs1770gain_print_label("momentary maximum",width,f);
#if defined (W_WIN32) // [
#if defined (WDBFMT) // [
    fwprintf(f,WDBFMT L" LUFS / " WDBFMT L" LU\n",db,norm-db);
#else // ] [
    fwprintf(f,L"%.1f LUFS / %.1f LU\n",db,norm-db);
#endif // ]
#else // ] [
#if defined (DBFMT) // [
    fprintf(f,DBFMT " LUFS / " DBFMT " LU\n",db,norm-db);
#else // ] [
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,norm-db);
#endif // ]
#endif // ]
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_RANGE)) {
    db=lib1770_stats_get_range(aggregate->momentary,
        options->momentary.range_gate,
        options->momentary.range_lower_bound,
        options->momentary.range_upper_bound);
    //fprintf(f,"            range:  %.1f LUFS\n",db);
    bs1770gain_print_label("momentary range",width,f);
#if defined (W_WIN32) // [
#if defined (WDBFMT) // [
    fwprintf(f,WDBFMT L" LUFS\n",db);
#else // ] [
    fwprintf(f,L"%.1f LUFS\n",db);
#endif // ]
#else // ] [
#if defined (DBFMT) // [
    fprintf(f,DBFMT " LUFS\n",db);
#else // ] [
    fprintf(f,"%.1f LUFS\n",db);
#endif // ]
#endif // ]
  }

  ////////
  if (0!=(flags&AGGREGATE_SHORTTERM_MEAN)) {
    db=lib1770_stats_get_mean(aggregate->shortterm,
        options->shortterm.mean_gate);
    //fprintf(f,"       integrated:  %.1f LUFS / %.1f LU\n",db,norm-db);
    bs1770gain_print_label("shortterm mean",width,f);
#if defined (W_WIN32) // [
#if defined (WDBFMT) // [
    fwprintf(f,WDBFMT L" LUFS / " WDBFMT L" LU\n",db,norm-db);
#else // ] [
    fwprintf(f,L"%.1f LUFS / %.1f LU\n",db,norm-db);
#endif // ]
#else // ] [
#if defined (DBFMT) // [
    fprintf(f,DBFMT " LUFS / " DBFMT " LU\n",db,norm-db);
#else // ] [
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,norm-db);
#endif // ]
#endif // ]
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_MAXIMUM)) {
    db=lib1770_stats_get_max(aggregate->shortterm);
    //fprintf(f,"       shortterm:  %.1f LUFS / %.1f LU\n",db,norm-db);
    bs1770gain_print_label("shortterm maximum",width,f);
#if defined (W_WIN32) // [
#if defined (WDBFMT) // [
    fwprintf(f,WDBFMT L" LUFS / " WDBFMT L" LU\n",db,norm-db);
#else // ] [
    fwprintf(f,L"%.1f LUFS / %.1f LU\n",db,norm-db);
#endif // ]
#else // ] [
#if defined (DBFMT) // [
    fprintf(f,DBFMT " LUFS / " DBFMT " LU\n",db,norm-db);
#else // ] [
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,norm-db);
#endif // ]
#endif // ]
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_RANGE)) {
    db=lib1770_stats_get_range(aggregate->shortterm,
        options->shortterm.range_gate,
        options->shortterm.range_lower_bound,
        options->shortterm.range_upper_bound);
    //fprintf(f,"            range:  %.1f LUFS\n",db);
    bs1770gain_print_label("range",width,f);
#if defined (W_WIN32) // [
#if defined (WDBFMT) // [
    fwprintf(f,WDBFMT L" LUFS\n",db);
#else // ] [
    fwprintf(f,L"%.1f LUFS\n",db);
#endif // ]
#else // ] [
#if defined (DBFMT) // [
    fprintf(f,DBFMT " LUFS\n",db);
#else // ] [
    fprintf(f,"%.1f LUFS\n",db);
#endif // ]
#endif // ]
  }

  ////////
  if (0!=(flags&AGGREGATE_SAMPLEPEAK)) {
    q=aggregate->samplepeak;
    db=LIB1770_Q2DB(q);
    //fprintf(f,"      sample peak:  %.1f SPFS / %f\n",db,q);
    bs1770gain_print_label("sample peak",width,f);
#if defined (W_WIN32) // [
#if defined (WDBFMT) // [
    fwprintf(f,WDBFMT L" SPFS / %f\n",db,q);
#else // ] [
    fwprintf(f,L"%.1f SPFS / %f\n",db,q);
#endif // ]
#else // ] [
#if defined (DBFMT) // [
    fprintf(f,DBFMT " SPFS / %f\n",db,q);
#else // ] [
    fprintf(f,"%.1f SPFS / %f\n",db,q);
#endif // ]
#endif // ]
  }

  if (0!=(flags&AGGREGATE_TRUEPEAK)) {
    q=aggregate->truepeak;
    db=LIB1770_Q2DB(q);
    //fprintf(f,"        true peak:  %.1f TPFS / %f\n",db,q);
    bs1770gain_print_label("true peak",width,f);
#if defined (W_WIN32) // [
#if defined (WDBFMT) // [
    fwprintf(f,WDBFMT L" TPFS / %f\n",db,q);
#else // ] [
    fwprintf(f,L"%.1f TPFS / %f\n",db,q);
#endif // ]
#else // ] [
#if defined (DBFMT) // [
    fprintf(f,DBFMT " TPFS / %f\n",db,q);
#else // ] [
    fprintf(f,"%.1f TPFS / %f\n",db,q);
#endif // ]
#endif // ]
  }
}

static void track_tail(bs1770gain_print_t *p)
{
  p->t=NULL;
}

////////
static const bs1770gain_print_vmt_t *get_vmt(void)
{
  static bs1770gain_print_vmt_t vmt;

  if (NULL==vmt.name) {
    vmt.name="bs1770gain_print_classic";
    vmt.session.head=session_head;
    vmt.session.file=session_file;
    vmt.session.tail=session_tail;
    vmt.album.head=album_head;
    vmt.album.tail=album_tail;
    vmt.track.head=track_head;
    vmt.track.body=track_body;
    vmt.track.tail=track_tail;
  }

  return &vmt;
}
