/*
 * mux3.c
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
#include <ffsox_priv.h>
#if defined (_WIN32) // [
#include <fcntl.h>
#endif // ]

#if defined (_WIN32) // [
//#define W_WIN32
//#define C_WIN32
#endif // ]

void ffsox_source_progress(const source_t *si, void *data)
{
  FILE *f=data;
  const AVPacket *pkt;
  const AVStream *st;
  int64_t duration;
  double percent;
#if defined (W_WIN32) // [
  wchar_t wbuf[32];
#else // ] [
  char buf[32];
#endif // ]
  int i;
#if defined (C_WIN32) // [
  HANDLE hConsoleOutput;
  CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
#endif // ]

  pkt=&si->pkt;
  st=si->f.fc->streams[pkt->stream_index];
  duration=av_rescale_q(si->f.fc->duration,AV_TIME_BASE_Q,st->time_base);
  percent=0ll<pkt->dts&&0ll<duration?100.0*pkt->dts/duration:0.0;
#if defined (W_WIN32) // [
  swprintf(wbuf,(sizeof wbuf)/(sizeof wbuf[0])-1,L"%.0f%%",percent);
#else // ] [
  sprintf(buf,"%.0f%%",percent);
#endif // ]

#if defined (C_WIN32) // [
  hConsoleOutput=GetStdHandle(_fileno(f));

  GetConsoleScreenBufferInfo(
    hConsoleOutput,
        // _In_  HANDLE                      hConsoleOutput,
    &consoleScreenBufferInfo
        // _Out_ PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo
  );
#endif // ]

#if defined (W_WIN32) // [
  i=wcslen(wbuf);
  fputws(wbuf,f);
#else // ] [
  i=strlen(buf);
  fputs(buf,f);
#endif // ]

#if defined (C_WIN32) // [
  // http://stackoverflow.com/questions/2732292/setting-the-cursor-position-in-a-win32-console-application
  consoleScreenBufferInfo.dwCursorPosition.X-=i;

  SetConsoleCursorPosition(
    hConsoleOutput,
        // _In_ HANDLE hConsoleOutput,
    consoleScreenBufferInfo.dwCursorPosition
        // _In_ COORD  dwCursorPosition
  );
#elif defined (W_WIN32) // ] [
  while (0<i) {
    fputwc(L'\b',f);
    --i;
  }
#else // ] [
  while (0<i) {
    fputc('\b',f);
    --i;
  }
#endif // ]
}
