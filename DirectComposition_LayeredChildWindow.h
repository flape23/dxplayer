// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <new>
#include <atlbase.h>
#include <atlhost.h>
#include <windows.h>
#include <windowsx.h>
#include <mfplay.h>
#include <mferror.h>
#include <shobjidl.h>   // defines IFileOpenDialog
#include <strsafe.h>
#include <math.h>
#include <d2d1.h>
#include <wincodec.h>

#include "resource.h"
class CApplication
{
    public: 
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
        static VOID ShowErrorMessage(PCWSTR format, HRESULT hr);
        static VOID OnMediaItemCreated(MFP_MEDIAITEM_CREATED_EVENT *pEvent);
        static VOID OnMediaItemSet(MFP_MEDIAITEM_SET_EVENT *pEvent);

    public:
        explicit CApplication(HINSTANCE hInstance);
        ~CApplication();

        int Run();

    private:		
        CApplication(const CApplication &); 

	private:
        HRESULT PlayMediaFile(HWND hwnd,const WCHAR *szPath);
		PWSTR pwszFilePath;
        HRESULT Initialize();

        // Initialize main window functions including layered child window
        HRESULT InitializeMainWindow();
       
	// Window message handlers
        LRESULT OnClose(HWND hwnd);
        HRESULT OnCommand(int id);
        LRESULT OnDestroy(HWND hwnd); 
        HRESULT OnPaint(HWND hwnd);
        VOID OnFileOpen(HWND hwnd);
		LRESULT OnKeyDown(int key);
		//HRESULT OnRightClick();
        
       int EnterMessageLoop();

        // Destroy
        VOID Destroy();
        VOID DestroyMainWindow();

    private:
        static CApplication *s_application;
        static CComPtr<IMFPMediaPlayer> s_pPlayer;

        static const float s_fanimationTime;

        HINSTANCE m_hInstance;
		const WCHAR *sURL;
        WCHAR m_fontTypeface[32];
        int   m_fontHeightLogo;
        int   m_fontHeightTitle;
        int   m_fontHeightDescription;

        HWND  m_hMainWindow;         // Main window
		ID2D1Factory *m_pD2DFactory;
		IWICImagingFactory *m_pWICFactory;
		ID2D1HwndRenderTarget *m_pRenderTarget;
              
        //MediaPlayerCallBack
        BOOL m_bControlOn;
        CComPtr<IMFPMediaPlayerCallback> s_pPlayerCB;

        enum    
        {
            ID_PLAYSTOP,        // Play/Stop button ID
            ID_ROTATE,          // Rotate button ID 
            ID_SCALE,           // Scale button ID 
            ID_SKEW,            // Skew button ID
            IDT_TIMER           // Timer ID
        };                 
};

// Implements the callback interface for MFPlay events.
class MediaPlayerCallback : public IMFPMediaPlayerCallback 
{
    private:
    
        long m_cRef; // Reference count

    public:
    
        MediaPlayerCallback() : m_cRef(1)
        {
        }

        STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
        {
            HRESULT hr = S_OK;
            *ppv = NULL; 

            if (riid == __uuidof(IMFPMediaPlayerCallback))
            {
                *ppv = static_cast<IMFPMediaPlayerCallback*>(this);
            }

            else if (riid == __uuidof(IUnknown))
            {
                *ppv = static_cast<IUnknown*>(this);
            }

            else
            {
                hr = E_NOINTERFACE;
            }

            if (SUCCEEDED(hr))
            {
                AddRef();
            }

            return hr;
        }

        STDMETHODIMP_(ULONG) AddRef() 
        {
            return InterlockedIncrement(&m_cRef); 
        }
    
        STDMETHODIMP_(ULONG) Release()
        {
            ULONG count = InterlockedDecrement(&m_cRef);
        
            if (count == 0)
            {
                delete this;
            }

            return count;
        }

        // IMFPMediaPlayerCallback methods
        void STDMETHODCALLTYPE OnMediaPlayerEvent(_In_ MFP_EVENT_HEADER *pEventHeader);
};