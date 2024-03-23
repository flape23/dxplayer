// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "resource.h"
#include "DirectComposition_LayeredChildWindow.h"

CApplication *CApplication::s_application           = nullptr;
CComPtr<IMFPMediaPlayer> CApplication::s_pPlayer    = nullptr;             // The MFPlay player object

BOOL m_bHasVideo = FALSE;

const float CApplication::s_fanimationTime = 6.0;     // 6 seconds animation

CApplication::CApplication(HINSTANCE instance) :
    m_hInstance(instance),
    m_bControlOn(TRUE),
    m_hMainWindow(NULL),
	pwszFilePath(NULL)
{
    s_application = this;
}
	//m_hControlChildWindow(NULL),
    //m_hVideoChildWindow(NULL),
    //m_hTextChildWindow(NULL)

CApplication::~CApplication()
{
    s_application = nullptr;
}

// Provides the entry point to the application.
INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE,  _In_ LPWSTR, _In_ INT)
{
    CApplication application(hInstance);
    return application.Run();
}

int CApplication::Run()
{
    int result = 0;

    if (SUCCEEDED(Initialize()))
    {
       result = EnterMessageLoop();
    }

    else
    {
       MessageBoxW(NULL, L"An error occuring when running the sample", NULL, MB_OK);
    }

    Destroy();

    return result;
}

//------------------------------------------------------
// Initialization
//------------------------------------------------------

HRESULT CApplication::Initialize()
{
    HRESULT hr = InitializeMainWindow();
	// Create a Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	if (SUCCEEDED(hr))
	{
		// Create WIC factory.
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void **>(&m_pWICFactory)
			);
	}

    return hr;
}

// Creates the main application window.
HRESULT CApplication::InitializeMainWindow()
{
    HRESULT hr = S_OK;

    // Register the window class.
    WNDCLASSEX wc     = {0};
    wc.cbSize         = sizeof(wc);
    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = WindowProc;
    wc.hInstance      = m_hInstance;
    wc.hIcon          = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszClassName  = "DirectComposition Window Class";
	wc.lpszMenuName   = MAKEINTRESOURCE(IDC_SOUNDMETERAPP);
    wc.hIconSm        = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
    RegisterClassEx(&wc);
	
	// Get desktop dc
	HDC desktopDc = GetDC(NULL);
	// Get native resolution
	int dpiX = GetDeviceCaps(desktopDc,LOGPIXELSX);
	int dpiY = GetDeviceCaps(desktopDc,LOGPIXELSY);
	
	// Creates the m_hMainWindow window.
	m_hMainWindow = CreateWindow(  wc.lpszClassName,                                // Name of window class
                                   "DirectComposition Layered Child Window Sample", 	// Title-bar string
                                   WS_OVERLAPPEDWINDOW,                      		// Top-level window
                                   1000,                                            	// Width
								   700,			// Height
								   static_cast<UINT>(ceil(1000.f * dpiX / 96.f)),	// Width
								   static_cast<UINT>(ceil(700.f * dpiY / 96.f)),	// Height	
								   NULL,                // Parent
                                   NULL,//(HMENU)wc.lpszMenuName,                       // Class menu
                                   GetModuleHandle(NULL),                           	// Handle to application instance
                                   NULL                                             	// Window-creation data
                                   );	

    if (!m_hMainWindow)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

int CApplication::EnterMessageLoop()
{
    int result = 0;

    MSG msg = { 0 };

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    result = static_cast<int>(msg.wParam);

    return result;
}

//------------------------------------------------------
// In Action
//------------------------------------------------------

// Main window procedure
LRESULT CALLBACK CApplication::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (uMsg)
    {
        case WM_COMMAND:
            result = s_application->OnCommand(wParam);
            break;
		case WM_KEYDOWN:
			result = s_application->OnKeyDown(wParam);
			break;
        case WM_PAINT:
            result = s_application->OnPaint(hwnd);
            break;
		//case WM_ERASEBKGND:
		//	result = 1;
		//	break;
		case WM_CLOSE:
            result = s_application->OnClose(hwnd);
            break;
        case WM_DESTROY:
            result = s_application->OnDestroy(hwnd);
            break;
			
        default:
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return result;
}

LRESULT CApplication::OnKeyDown(int key)
{
	HRESULT hr = S_OK;

        MFP_MEDIAPLAYER_STATE state = MFP_MEDIAPLAYER_STATE_EMPTY;

        hr = s_pPlayer->GetState(&state);

        if (SUCCEEDED(hr))
        {
            if (state == MFP_MEDIAPLAYER_STATE_PAUSED || state == MFP_MEDIAPLAYER_STATE_STOPPED)
            {
                hr = s_pPlayer->Play();
            }
            else if (state == MFP_MEDIAPLAYER_STATE_PLAYING)
            {
                hr = s_pPlayer->Pause();
            }
        }

	return hr;
}

// Handles the WM_COMMAND message.
HRESULT CApplication::OnCommand(int id)
{
    HRESULT hr = S_OK;
    //IDCompositionAnimation* pAnimation = NULL;

    switch (id)
    {
        /*case ID_ROTATE:
            hr = s_application->OnRotate();
            break;

        case ID_SCALE:
            hr = s_application->OnScale();
            break;

        case ID_SKEW:
            hr = s_application->OnSkew();
            break;*/
        case IDM_ABOUT:
            DialogBox(m_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), m_hMainWindow, About);
            break;
		case ID_FILE_OPEN:
			 OnFileOpen(m_hMainWindow);
			 break;
        case ID_FILE_OPEN_URL:
            PlayMediaFile(m_hMainWindow, L"");
            break;
		case ID_FILE_EXIT:
			OnClose(m_hMainWindow);
			break;
        case ID_PLAYSTOP:
            // Play/Stop the video.
            if (!s_pPlayer)
            {
                PlayMediaFile(m_hMainWindow, pwszFilePath);
            }
            else
            {
                MFP_MEDIAPLAYER_STATE state = MFP_MEDIAPLAYER_STATE_EMPTY;

                hr = s_pPlayer->GetState(&state);

                if (SUCCEEDED(hr))
                {
                    if (state == MFP_MEDIAPLAYER_STATE_PAUSED || state == MFP_MEDIAPLAYER_STATE_STOPPED)
                    {
                        hr = s_pPlayer->Play();
                    }
                    else if (state == MFP_MEDIAPLAYER_STATE_PLAYING)
                    {
                        hr = s_pPlayer->Pause();
                    }
                }
            }
            break;
    }

    return hr;
}


