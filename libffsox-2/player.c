/*
 * player.c
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
#include <initguid.h>
#include <ffsox_priv.h>
#include <getopt.h>

static audio_player_t *ap;

static BOOL WINAPI handler(DWORD signal)
{
  switch (signal) {
  case CTRL_C_EVENT:
    if (NULL!=ap)
      ffsox_audio_player_kill(ap);

    return FALSE;
  default:
    return FALSE;
  }
}

int play(IMMDevice *pDevice, const char *path)
{
#if 1 // {
  enum { SHAREMODE=AUDCLNT_SHAREMODE_EXCLUSIVE };
#else // } {
  enum { SHAREMODE=AUDCLNT_SHAREMODE_SHARED };
#endif // }
  int code=-1;
  source_t si;
  AVDictionaryEntry *track,*album;
  double q;
  frame_reader_t *fr;

  // open the input file.
  if (ffsox_source_create(&si,path,-1,-1,ffsox_source_progress,stdout)<0) {
fprintf(stderr,"\"%s\"\n",path);
    MESSAGE("creating source");
    goto si;
  }

  av_dump_format(si.f.fc,0,si.f.path,0);

  track=av_dict_get(si.f.fc->metadata,"REPLAYGAIN_TRACK_GAIN",NULL,0);
  album=av_dict_get(si.f.fc->metadata,"REPLAYGAIN_ALBUM_GAIN",NULL,0);

  if (NULL==track&&NULL==album) {
    fprintf(stderr,"GAIN: 0.0\n");
    q=1.0;
  }
  else if (NULL==album) {
    q=atof(track->value);
    fprintf(stderr,"GAIN: %.1f\n",q);
    q=LIB1770_DB2Q(q);
  }
  else if (NULL==track) {
    q=atof(album->value);
    fprintf(stderr,"GAIN: %.1f\n",q);
    q=LIB1770_DB2Q(q);
  }
  else {
    q=atof(track->value);
    q+=atof(album->value);
    q*=0.5;
    fprintf(stderr,"GAIN: %.1f\n",q);
    q=LIB1770_DB2Q(q);
  }

  // create a frame reader.
  if (NULL==(fr=ffsox_frame_reader_new(&si,si.ai,0.0))) {
    MESSAGE("creating frame reader");
    goto fr;
  }

  // create an audio player.
  if (NULL==(ap=ffsox_audio_player_new(fr,q,pDevice,SHAREMODE))) {
    MESSAGE("creating audio player");
    goto ap;
  }

  fr->next=&ap->frame_consumer;
  ap->prev=&fr->node;

  // play.
  if (ffsox_audio_player_play(ap)<0) {
    MESSAGE("playing");
    goto play;
  }

  code=0;
// cleanup:
play:
  ap=NULL;
ap:
fr:
  si.vmt->cleanup(&si);
si:
  return code;
}

int main(int argc, char **argv)
{
  int loop=0;
  HRESULT hr;
  IMMDeviceEnumerator *pEnumerator;
  IMMDevice *pDevice;
  int c,i;

  opterr=0;

  while (-1!=(c=getopt_long(argc,argv,"l",NULL,NULL))) {
    switch (c) {
    case '?':
      MESSAGE("usage");
      goto usage;
    case 'l':
      loop=1;
      break;
    default:
      MESSAGE("usage");
      goto usage;
    }
  }

  if (!SetConsoleCtrlHandler(handler,TRUE)) {
    MESSAGE("setting console handler");
    goto handler;
  }

  // check the number of arguments.
  if (argc<2) {
    MESSAGE("usage");
    goto usage;
  }

  // initialize COM.
  hr=CoInitializeEx(
    NULL,                       // _In_opt_  LPVOID pvReserved,
    COINIT_MULTITHREADED        // _In_      DWORD dwCoInit
  );

  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(RPC_E_CHANGED_MODE);
    }

    MESSAGE("initializing COM");
    goto coinit;
  }

  // dynamically load the FFmpeg and SoX libraries.
  if (ffsox_dynload(NULL)<0) {
    MESSAGE("loading shared libraries");
    goto load;
  }

  // initialize SoX.
  if (SOX_SUCCESS!=sox_init()) {
    MESSAGE("initializing SoX");
    goto sox;
  }

  // initialize FFmpeg.
  av_register_all();

  // create a device enumerator.
  hr=CoCreateInstance(
    &CLSID_MMDeviceEnumerator,  // _In_   REFCLSID rclsid,
    NULL,                       // _In_   LPUNKNOWN pUnkOuter,
    CLSCTX_ALL,                 // _In_   DWORD dwClsContext,
    &IID_IMMDeviceEnumerator,   // _In_   REFIID riid,
    (void**)&pEnumerator        // _Out_  LPVOID *ppv
  );

  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(REGDB_E_CLASSNOTREG);
      HR_ERROR_CASE(CLASS_E_NOAGGREGATION);
      HR_ERROR_CASE(E_NOINTERFACE);
      HR_ERROR_CASE(E_POINTER);
    }

    MESSAGE("creating device enumerator");
    goto devnum;
  }

  // get the default audio device.
  hr=pEnumerator->lpVtbl->GetDefaultAudioEndpoint(pEnumerator,
    eRender,          // [in]   EDataFlow dataFlow,
    eMultimedia,      // [in]   ERole role,
    &pDevice          // [out]  IMMDevice **ppDevice
  );

  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(E_POINTER);
      HR_ERROR_CASE(E_INVALIDARG);
      HR_ERROR_CASE(E_NOTFOUND);
      HR_ERROR_CASE(E_OUTOFMEMORY);
    }

    MESSAGE("getting default audio device");
    goto device;
  }

  do {
    for (i=optind;i<argc;++i)
      play(pDevice,argv[i]);
  } while (0!=loop);
// cleanup:
  pDevice->lpVtbl->Release(pDevice);
device:
  pEnumerator->lpVtbl->Release(pEnumerator);
devnum:
  sox_quit();
sox:
load:
coinit:
  CoUninitialize();
usage:
handler:
  return 0;
}
