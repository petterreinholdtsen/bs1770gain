/*
 * ffsox.c
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
#include <ffsox_priv.h>
#define COBJMACROS
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

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
  hr=IMMDevice_GetId(
    pDevice,
    &id                         // [out]  LPWSTR *ppstrId
  );
  
  if (FAILED(hr)) {
    MESSAGE("getting device id");
    goto id;
  }
#endif // }

  hr=IMMDevice_OpenPropertyStore(
    pDevice,
    STGM_READ,                  // [in]   DWORD stgmAccess,
    &pProperties                // [out]  IPropertyStore **ppProperties
  );
  
  if (FAILED(hr)) {
    MESSAGE("opening device property store");
    goto prop;
  }

  hr=IPropertyStore_GetValue(
    pProperties,
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
  IPropertyStore_Release(pProperties);
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
  const wchar_t *pfx;
  //REFERENCE_TIME hnsDuration=0;

  // from the device, get an audio client ("audioclient.h").
  hr=IMMDevice_Activate(
    pDevice,
    &IID_IAudioClient,        // [in]   REFIID iid,
    CLSCTX_ALL,               // [in]   DWORD dwClsCtx,
    NULL,                     // [in]   PROPVARIANT *pActivationParams,
    (void **)&pAudioClient    // [out]  void **ppInterface
  );

  if (FAILED(hr)) {
    MESSAGE("activationg audio client");
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

  hr=IAudioClient_IsFormatSupported(
    pAudioClient,
    eShareMode,             // [in]   AUDCLNT_SHAREMODE ShareMode,
    &wfx.Format,            // [in]   const WAVEFORMATEX *pFormat,
    ppClosestMatch          // [out]  WAVEFORMATEX **ppClosestMatch
  );

  if (S_OK==hr) {
    // the format is supported.
    pwfx=&wfx.Format;
    pfx=L"SUPPORTED";
  }
  else if (S_FALSE==hr) {
    // the format is not supported but an alternative is proposed.
    pfx=L"ALTERNATE";
  }
  else {
    // the format is not supported at all.
    MESSAGE("format not supported");
    goto support;
  }

  if (NULL!=f) {
    fwprintf(f,L"  %s: NCH=%d, RATE=%d, BPS=%d.\n",pfx,pwfx->nChannels,
	    pwfx->nSamplesPerSec,pwfx->wBitsPerSample);
  }

  // we're only interested in an exact match.
  if (FAILED(hr)) {
    MESSAGE("format not supported");
    goto alternate;
  }

#if 0 // {
#if 0 // {
  //
  hr=pAudioClient->GetDevicePeriod(
    NULL,                 // [out]  REFERENCE_TIME *phnsDefaultDevicePeriod,
    &hnsDuration          // [out]  REFERENCE_TIME *phnsMinimumDevicePeriod
  );
  
  if (FAILED(hr)) {
    MESSAGE("getting audio client period");
    goto period;
  }

  // initialize the audio client.
  hr=pAudioClient->Initialize(
    eShareMode,           // [in]  AUDCLNT_SHAREMODE ShareMode,
    AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                          // [in]  DWORD StreamFlags,
    hnsDuration,          // [in]  REFERENCE_TIME hnsBufferDuration,
    hnsDuration,          // [in]  REFERENCE_TIME hnsPeriodicity,
    pwfx,                 // [in]  const WAVEFORMATEX *pFormat,
    NULL                  // [in]  LPCGUID AudioSessionGuid
  );
#else // } {
  //
  hr=IAudioClient_GetDevicePeriod(
    pAudioClient,
    NULL,                 // [out]  REFERENCE_TIME *phnsDefaultDevicePeriod,
    &hnsDuration          // [out]  REFERENCE_TIME *phnsMinimumDevicePeriod
  );
fprintf(stderr,">>> %I64d\n",hnsDuration);
  
  if (FAILED(hr)) {
    MESSAGE("getting audio client period");
    goto period;
  }

  // initialize the audio client.
  hr=IAudioClient_Initialize(
    pAudioClient,
    eShareMode,           // [in]  AUDCLNT_SHAREMODE ShareMode,
    AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                          // [in]  DWORD StreamFlags,
    30000,//hnsDuration,          // [in]  REFERENCE_TIME hnsBufferDuration,
    3,//hnsDuration,          // [in]  REFERENCE_TIME hnsPeriodicity,
    pwfx,                 // [in]  const WAVEFORMATEX *pFormat,
    NULL                  // [in]  LPCGUID AudioSessionGuid
  );
#endif // }
  
  if (FAILED(hr)) {
    switch (hr) {
    case AUDCLNT_E_ALREADY_INITIALIZED:
      fputs("AUDCLNT_E_ALREADY_INITIALIZED\n",stderr);
      break;
    case AUDCLNT_E_WRONG_ENDPOINT_TYPE:
      fputs("AUDCLNT_E_WRONG_ENDPOINT_TYPE\n",stderr);
      break;
    case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED:
      fputs("AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED\n",stderr);
      break;
    case AUDCLNT_E_BUFFER_SIZE_ERROR:
      fputs("AUDCLNT_E_BUFFER_SIZE_ERROR\n",stderr);
      break;
    case AUDCLNT_E_CPUUSAGE_EXCEEDED:
      fputs("AUDCLNT_E_CPUUSAGE_EXCEEDED\n",stderr);
      break;
    case AUDCLNT_E_DEVICE_INVALIDATED:
      fputs("AUDCLNT_E_DEVICE_INVALIDATED\n",stderr);
      break;
    case AUDCLNT_E_DEVICE_IN_USE:
      fputs("AUDCLNT_E_DEVICE_IN_USE\n",stderr);
      break;
    case AUDCLNT_E_ENDPOINT_CREATE_FAILED:
      fputs("AUDCLNT_E_ENDPOINT_CREATE_FAILED\n",stderr);
      break;
    case AUDCLNT_E_INVALID_DEVICE_PERIOD:
      fputs("AUDCLNT_E_INVALID_DEVICE_PERIOD\n",stderr);
      break;
    case AUDCLNT_E_UNSUPPORTED_FORMAT:
      fputs("AUDCLNT_E_UNSUPPORTED_FORMAT\n",stderr);
      break;
    case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED:
      fputs("AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED\n",stderr);
      break;
    case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL:
      fputs("AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL\n",stderr);
      break;
    case AUDCLNT_E_SERVICE_NOT_RUNNING:
      fputs("AUDCLNT_E_SERVICE_NOT_RUNNING\n",stderr);
      break;
    case E_POINTER:
      fputs("E_POINTER\n",stderr);
      break;
    case E_INVALIDARG:
      fputs("E_INVALIDARG\n",stderr);
      break;
    case E_OUTOFMEMORY:
      fputs("E_OUTOFMEMORY\n",stderr);
      break;
    default:
      fputs("default\n",stderr);
      break;
    }

    MESSAGE("initializing audio client");
    goto init;
  }
// cleanup:
init:
period:
#endif // }
alternate:
  if (AUDCLNT_SHAREMODE_EXCLUSIVE==eShareMode)
    CoTaskMemFree(pwfx);
support:
  IAudioClient_Release(pAudioClient);
client:
  return hr;
}

int main(int argc, char **argv)
{
  HRESULT hr;
  IMMDeviceEnumerator *pEnumerator;
  IMMDeviceCollection *pDevices;
  UINT cDevices,nDevice;
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

  // from the device enumerator, create a device collection.
  // we're interested just in the active rendering devices.
  hr=IMMDeviceEnumerator_EnumAudioEndpoints(
    pEnumerator,
    eRender,                    // [in]   EDataFlow dataFlow,
    DEVICE_STATE_ACTIVE,        // [in]   DWORD dwStateMask,
    &pDevices                   // [out]  IMMDeviceCollection **ppDevices
  );
  
  if (FAILED(hr)) {
    MESSAGE("creating device collection");
    goto devcoll;
  }

  // from the device collection, get the number of devices.
  hr=IMMDeviceCollection_GetCount(
    pDevices,
    &cDevices                   // [out]  UINT *pcDevices
  );

  if (FAILED(hr)) {
    MESSAGE("getting device count");
    goto devcount;
  }

  // for each device ...
  for (nDevice=0;nDevice<cDevices;++nDevice) {
    // ... from the device collection, get the respective item,
    hr=IMMDeviceCollection_Item(
      pDevices,
      nDevice,                  // [in]   UINT nDevice,
      &pDevice                  // [out]  IMMDevice **ppDevice
    );
  
    if (FAILED(hr)) {
      MESSAGE("getting device");
      goto item;
    }

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
    IMMDevice_Release(pDevice);
  item:
    ;
  }

  fprintf(stdout,"yep!\n");
// cleanup:
devcount:
  IMMDeviceCollection_Release(pDevices);
devcoll:
  IMMDeviceEnumerator_Release(pEnumerator);
devnum:
  CoUninitialize();
coinit:
  return 0;
}