// Handles the WM_PAINT message.
HRESULT CApplication::OnPaint(HWND hwnd)
{
    HDC hdc;
    PAINTSTRUCT ps;
    HRESULT hr = S_OK;

	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
			);

		// Create a Direct2D render target.
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, size),
			&m_pRenderTarget
			);
	}
    hdc = BeginPaint(hwnd, &ps);

        // Paint the video window.
    if (s_pPlayer && m_bHasVideo)
    {
		// Playback has started and there is video.
        // Do not draw the window background, because the video
        // frame fills the entire client area.
		s_pPlayer->UpdateVideo();
    }

    EndPaint(hwnd, &ps);
    return hr;
}

// Handles the WM_CLOSE message.
LRESULT CApplication::OnClose(HWND /*hwnd*/)
{
    // Close the MFPlay player object.
    if (s_pPlayer)
    {
        s_pPlayer->Shutdown();
        s_pPlayer = nullptr;
    }

    // Close the application callback object.
    if (s_pPlayerCB)
    {
        s_pPlayerCB = nullptr;
    }

    // Destroy the main window.
    DestroyWindow(m_hMainWindow);

    return 0;
}

// Handles the WM_DESTROY message.
LRESULT CApplication::OnDestroy(HWND /*hwnd*/)
{
    PostQuitMessage(0);

    return 0;
}

//------------------------------------------------------
// Destroy
//------------------------------------------------------

VOID CApplication::Destroy()
{
    DestroyMainWindow();
    CoUninitialize();
}

VOID CApplication::DestroyMainWindow()
{
    if (m_hMainWindow != NULL)
    {
       DestroyWindow(m_hMainWindow);
       m_hMainWindow = NULL;
    }
}

//-------------------------------------------------------------------
// OnFileOpen
//
// Handles the "File Open" command.
//-------------------------------------------------------------------

void CApplication::OnFileOpen(HWND hwnd)
{    
    HRESULT hr = S_OK;

    IFileOpenDialog *pFileOpen = NULL;
    IShellItem *pItem = NULL;

    

    // Create the FileOpenDialog object.
    hr = CoCreateInstance(
        __uuidof(FileOpenDialog), 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_PPV_ARGS(&pFileOpen)
        );

    if (FAILED(hr)) { goto done; }


    hr = pFileOpen->SetTitle(L"Select a File to Play");

    if (FAILED(hr)) { goto done; }


    // Show the file-open dialog.
    hr = pFileOpen->Show(hwnd);

    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
    {
        // User cancelled.
        hr = S_OK;
        goto done;
    }
    if (FAILED(hr)) { goto done; }


    // Get the file name from the dialog.
    hr = pFileOpen->GetResult(&pItem);

    if (FAILED(hr)) { goto done; }


    hr = pItem->GetDisplayName(SIGDN_URL, &pwszFilePath);

    if (FAILED(hr)) { goto done; }


    // Open the media file.
    hr = PlayMediaFile(hwnd, pwszFilePath);

    if (FAILED(hr)) { goto done; }

done:
    if (FAILED(hr))
    {
        ShowErrorMessage(L"Could not open file.", hr);
    }

    CoTaskMemFree(pwszFilePath);

    if (pItem)
    {
        pItem->Release();
    }
    if (pFileOpen)
    {
        pFileOpen->Release();
    }
}

