
#include <Windows.h>
#include <CommCtrl.h>


#include "main_window.h"
#include "file_operation.h"
#include "file_system_utility.h"
#include "media_setting_dialogue.h"
#include "Resource.h"

#pragma comment(lib, "Comctl32.lib")

CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_SWM));
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_ICON_SWM);
    wcex.lpszClassName = m_class_name.c_str();
    wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_SWM));

    if (::RegisterClassExW(&wcex))
    {
        m_hInstance = hInstance;

        m_hWnd = ::CreateWindowW(m_class_name.c_str(), m_window_name.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, nullptr, nullptr, hInstance, this);
        if (m_hWnd != nullptr)
        {
            return true;
        }
        else
        {
            std::wstring wstrMessage = L"CreateWindowExW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
        }
    }
    else
    {
        std::wstring wstrMessage = L"RegisterClassW failed; code: " + std::to_wstring(::GetLastError());
        ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    }

	return false;
}

int CMainWindow::MessageLoop()
{
    MSG msg;

    for (;;)
    {
        BOOL bRet = ::GetMessageW(&msg, 0, 0, 0);
        if (bRet > 0)
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        else if (bRet == 0)
        {
            /*ループ終了*/
            return static_cast<int>(msg.wParam);
        }
        else
        {
            /*ループ異常*/
            std::wstring wstrMessage = L"GetMessageW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
            return -1;
        }
    }
    return 0;
}
/*C CALLBACK*/
LRESULT CMainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMainWindow* pThis = nullptr;
    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = reinterpret_cast<CMainWindow*>(pCreateStruct->lpCreateParams);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }

    pThis = reinterpret_cast<CMainWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (pThis != nullptr)
    {
        return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CMainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return OnCreate(hWnd);
    case WM_DESTROY:
        return OnDestroy();
    case WM_CLOSE:
        return OnClose();
    case WM_PAINT:
        return OnPaint();
    case WM_ERASEBKGND:
        return 0;
    case WM_COMMAND:
        return OnCommand(wParam);
    case WM_MOUSEWHEEL:
        return OnMouseWheel(wParam, lParam);
    case WM_LBUTTONDOWN:
        return OnLButtonDown(wParam, lParam);
    case WM_LBUTTONUP:
        return OnLButtonUp(wParam, lParam);
    case WM_MBUTTONUP:
        return OnMButtonUp(wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
    m_hWnd = hWnd;

    InitialiseMenuBar();

    m_pScenePlayer = new CScenePlayer(m_hWnd);

    m_pMediaPlayer = new CMediaPlayer(m_hWnd);

    return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
    ::PostQuitMessage(0);

    if (m_pScenePlayer != nullptr)
    {
        delete m_pScenePlayer;
        m_pScenePlayer = nullptr;
    }

    if (m_pMediaPlayer != nullptr)
    {
        delete m_pMediaPlayer;
        m_pMediaPlayer = nullptr;
    }
    return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
    ::DestroyWindow(m_hWnd);
    ::UnregisterClassW(m_class_name.c_str(), m_hInstance);

    return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(m_hWnd, &ps);

    if (m_pScenePlayer != nullptr)
    {
        m_pScenePlayer->DisplayImage();
    }

    ::EndPaint(m_hWnd, &ps);

    return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{
    return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam)
{
    int wmKind = HIWORD(wParam);
    int wmId = LOWORD(wParam);
    if (wmKind == 0)
    {
        /*Menus*/
        switch (wmId)
        {
        case Menu::kOpen:
            MenuOnOpen();
            break;
        case Menu::kNextFolder:
            MenuOnNextFolder();
            break;
        case Menu::kForeFolder:
            MenuOnForeFolder();
            break;
        case Menu::kNextAudio:
            MenuOnNextAudio();
            break;
        case Menu::kBack:
            MenuOnBack();
            break;
        case Menu::kPlay:
            MenuOnPlay();
            break;
        case Menu::kLoop:
            MenuOnLoop();
            break;
        case Menu::kVolume:
            MenuOnVolume();
            break;
        case Menu::kPauseImage:
            MenuOnPauseImage();
            break;
        }
    }
    if (wmKind > 1)
    {
        /*Controls*/
    }

    return 0;
}
/*WM_MOUSEWHEEL*/
LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    int iScroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
    WORD wKey = LOWORD(wParam);

    if (wKey == 0 && m_pScenePlayer != nullptr)
    {
        if (iScroll > 0)
        {
            m_pScenePlayer->UpScale();
        }
        else
        {
            m_pScenePlayer->DownScale();
        }
    }

    if (wKey == MK_RBUTTON && m_pMediaPlayer != nullptr)
    {
        if (iScroll > 0)
        {
            m_pMediaPlayer->Next();
        }
        else
        {
            m_pMediaPlayer->Back();
        }
    }

    if (wKey == MK_LBUTTON && m_pScenePlayer != nullptr)
    {
        if (iScroll > 0)
        {
            m_pScenePlayer->SpeedUp();
        }
        else
        {
            m_pScenePlayer->SpeedDown();
        }
        m_bSpeedSet = true;
    }

    return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    ::GetCursorPos(&m_CursorPos);

    return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (m_bSpeedSet == true)
    {
        m_bSpeedSet = false;
        return 0;
    }

    WORD usKey = LOWORD(wParam);

    if (usKey == MK_RBUTTON && m_bHideBar)
    {
        ::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_DOWN;
        ::SendInput(1, &input, sizeof(input));
    }

    if (usKey == 0 && m_pScenePlayer != nullptr)
    {
        POINT pt{};
        ::GetCursorPos(&pt);
        int iX = m_CursorPos.x - pt.x;
        int iY = m_CursorPos.y - pt.y;

        if (iX == 0 && iY == 0)
        {
            m_pScenePlayer->Next();
        }
        else
        {
            m_pScenePlayer->SetOffset(iX, iY);
        }

    }
    return 0;
}
/*WM_MBUTTONUP*/
LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
    WORD usKey = LOWORD(wParam);
    if (usKey == 0 && m_pScenePlayer != nullptr)
    {
        m_pScenePlayer->ResetScale();
    }

    if (usKey == MK_RBUTTON && m_pScenePlayer != nullptr)
    {
        SwitchWindowMode();
    }

    return 0;
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
    HMENU hManuFolder = nullptr;
    HMENU hMenuAudio = nullptr;
    HMENU kMenuImage = nullptr;
    HMENU hMenuBar = nullptr;
    BOOL iRet = FALSE;

    if (m_hMenuBar != nullptr)return;

    hManuFolder = ::CreateMenu();
    if (hManuFolder == nullptr)goto failed;

    iRet = ::AppendMenuA(hManuFolder, MF_STRING, Menu::kOpen, "Open");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hManuFolder, MF_STRING, Menu::kNextFolder, "Next");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hManuFolder, MF_STRING, Menu::kForeFolder, "Back");
    if (iRet == 0)goto failed;

    hMenuAudio = ::CreateMenu();
    if (hMenuAudio == nullptr)goto failed;

    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kNextAudio, "Next");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kBack, "Back");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kPlay, "Play");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kLoop, "Loop");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kVolume, "Setting");
    if (iRet == 0)goto failed;

    kMenuImage = ::CreateMenu();
    iRet = ::AppendMenuA(kMenuImage, MF_STRING, Menu::kPauseImage, "Pause");
    if (iRet == 0)goto failed;

    hMenuBar = ::CreateMenu();
    if (hMenuBar == nullptr) goto failed;

    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hManuFolder), "Folder");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuAudio), "Audio");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(kMenuImage), "Image");
    if (iRet == 0)goto failed;

    iRet = ::SetMenu(m_hWnd, hMenuBar);
    if (iRet == 0)goto failed;

    m_hMenuBar = hMenuBar;

    /*正常終了*/
    return;

