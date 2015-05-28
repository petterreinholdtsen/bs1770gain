/*
 * ffsox_audio_player.c
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

#if defined (WIN32) // {
static audio_player_vmt_t vmt;

int ffsox_audio_player_create(ffsox_audio_player_t *ap,
    ffsox_frame_reader_t *fr, double q, IMMDevice *pDevice,
    AUDCLNT_SHAREMODE eShareMode)
{
  HRESULT hr;
  AVFrame *frame;
  IAudioClient *pAudioClient;
  WAVEFORMATEX *pwfx,**ppClosestMatch;
  REFERENCE_TIME hnsDevicePeriod;
  UINT32 nFrames;
  IAudioRenderClient *pRenderClient;

  // initialize the base class.
  if (ffsox_frame_consumer_create(&ap->frame_consumer)<0) {
    MESSAGE("creating frame consumer");
    goto base;
  }

  // set the vmt.
  ap->vmt=ffsox_audio_player_get_vmt();

  // set amplification/attenuation factor.
  ap->q=q;

  // reset the sync state flag.
  ap->sync.state=0;

  // create the sync mutex.
  ap->sync.hMutex=CreateMutex(
    NULL,         // _In_opt_  LPSECURITY_ATTRIBUTES lpMutexAttributes,
    FALSE,        // _In_      BOOL bInitialOwner,
    NULL          // _In_opt_  LPCTSTR lpName
  );

  if (NULL==ap->sync.hMutex) {
    MESSAGE("creating mutex");
    goto mutex;
  }

  // initialize the sync event.
  ap->sync.hEvent=CreateEvent(
    NULL,        // lpEventAttributes
    FALSE,       // bManualReset
    FALSE,       // bInitialState
    NULL         // lpName
  );

  if (NULL==ap->sync.hEvent) {
    MESSAGE("creating play event");
    goto event;
  }

  // create the frame.
  if (ffsox_frame_create(&ap->fo)<0) {
    MESSAGE("creating frame");
    goto frame;
  }

  frame=ap->fo.frame;
  frame->channel_layout=fr->si.cc->channel_layout;
  frame->channels=fr->si.cc->channels;
  frame->sample_rate=fr->si.cc->sample_rate;
  frame->nb_samples=0;

  // from the device, get an audio client.
  hr=pDevice->lpVtbl->Activate(pDevice,
    &IID_IAudioClient,        // [in]   REFIID iid,
    CLSCTX_ALL,               // [in]   DWORD dwClsCtx,
    NULL,                     // [in]   PROPVARIANT *pActivationParams,
    (void **)&pAudioClient    // [out]  void **ppInterface
  );

  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(E_NOINTERFACE);
      HR_ERROR_CASE(E_POINTER);
      HR_ERROR_CASE(E_INVALIDARG);
      HR_ERROR_CASE(E_OUTOFMEMORY);
      HR_ERROR_CASE(AUDCLNT_E_DEVICE_INVALIDATED);
    }

    MESSAGE("activating audio client");
    goto client;
  }

  ap->pAudioClient=pAudioClient;

  // from the audio client, query wether the format is supported.
  pwfx=&ap->wfx.Format;
  pwfx->wFormatTag=WAVE_FORMAT_EXTENSIBLE;
  pwfx->nChannels=fr->si.cc->channels;
  pwfx->nSamplesPerSec=fr->si.cc->sample_rate;

#if 0 // {
  switch (fr->si.cc->sample_fmt) {
  case AV_SAMPLE_FMT_S16:
  case AV_SAMPLE_FMT_S16P:
    pwfx->wBitsPerSample=16;
    ap->wfx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;
    frame->format=AV_SAMPLE_FMT_S16;
    break;
  case AV_SAMPLE_FMT_S32:
  case AV_SAMPLE_FMT_S32P:
    pwfx->wBitsPerSample=32;
    ap->wfx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;
    frame->format=AV_SAMPLE_FMT_S32;
    break;
#if 0 // {
  case AV_SAMPLE_FMT_FLT:
  case AV_SAMPLE_FMT_FLTP:
    pwfx->wBitsPerSample=32;
    ap->wfx.SubFormat=KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    frame->format=AV_SAMPLE_FMT_FLT;
    break;
  case AV_SAMPLE_FMT_DBL:
  case AV_SAMPLE_FMT_DBLP:
    pwfx->wBitsPerSample=64;
    ap->wfx.SubFormat=KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    frame->format=AV_SAMPLE_FMT_DBL;
    break;
#else // } {
  case AV_SAMPLE_FMT_FLT:
  case AV_SAMPLE_FMT_FLTP:
    pwfx->wBitsPerSample=16;
    ap->wfx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;
    frame->format=AV_SAMPLE_FMT_S16;
    break;
  case AV_SAMPLE_FMT_DBL:
  case AV_SAMPLE_FMT_DBLP:
    pwfx->wBitsPerSample=16;
    ap->wfx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;
    frame->format=AV_SAMPLE_FMT_S16;
    break;
#endif // }
  default:
    MESSAGE("format not supported");
    goto support;
  }
#elif 0 // {
  pwfx->wBitsPerSample=32;
  ap->wfx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;
  frame->format=AV_SAMPLE_FMT_S32;
#elif 1 // {
  pwfx->wBitsPerSample=16;
  ap->wfx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;
  frame->format=AV_SAMPLE_FMT_S16;
#else // } {
  pwfx->wBitsPerSample=32;
  ap->wfx.SubFormat=KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
  frame->format=AV_SAMPLE_FMT_FLT;
#endif // }

  //////
  pwfx->nBlockAlign=pwfx->nChannels*pwfx->wBitsPerSample/8;
  pwfx->nAvgBytesPerSec=pwfx->nSamplesPerSec*pwfx->nBlockAlign;
  pwfx->cbSize=(sizeof ap->wfx)-(sizeof ap->wfx.Format);
  //////
  ap->wfx.Samples.wValidBitsPerSample=pwfx->wBitsPerSample;
  ap->wfx.dwChannelMask=SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT;

  pwfx=NULL;
  ppClosestMatch=AUDCLNT_SHAREMODE_EXCLUSIVE==eShareMode?NULL:&pwfx;

  hr=pAudioClient->lpVtbl->IsFormatSupported(pAudioClient,
    eShareMode,             // [in]   AUDCLNT_SHAREMODE ShareMode,
    &ap->wfx.Format,        // [in]   const WAVEFORMATEX *pFormat,
    ppClosestMatch          // [out]  WAVEFORMATEX **ppClosestMatch
  );

  if (FAILED(hr)||S_FALSE==hr) {
    if (NULL!=pwfx) {
      CoTaskMemFree(pwfx);
    }

    switch (hr) {
      HR_ERROR_CASE(S_FALSE);
      HR_ERROR_CASE(AUDCLNT_E_UNSUPPORTED_FORMAT);
      HR_ERROR_CASE(E_POINTER);
      HR_ERROR_CASE(E_INVALIDARG);
      HR_ERROR_CASE(AUDCLNT_E_DEVICE_INVALIDATED);
      HR_ERROR_CASE(AUDCLNT_E_SERVICE_NOT_RUNNING);
    }

    MESSAGE("format not supported");
    goto support;
  }

#if 1 // {
  // get minimum device period (low latency).
  hr=pAudioClient->lpVtbl->GetDevicePeriod(pAudioClient,
    NULL,                 // [out]  REFERENCE_TIME *phnsDefaultDevicePeriod,
    &hnsDevicePeriod      // [out]  REFERENCE_TIME *phnsMinimumDevicePeriod
  );
#else // } {
  // get default device period.
  hr=pAudioClient->lpVtbl->GetDevicePeriod(pAudioClient,
    &hnsDevicePeriod,     // [out]  REFERENCE_TIME *phnsDefaultDevicePeriod,
    NULL                  // [out]  REFERENCE_TIME *phnsMinimumDevicePeriod
  );
#endif // }

  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(AUDCLNT_E_DEVICE_INVALIDATED);
      HR_ERROR_CASE(AUDCLNT_E_SERVICE_NOT_RUNNING);
      HR_ERROR_CASE(E_POINTER);
    }

    MESSAGE("getting device period");
    goto period;
  }

  // initialize the audio client.
  pwfx=&ap->wfx.Format;

  hr=pAudioClient->lpVtbl->Initialize(pAudioClient,
    eShareMode,           // [in]  AUDCLNT_SHAREMODE ShareMode,
    0,                    // [in]  DWORD StreamFlags,
    hnsDevicePeriod,      // [in]  REFERENCE_TIME hnsBufferDuration,
    0,                    // [in]  REFERENCE_TIME hnsPeriodicity,
    pwfx,                 // [in]  const WAVEFORMATEX *pFormat,
    NULL                  // [in]  LPCGUID AudioSessionGuid
  );

  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(AUDCLNT_E_ALREADY_INITIALIZED);
      HR_ERROR_CASE(AUDCLNT_E_WRONG_ENDPOINT_TYPE);
      HR_ERROR_CASE(AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED);
      HR_ERROR_CASE(AUDCLNT_E_BUFFER_SIZE_ERROR);
      HR_ERROR_CASE(AUDCLNT_E_CPUUSAGE_EXCEEDED);
      HR_ERROR_CASE(AUDCLNT_E_DEVICE_INVALIDATED);
      HR_ERROR_CASE(AUDCLNT_E_DEVICE_IN_USE);
      HR_ERROR_CASE(AUDCLNT_E_ENDPOINT_CREATE_FAILED);
      HR_ERROR_CASE(AUDCLNT_E_INVALID_DEVICE_PERIOD);
      HR_ERROR_CASE(AUDCLNT_E_UNSUPPORTED_FORMAT);
      HR_ERROR_CASE(AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED);
      HR_ERROR_CASE(AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL);
      HR_ERROR_CASE(AUDCLNT_E_SERVICE_NOT_RUNNING);
      HR_ERROR_CASE(E_POINTER);
      HR_ERROR_CASE(E_INVALIDARG);
      HR_ERROR_CASE(E_OUTOFMEMORY);
    }

    MESSAGE("initializing audio client");
    goto initialize;
  }

  // get the buffer size.
  //
  // This method retrieves the length of the endpoint buffer shared
  // between the client application and the audio engine. The length
  // is expressed as the number of audio frames the buffer can hold.
  // The size in bytes of an audio frame is calculated as the number
  // of channels in the stream multiplied by the sample size per channel.
  // For example, the frame size is four bytes for a stereo (2-channel)
  // stream with 16-bit samples.
  hr=pAudioClient->lpVtbl->GetBufferSize(pAudioClient,
    &nFrames                  // [out]  UINT32 *pNumBufferFrames
  );

  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(AUDCLNT_E_NOT_INITIALIZED);
      HR_ERROR_CASE(AUDCLNT_E_DEVICE_INVALIDATED);
      HR_ERROR_CASE(AUDCLNT_E_SERVICE_NOT_RUNNING);
      HR_ERROR_CASE(E_POINTER);
    }

    MESSAGE("getting buffer size");
    goto size;
  }

  ap->hnsDuration=(double)REFTIMES_PER_SEC*nFrames/pwfx->nSamplesPerSec;

  // get the render client.
  hr=pAudioClient->lpVtbl->GetService(pAudioClient,
    &IID_IAudioRenderClient,  // [in]   REFIID riid,
    (void**)&pRenderClient    // [out]  void **ppv
  );

  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(E_POINTER);
      HR_ERROR_CASE(E_NOINTERFACE);
      HR_ERROR_CASE(AUDCLNT_E_NOT_INITIALIZED);
      HR_ERROR_CASE(AUDCLNT_E_WRONG_ENDPOINT_TYPE);
      HR_ERROR_CASE(AUDCLNT_E_DEVICE_INVALIDATED);
      HR_ERROR_CASE(AUDCLNT_E_SERVICE_NOT_RUNNING);
    }

    MESSAGE("getting render client");
    goto render;
  }

  ap->pRenderClient=pRenderClient;

  // allocate frame buffer.
  ap->nb_samples=nFrames;
  frame->nb_samples=ap->nb_samples;

  if (NULL==(frame->data[0]=malloc(nFrames*pwfx->nBlockAlign))) {
    MESSAGE("allocatiing frame buffer");
    goto buffer;
  }

  return 0;
// cleanup:
  free(frame->data[0]);
buffer:
  ap->pRenderClient->lpVtbl->Release(ap->pRenderClient);
render:
size:
initialize:
period:
support:
  ap->pAudioClient->lpVtbl->Release(ap->pAudioClient);
client:
  ffsox_frame_cleanup(&ap->fo);
frame:
  CloseHandle(ap->sync.hEvent);
event:
  CloseHandle(ap->sync.hMutex);
mutex:
  vmt.parent->cleanup(&ap->frame_consumer);
base:
  return -1;
}

ffsox_audio_player_t *ffsox_audio_player_new(ffsox_frame_reader_t *fr,
    double q, IMMDevice *pDevice, AUDCLNT_SHAREMODE eShareMode)
{
  audio_player_t *ap;

  if (NULL==(ap=malloc(sizeof *ap))) {
    MESSAGE("allocating audio player");
    goto malloc;
  }

  if (ffsox_audio_player_create(ap,fr,q,pDevice,eShareMode)<0) {
    MESSAGE("creating audio player");
    goto create;
  }

  return ap;
create:
  free(ap);
malloc:
  return NULL;
}

////////
static int audio_player_render(audio_player_t *ap)
{
  HRESULT hr;
  IAudioClient *pAudioClient=ap->pAudioClient;
  IAudioRenderClient *pRenderClient=ap->pRenderClient;
  frame_t *fo=&ap->fo;
  AVFrame *frame=fo->frame;
  UINT32 nBlockAlign=ap->wfx.Format.nBlockAlign;
  UINT32 nFrames,nPaddingFrames;
  BYTE *pData;
  size_t size1,size2;

  hr=pAudioClient->lpVtbl->GetCurrentPadding(pAudioClient,
    &nPaddingFrames     // [out]  UINT32 *pNumPaddingFrames
  );

  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(AUDCLNT_E_NOT_INITIALIZED);
      HR_ERROR_CASE(AUDCLNT_E_DEVICE_INVALIDATED);
      HR_ERROR_CASE(AUDCLNT_E_SERVICE_NOT_RUNNING);
      HR_ERROR_CASE(E_POINTER);
    }

    MESSAGE("getting padding");
    return -1;
  }

  nFrames=frame->nb_samples-nPaddingFrames;

  hr=pRenderClient->lpVtbl->GetBuffer(pRenderClient,
    nFrames,            // [in]   UINT32 NumFramesRequested,
    &pData              // [out]  BYTE **ppData
  );

  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(AUDCLNT_E_BUFFER_ERROR);
      HR_ERROR_CASE(AUDCLNT_E_BUFFER_TOO_LARGE);
      HR_ERROR_CASE(AUDCLNT_E_BUFFER_SIZE_ERROR);
      HR_ERROR_CASE(AUDCLNT_E_OUT_OF_ORDER);
      HR_ERROR_CASE(AUDCLNT_E_DEVICE_INVALIDATED);
      HR_ERROR_CASE(AUDCLNT_E_BUFFER_OPERATION_PENDING);
      HR_ERROR_CASE(AUDCLNT_E_SERVICE_NOT_RUNNING);
      HR_ERROR_CASE(E_POINTER);
    }

    MESSAGE("getting buffer");
    return -1;
  }

  size1=nFrames*nBlockAlign;
  memcpy(pData,frame->data[0],size1);

  hr=pRenderClient->lpVtbl->ReleaseBuffer(pRenderClient,
    nFrames,        // [in]  UINT32 NumFramesWritten,
    0               // [in]  DWORD dwFlags
  );

  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(AUDCLNT_E_INVALID_SIZE);
      HR_ERROR_CASE(AUDCLNT_E_BUFFER_SIZE_ERROR);
      HR_ERROR_CASE(AUDCLNT_E_OUT_OF_ORDER);
      HR_ERROR_CASE(AUDCLNT_E_DEVICE_INVALIDATED);
      HR_ERROR_CASE(AUDCLNT_E_SERVICE_NOT_RUNNING);
      HR_ERROR_CASE(E_INVALIDARG);
    }

    MESSAGE("releasing buffer");
    return -1;
  }

  size2=frame->nb_samples*nBlockAlign;
  size2-=size1;

  if (0<size2)
    memmove(frame->data[0],frame->data[0]+size1,size2);

  fo->nb_samples.frame-=nFrames;
  fo->nb_samples.stream+=nFrames;

  return 0;
}

static DWORD WINAPI ffsox_audio_player_thread(LPVOID lpParameter)
{
  audio_player_t *ap=lpParameter;
  IAudioClient *pAudioClient=ap->pAudioClient;
  int stop=0;
  HRESULT hr;

  hr=pAudioClient->lpVtbl->Start(pAudioClient); 

  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(AUDCLNT_E_NOT_INITIALIZED);
      HR_ERROR_CASE(AUDCLNT_E_NOT_STOPPED);
      HR_ERROR_CASE(AUDCLNT_E_EVENTHANDLE_NOT_SET);
      HR_ERROR_CASE(AUDCLNT_E_DEVICE_INVALIDATED);
      HR_ERROR_CASE(AUDCLNT_E_SERVICE_NOT_RUNNING);
    }

    MESSAGE("starting audio client");
    goto start;
  }

  do {
    Sleep((DWORD)(ap->hnsDuration/REFTIMES_PER_MILLISEC/2));
    WaitForSingleObject(ap->sync.hMutex,INFINITE);

    while (0==ap->sync.state) {
      SignalObjectAndWait(ap->sync.hMutex,ap->sync.hEvent,INFINITE,FALSE);
      WaitForSingleObject(ap->sync.hMutex,INFINITE);
    }

    switch (ap->sync.state) {
    case AUDIO_PLAYER_RENDER:
      if (audio_player_render(ap)<0) {
        MESSAGE("rendering audio");
        stop=1;
      }

      break;
    case AUDIO_PLAYER_END:
      stop=1;
      break;
    default:
      MESSAGE("illeagl sync state");
      stop=1;
      break;
    }

    ap->sync.state=0;
    SetEvent(ap->sync.hEvent);
    ReleaseMutex(ap->sync.hMutex);
  } while (0==stop);

  Sleep((DWORD)(ap->hnsDuration/REFTIMES_PER_MILLISEC/2));
// cleanup:
  pAudioClient->lpVtbl->Stop(pAudioClient); 
start:
  return 0;
}

void ffsox_audio_player_notify(audio_player_t *ap, int state)
{
  WaitForSingleObject(ap->sync.hMutex,INFINITE);

  while (0!=(ap->sync.state)) {
    SignalObjectAndWait(ap->sync.hMutex,ap->sync.hEvent,INFINITE,FALSE);
    WaitForSingleObject(ap->sync.hMutex,INFINITE);
  }

  ap->sync.state=state;
  SetEvent(ap->sync.hEvent);
  ReleaseMutex(ap->sync.hMutex);
}

int ffsox_audio_player_play(audio_player_t *ap)
{
  int code=-1;
  HANDLE hThread;
  machine_t m;

  if (ffsox_machine_run(&m,&ap->node)<0) {
    MESSAGE("running machine");
    goto pfx;
  }

  hThread=CreateThread(
    NULL,   // _In_opt_   LPSECURITY_ATTRIBUTES lpThreadAttributes,
    0,      // _In_       SIZE_T dwStackSize,
    ffsox_audio_player_thread,
            // _In_       LPTHREAD_START_ROUTINE lpStartAddress,
    ap,     // _In_opt_   LPVOID lpParameter,
    0,      // _In_       DWORD dwCreationFlags,
    NULL    // _Out_opt_  LPDWORD lpThreadId
  );

  if (NULL==hThread) {
    MESSAGE("creating thread");
    goto thread;
  }

  while (STATE_END!=ap->state) {
    if (ffsox_machine_run(&m,&ap->node)<0) {
      MESSAGE("running machine");
      goto run;
    }
  }

  code=0;
// cleanup:
run:
  WaitForSingleObject(hThread,INFINITE);
  CloseHandle(hThread);
thread:
pfx:
  return code;
}

void ffsox_audio_player_kill(audio_player_t *ap)
{
  DWORD time;

  time=1.5*ap->hnsDuration/REFTIMES_PER_MILLISEC;
  ffsox_audio_player_notify(ap,AUDIO_PLAYER_END);
  Sleep(time);
}

////////
static void audio_player_cleanup(audio_player_t *ap)
{
  free(ap->fo.frame->data[0]);
  ap->fo.frame->data[0]=NULL;
  ap->pRenderClient->lpVtbl->Release(ap->pRenderClient);
  ap->pAudioClient->lpVtbl->Release(ap->pAudioClient);
  ffsox_frame_cleanup(&ap->fo);
  CloseHandle(ap->sync.hEvent);
  CloseHandle(ap->sync.hMutex);
  vmt.parent->cleanup(&ap->frame_consumer);
}

static int audio_player_run(audio_player_t *ap)
{
  frame_t *fi=ap->fi;
  frame_t *fo=&ap->fo;

  switch (ap->state) {
  case STATE_RUN:
    if (NULL!=fi) {
      while (0==ffsox_frame_complete(fi)) {
        if (ffsox_frame_convert(fi,fo,ap->q)<0) {
          MESSAGE("converting frame");
          return -1;
        }

        if (0!=ffsox_frame_complete(fo)) {
          ffsox_audio_player_notify(ap,AUDIO_PLAYER_RENDER);
          return MACHINE_PUSH;
        }
      }

      ffsox_frame_reset(fi);
    }

    return MACHINE_POP;
  case STATE_FLUSH:
    if (0<fo->nb_samples.frame) {
      fo->frame->nb_samples=fo->nb_samples.frame;
      ffsox_audio_player_notify(ap,AUDIO_PLAYER_RENDER);
    }

    ffsox_audio_player_notify(ap,AUDIO_PLAYER_END);
    ap->state=STATE_END;

    return MACHINE_POP;
  case STATE_END:
    return MACHINE_POP;
  default:
    MESSAGE("illegal state");
    return -1;
  }
}

const audio_player_vmt_t *ffsox_audio_player_get_vmt(void)
{
  const frame_consumer_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_frame_consumer_get_vmt();
    vmt.frame_consumer=*parent;
    vmt.parent=parent;
    vmt.name="audio_player";
    vmt.cleanup=audio_player_cleanup;
    vmt.run=audio_player_run;
  }

  return &vmt;
}
#endif // }