// Message handler for about box.
INT_PTR CALLBACK CApplication::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Show error messages.
void CApplication::ShowErrorMessage(PCWSTR format, HRESULT hrErr)
{
    WCHAR msg[MAX_PATH];
    HRESULT hr = StringCbPrintf((STRSAFE_LPSTR)msg, sizeof(msg), (STRSAFE_LPCSTR)(L"%s (hr=0x%08X)"), format, hrErr);

    if (SUCCEEDED(hr))
    {
        MessageBox(NULL, (LPCSTR)msg, NULL, MB_ICONERROR);
    }
}

// Plays a media file, using the IMFPMediaPlayer interface.
HRESULT CApplication::PlayMediaFile(HWND hwnd, const WCHAR *sURL)
{
    HRESULT hr = S_OK;

    // Create the MFPlayer object.
    if (s_pPlayer == nullptr)
    {
        s_pPlayerCB = new MediaPlayerCallback();

        if (s_pPlayerCB == nullptr)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = MFPCreateMediaPlayer(NULL,
                                  FALSE,                // Start playback automatically?
                                  MFP_OPTION_NONE,      // Flags
                                  s_pPlayerCB,          // Callback pointer
                                  hwnd,  // Video window
                                  &s_pPlayer
                                  );
    }

    // Create a new media item for this URL.
    if (SUCCEEDED(hr))
    {
        hr = s_pPlayer->CreateMediaItemFromURL(sURL, FALSE, 0, NULL);//L"media/Star.Wars.The.Last.Jedi.2017.FRENCH.BDRip.XviD-FuN.avi", FALSE, 0, NULL);
    }

    // The CreateMediaItemFromURL method completes asynchronously.
    // The application will receive an MFP_EVENT_TYPE_MEDIAITEM_CREATED
    // event. See MediaPlayerCallback::OnMediaPlayerEvent().
    return hr;
}

// Called when the IMFPMediaPlayer::CreateMediaItemFromURL method
// completes.
void CApplication::OnMediaItemCreated(MFP_MEDIAITEM_CREATED_EVENT *pEvent)
{
    HRESULT hr = S_OK;

    // The media item was created successfully.
    if (s_pPlayer)
    {
        BOOL bHasVideo = FALSE, bIsSelected = FALSE;

        // Check if the media item contains video.
        hr = pEvent->pMediaItem->HasVideo(&bHasVideo, &bIsSelected);

        if (FAILED(hr))
        {
            ShowErrorMessage(L"Error playing this file.", hr);
        }

        m_bHasVideo = bHasVideo && bIsSelected;

        // Set the media item on the player. This method completes asynchronously.
        s_pPlayer->SetMediaItem(pEvent->pMediaItem);
    }
}

// Called when the IMFPMediaPlayer::SetMediaItem method completes.
void CApplication::OnMediaItemSet(MFP_MEDIAITEM_SET_EVENT * /*pEvent*/)
{
    HRESULT hr = s_pPlayer->Play();

    if (FAILED(hr))
    {
        ShowErrorMessage(L"IMFPMediaPlayer::Play failed.", hr);
    }
}

// Implements IMFPMediaPlayerCallback::OnMediaPlayerEvent.
// This callback method handles events from the MFPlay object.
void MediaPlayerCallback::OnMediaPlayerEvent(_In_ MFP_EVENT_HEADER * pEventHeader)
{
    if (FAILED(pEventHeader->hrEvent))
    {
        CApplication::ShowErrorMessage(L"Playback error", pEventHeader->hrEvent);
        return;
    }

    switch (pEventHeader->eEventType)
    {
    case MFP_EVENT_TYPE_MEDIAITEM_CREATED:
        CApplication::OnMediaItemCreated(MFP_GET_MEDIAITEM_CREATED_EVENT(pEventHeader));
        break;

    case MFP_EVENT_TYPE_MEDIAITEM_SET:
        CApplication::OnMediaItemSet(MFP_GET_MEDIAITEM_SET_EVENT(pEventHeader));
        break;
    }
}