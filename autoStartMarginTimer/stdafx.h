// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NTDDI_VERSION NTDDI_WIN10_RS4
#define WINVER _WIN32_WINNT_WIN10
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#define _WIN32_IE _WIN32_IE_IE110
#define NOATOM
#define NOMEMMGR
#define NOOPENFILE
#define NOMINMAX
#define NOSERVICE
#define NOCOMM
#define NOKANJI
#define NOMCX
#define NOSOUND
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#define _ATL_NO_COM
#define _ATL_NO_COM_SUPPORT
#define _ATL_NO_EXCEPTIONS
#define _ATL_NO_HOSTING
#define _ATL_NO_OPENGL
#define _ATL_NO_PERF_SUPPORT
#define _ATL_NO_SERVICE

#include <atlbase.h>
#include <atlapp.h>

#include <atlwin.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include <memory>
#include <psapi.h>
#include <shellapi.h>

#include <functional>
#include <dxgi.h>
#include <d3d11.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.Capture.h>

#include <opencv2/opencv.hpp>

#define WM_T99TIMER_OBSCLOSED    WM_APP + 1
#define WM_T99TIMER_INITIALIZED  WM_APP + 2
#define WM_T99TIMER_MATCHING     WM_APP + 3
#define WM_T99TIMER_READY        WM_APP + 4
#define WM_T99TIMER_START        WM_APP + 5


#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
