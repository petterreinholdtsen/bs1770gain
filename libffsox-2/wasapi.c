/*
 * wasapi.c
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

#if ! defined (PKEY_Device_FriendlyName) // {
DEFINE_PROPERTYKEY(PKEY_Device_FriendlyName,
    0xa45c254e,0xdf1c,0x4efd,0x80,0x20,0x67,0xd1,0x46,0xa8,0x50,0xe0,14);
#endif // }

//#define DEVICE_PRINT_ID
HRESULT DevicePrint(IMMDevice *pDevice, UINT nDevice, FILE *f)
{
  HRESULT hr;
#if defined (DEVICE_PRINT_ID) // {
  LPWSTR id;
#endif // }
  PROPVARIANT varName;
  IPropertyStore *pProperties;

  PropVariantInit(&varName);

#if defined (DEVICE_PRINT_ID) // {
  hr=pDevice->lpVtbl->GetId(pDevice,
    &id                         // [out]  LPWSTR *ppstrId
  );
  
  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(E_OUTOFMEMORY);
      HR_ERROR_CASE(E_POINTER);
    }

    MESSAGE("getting device id");
    goto id;
  }
#endif // }

  hr=pDevice->lpVtbl->OpenPropertyStore(pDevice,
    STGM_READ,                  // [in]   DWORD stgmAccess,
    &pProperties                // [out]  IPropertyStore **ppProperties
  );
  
  if (FAILED(hr)) {
    switch (hr) {
      HR_ERROR_CASE(E_INVALIDARG);
      HR_ERROR_CASE(E_POINTER);
      HR_ERROR_CASE(E_OUTOFMEMORY);
    }

    MESSAGE("opening device property store");
    goto prop;
  }

  hr=pProperties->lpVtbl->GetValue(pProperties,
    &PKEY_Device_FriendlyName,  // [in]   REFPROPERTYKEY key,
    &varName                    // [out]  PROPVARIANT *pv
  );

  if (FAILED(hr)) {
    MESSAGE("getting device name");
    goto name;
  }

  if (NULL!=f)
#if defined (DEVICE_PRINT_ID) // {
    fwprintf(f,L"Device %d %s: \"%s\"\n",nDevice,id,varName.pwszVal);
#else // } {
    fwprintf(f,L"Device %d: \"%s\"\n",nDevice,varName.pwszVal);
#endif // }

// cleanup:
name:
  pProperties->lpVtbl->Release(pProperties);
prop:
#if defined (DEVICE_PRINT_ID) // {
  CoTaskMemFree(id);
id:
#endif // }
  PropVariantClear(&varName);
  return hr;
}

HRESULT DeviceUse(IMMDevice *pDevice, AUDCLNT_SHAREMODE eShareMode,
    WORD wBitsPerSample, WORD nChannels, DWORD nSamplesPerSec, FILE *f)
{
  HRESULT hr;
  IAudioClient *pAudioClient;
  WAVEFORMATEXTENSIBLE wfx;
  WAVEFORMATEX *pwfx,**ppClosestMatch;
  REFERENCE_TIME hnsDevicePeriod;
  UINT32 numBufferFrames;

  // from the device, get an audio client ("audioclient.h").
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

  // from the audio client, query wether the format is supported.
  wfx.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
  wfx.Format.nChannels=nChannels;
  wfx.Format.nSamplesPerSec=nSamplesPerSec;
  wfx.Format.wBitsPerSample=wBitsPerSample;
  //////
  wfx.Format.nBlockAlign=nChannels*wBitsPerSample/8;
  wfx.Format.nAvgBytesPerSec=nSamplesPerSec*wfx.Format.nBlockAlign;
  wfx.Format.cbSize=(sizeof wfx)-(sizeof wfx.Format);
  //////
  wfx.Samples.wValidBitsPerSample=wBitsPerSample;
  wfx.dwChannelMask=SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT;
  wfx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;

  pwfx=NULL;
  ppClosestMatch=AUDCLNT_SHAREMODE_EXCLUSIVE==eShareMode?NULL:&pwfx;

  hr=pAudioClient->lpVtbl->IsFormatSupported(pAudioClient,
    eShareMode,             // [in]   AUDCLNT_SHAREMODE ShareMode,
    &wfx.Format,            // [in]   const WAVEFORMATEX *pFormat,
    ppClosestMatch          // [out]  WAVEFORMATEX **ppClosestMatch
  );

  if (FAILED(hr)) {
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

  pwfx=&wfx.Format;

  if (NULL!=f) {
    fwprintf(f,L"  NCH=%d, RATE=%d, BPS=%d.\n",pwfx->nChannels,
	    pwfx->nSamplesPerSec,pwfx->wBitsPerSample);
  }

  // get the minimum device period (low latency).
  hr=pAudioClient->lpVtbl->GetDevicePeriod(pAudioClient,
    NULL,                 // [out]  REFERENCE_TIME *phnsDefaultDevicePeriod,
    &hnsDevicePeriod      // [out]  REFERENCE_TIME *phnsMinimumDevicePeriod
  );

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
  //
  // numBufferFrames = wfx.Format.nBlockAlign
  hr=pAudioClient->lpVtbl->GetBufferSize(pAudioClient,
    &numBufferFrames      // [out]  UINT32 *pNumBufferFrames
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

  // calculate how many samples fit into the buffer.

  fprintf(stderr,"  yep!\n");
// cleanup:
size:
initialize:
period:
  if (AUDCLNT_SHAREMODE_EXCLUSIVE==eShareMode)
    CoTaskMemFree(pwfx);
support:
  pAudioClient->lpVtbl->Release(pAudioClient);
client:
  return hr;
}

HRESULT Device(IMMDevice *pDevice, UINT nDevice)
{
  HRESULT hr;

  // print the device's name, and finally
  hr=DevicePrint(pDevice,nDevice,stdout);

  if (FAILED(hr)) {
    MESSAGE("printing device");
    goto print;
  }

  // use the device.
  hr=DeviceUse(pDevice,AUDCLNT_SHAREMODE_EXCLUSIVE,24,2,48000,stdout);
  //hr=DeviceUse(pDevice,AUDCLNT_SHAREMODE_SHARED,24,2,48000,stdout);

  if (FAILED(hr)) {
    MESSAGE("using device");
    goto device;
  }
device:
print:
  return hr;
}

int main(int argc, char **argv)
{
  HRESULT hr;
  IMMDeviceEnumerator *pEnumerator;
#if defined (ENUMERATE) // {
  IMMDeviceCollection *pDevices;
  UINT cDevices,nDevice;
#endif // }
  IMMDevice *pDevice;

  // initialize COM.
  hr=CoInitializeEx(
    NULL,                       // _In_opt_  LPVOID pvReserved,
    COINIT_MULTITHREADED        // _In_      DWORD dwCoInit
  );

  if (FAILED(hr)) {
    MESSAGE("initializing COM");
    goto coinit;
  }

  // create a device enumerator ("mmdeviceapi.h").
  hr=CoCreateInstance(
    &CLSID_MMDeviceEnumerator,  // _In_   REFCLSID rclsid,
    NULL,                       // _In_   LPUNKNOWN pUnkOuter,
    CLSCTX_ALL,                 // _In_   DWORD dwClsContext,
    &IID_IMMDeviceEnumerator,   // _In_   REFIID riid,
    (void**)&pEnumerator        // _Out_  LPVOID *ppv
  );

  if (FAILED(hr)) {
    MESSAGE("creating device enumerator");
    goto devnum;
  }

#if defined (ENUMERATE) // {
  // from the device enumerator, create a device collection.
  // we're interested just in the active rendering devices.
  hr=pEnumerator->lpVtbl->EnumAudioEndpoints(pEnumerator,
    eRender,                    // [in]   EDataFlow dataFlow,
    DEVICE_STATE_ACTIVE,        // [in]   DWORD dwStateMask,
    &pDevices                   // [out]  IMMDeviceCollection **ppDevices
  );
  
  if (FAILED(hr)) {
    MESSAGE("creating device collection");
    goto devcoll;
  }

  // from the device collection, get the number of devices.
  hr=pDevices->lpVtbl->GetCount(pDevices,
    &cDevices                   // [out]  UINT *pcDevices
  );

  if (FAILED(hr)) {
    MESSAGE("getting device count");
    goto devcount;
  }

  // for each device ...
  for (nDevice=0;nDevice<cDevices;++nDevice) {
    // ... from the device collection, get the respective item,
    hr=pDevices->lpVtbl->Item(pDevices,
      nDevice,                  // [in]   UINT nDevice,
      &pDevice                  // [out]  IMMDevice **ppDevice
    );
  
    if (FAILED(hr)) {
      MESSAGE("getting device");
      goto item;
    }

    hr=Device(pDevice,nDevice);
  
    if (FAILED(hr)) {
      MESSAGE("using device");
      goto use;
    }
  // cleanup:
  use:
    pDevice->lpVtbl->Release(pDevice);
  item:
    ;
  }
#else // } {
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

    MESSAGE("getting default device");
    goto device;
  }

  hr=Device(pDevice,0);

  if (FAILED(hr)) {
    MESSAGE("using device");
    goto use;
  }
#endif // }

  fprintf(stdout,"yep!\n");
// cleanup:
#if defined (ENUMERATE) // {
devcount:
  pDevices->lpVtbl->Release(pDevices);
devcoll:
#else // } {
use:
  pDevice->lpVtbl->Release(pDevice);
device:
#endif // }
  pEnumerator->lpVtbl->Release(pEnumerator);
devnum:
  CoUninitialize();
coinit:
  return 0;
}