failed:
    std::wstring wstrMessage = L"Failed to create menu; code: " + std::to_wstring(::GetLastError());
    ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    /*SetMenu成功後はウィンドウ破棄時に破棄されるが、今は紐づけ前なのでここで破棄する。*/
    if (hManuFolder != nullptr)
    {
        ::DestroyMenu(hManuFolder);
    }
    if (hMenuAudio != nullptr)
    {
        ::DestroyMenu(hMenuAudio);
    }
    if (kMenuImage != nullptr)
    {
        ::DestroyMenu(kMenuImage);
    }
    if (hMenuBar != nullptr)
    {
        ::DestroyMenu(hMenuBar);
    }

}
/*フォルダ選択*/
void CMainWindow::MenuOnOpen()
{
    wchar_t* buffer = SelectWorkingFolder(m_hWnd);
    if (buffer != nullptr)
    {
        SetPlayerFolder(buffer);
        CreateFolderList(buffer);

        ::CoTaskMemFree(buffer);
    }
}
/*次フォルダに移動*/
void CMainWindow::MenuOnNextFolder()
{
    if (m_folders.empty())return;

    ++m_nIndex;
    if (m_nIndex >= m_folders.size())m_nIndex = 0;
    SetPlayerFolder(m_folders.at(m_nIndex).c_str());
}
/*前フォルダに移動*/
void CMainWindow::MenuOnForeFolder()
{
    if (m_folders.empty())return;

    --m_nIndex;
    if (m_nIndex >= m_folders.size())m_nIndex = m_folders.size() - 1;
    SetPlayerFolder(m_folders.at(m_nIndex).c_str());
}
/*次音声再生*/
void CMainWindow::MenuOnNextAudio()
{
    if (m_pMediaPlayer != nullptr)
    {
        m_pMediaPlayer->Next();
    }
}
/*前音声再生*/
void CMainWindow::MenuOnBack()
{
    if (m_pMediaPlayer != nullptr)
    {
        m_pMediaPlayer->Back();
    }
}
/*現行音声再再生*/
void CMainWindow::MenuOnPlay()
{
    if (m_pMediaPlayer != nullptr)
    {
        m_pMediaPlayer->Play();
    }
}
/*音声ループ設定変更*/
void CMainWindow::MenuOnLoop()
{
    if (m_pMediaPlayer != nullptr)
    {
        HMENU hMenuBar = ::GetMenu(m_hWnd);
        if (hMenuBar != nullptr)
        {
            HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kAudio);
            if (hMenu != nullptr)
            {
                BOOL iRet = m_pMediaPlayer->SwitchLoop();
                ::CheckMenuItem(hMenu, Menu::kLoop, iRet == TRUE ? MF_CHECKED : MF_UNCHECKED);
            }
        }
    }
}
/*音量・再生速度変更*/
void CMainWindow::MenuOnVolume()
{
    CMediaSettingDialogue* pMediaSettingDialogue = new CMediaSettingDialogue();
    if (pMediaSettingDialogue != nullptr)
    {
        pMediaSettingDialogue->Open(m_hInstance, m_hWnd, m_pMediaPlayer, L"Audio Setting");

        delete pMediaSettingDialogue;
    }
}
/*一時停止*/
void CMainWindow::MenuOnPauseImage()
{
    if (m_pScenePlayer != nullptr)
    {
        HMENU hMenuBar = ::GetMenu(m_hWnd);
        if (hMenuBar != nullptr)
        {
            HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kImage);
            if (hMenu != nullptr)
            {
                bool bRet = m_pScenePlayer->SwitchPause();
                ::CheckMenuItem(hMenu, Menu::kPauseImage, bRet ? MF_CHECKED : MF_UNCHECKED);
            }
        }
    }
}
/*標題変更*/
void CMainWindow::ChangeWindowTitle(const wchar_t* pzTitle)
{
    std::wstring wstr;
    if (pzTitle != nullptr)
    {
        std::wstring wstrTitle = pzTitle;
        size_t pos = wstrTitle.find_last_of(L"\\/");
        wstr = pos == std::wstring::npos ? wstrTitle : wstrTitle.substr(pos + 1);
    }

    ::SetWindowTextW(m_hWnd, wstr.empty() ? m_window_name.c_str() : wstr.c_str());
}
/*表示形式変更*/
void CMainWindow::SwitchWindowMode()
{
    if (!m_bPlayReady)return;

    RECT rect;
    ::GetWindowRect(m_hWnd, &rect);
    LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

    m_bHideBar ^= true;

    if (m_bHideBar)
    {
        ::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU);
        ::SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
        ::SetMenu(m_hWnd, nullptr);
    }
    else
    {
        ::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU);
        ::SetMenu(m_hWnd, m_hMenuBar);
    }

    if (m_pScenePlayer != nullptr)
    {
        m_pScenePlayer->SwitchSizeLore(m_bHideBar);
    }

}
/*フォルダ一覧表作成*/
bool CMainWindow::CreateFolderList(const wchar_t* pwzFolderPath)
{
    if (pwzFolderPath == nullptr)return false;

    m_folders.clear();
    m_nIndex = 0;

    std::wstring wstrFolder = pwzFolderPath;
    std::wstring wstrParent;
    std::wstring wstrCurrent;

    size_t nPos = wstrFolder.find_last_of(L"\\/");
    if (nPos != std::wstring::npos)
    {
        wstrParent = wstrFolder.substr(0, nPos);
        wstrCurrent = wstrFolder.substr(nPos + 1);
    }

    if (wstrParent.empty())return false;

    CreateFilePathList(wstrParent.c_str(), nullptr, m_folders);

    auto iter = std::find(m_folders.begin(), m_folders.end(), wstrFolder);
    if (iter != m_folders.end())
    {
        m_nIndex = std::distance(m_folders.begin(), iter);
    }

    return m_folders.size() > 0;
}
/*再生フォルダ設定*/
void CMainWindow::SetPlayerFolder(const wchar_t* pwzFolderPath)
{
    if (pwzFolderPath == nullptr)return;

    std::wstring wstrFolder = pwzFolderPath;
    std::wstring wstrParent;
    std::wstring wstrCurrent;

    size_t nPos = wstrFolder.find_last_of(L"\\/");
    if (nPos != std::wstring::npos)
    {
        wstrParent = wstrFolder.substr(0, nPos);
        wstrCurrent = wstrFolder.substr(nPos + 1);
    }

    if (wstrParent.empty())return;

    /*<algorithm>は使わない*/
    std::wstring wstrImageFolderPath;
    std::wstring wstrAudioFolderPath;
    nPos = wstrCurrent.find_first_of(L"0123456789");
    if (nPos != std::wstring::npos)
    {
        if (wcsstr(wstrCurrent.c_str(), L"Stillstill") != nullptr)
        {
            wstrImageFolderPath = wstrFolder;
            wstrAudioFolderPath = wstrParent + L"\\VoiceStoryCardcard" + wstrCurrent.substr(nPos);
        }
        else
        {
            wstrAudioFolderPath = wstrFolder;
            wstrImageFolderPath = wstrParent + L"\\Stillstill" + wstrCurrent.substr(nPos);
        }
    }

    SetPlayFiles(wstrImageFolderPath.c_str(), wstrAudioFolderPath.c_str());

    ChangeWindowTitle(m_bPlayReady ? pwzFolderPath : nullptr);
}
/*再生ファイル群設定*/
void CMainWindow::SetPlayFiles(const wchar_t* pwzImageFolderPath, const wchar_t* pwzAudioFolderPath)
{
    /*音声なし・画像なしの脚本あり*/
    if (pwzImageFolderPath == nullptr)return;

    bool bRet = false;

    std::vector<std::wstring> imageFiles;
    bRet = CreateFilePathList(pwzImageFolderPath, L".jpg", imageFiles);
    if (bRet && m_pScenePlayer != nullptr)
    {
        m_bPlayReady = m_pScenePlayer->SetFiles(imageFiles);
    }

    std::vector<std::wstring> audioFiles;
    bRet = CreateFilePathList(pwzAudioFolderPath, L".mp3", audioFiles);
    if (bRet && m_pMediaPlayer != nullptr)
    {
        m_pMediaPlayer->SetFiles(audioFiles);
    }

    ChangeWindowTitle(m_bPlayReady ? pwzImageFolderPath : nullptr);
}
