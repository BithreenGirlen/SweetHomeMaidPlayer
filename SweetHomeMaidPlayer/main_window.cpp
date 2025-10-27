
#include <Windows.h>
#include <CommCtrl.h>


#include "main_window.h"
#include "win_dialogue.h"
#include "win_filesystem.h"
#include "media_setting_dialogue.h"
#include "Resource.h"
#include "swmd.h"

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
	wcex.lpszClassName = m_class_name;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_SWM));

	if (::RegisterClassExW(&wcex))
	{
		m_hInstance = hInstance;

		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);
		int iWindowHeight = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);

		m_hWnd = ::CreateWindowW(m_class_name, m_window_name, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			CW_USEDEFAULT, CW_USEDEFAULT, iWindowWidth, iWindowHeight, nullptr, nullptr, hInstance, this);
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
		return 1;
	case WM_KEYUP:
		return OnKeyUp(wParam, lParam);
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	case WM_TIMER:
		return OnTimer(wParam);
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam, lParam);
	case WM_MBUTTONUP:
		return OnMButtonUp(wParam, lParam);
	case EventMessage::kAudioPlayer:
		OnAudioPlayerEvent(static_cast<unsigned long>(lParam));
		break;
	default:
		break;
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
	m_hWnd = hWnd;

	InitialiseMenuBar();

	m_pScenePlayer = new CScenePlayer(m_hWnd);

	m_pMediaPlayer = new CMfMediaPlayer(m_hWnd, EventMessage::kAudioPlayer);

	m_pD2TextWriter = new CD2TextWriter(m_pScenePlayer->GetD2Factory(), m_pScenePlayer->GetD2DeviceContext());
	m_pD2TextWriter->SetupOutLinedDrawing(L"C:\\Windows\\Fonts\\yumindb.ttf");

	return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
	::PostQuitMessage(0);

	if (m_pD2TextWriter != nullptr)
	{
		delete m_pD2TextWriter;
		m_pD2TextWriter = nullptr;
	}

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
	::UnregisterClassW(m_class_name, m_hInstance);

	return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	if (m_pScenePlayer != nullptr)
	{
		bool bRet = m_pScenePlayer->DrawImage();
		if (bRet)
		{
			if (m_nTextIndex < m_textData.size() && !m_bTextHidden && m_pD2TextWriter != nullptr)
			{
				const adv::TextDatum& t = m_textData[m_nTextIndex];
				std::wstring wstr = t.wstrText;
				if (!wstr.empty() && wstr.back() != L'\n') wstr += L"\n ";
				wstr += std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
				m_pD2TextWriter->OutLinedDraw(wstr.c_str(), static_cast<unsigned long>(wstr.size()));
			}
			m_pScenePlayer->Display();
		}
	}

	::EndPaint(m_hWnd, &ps);

	return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{
	return 0;
}
LRESULT CMainWindow::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case 0x54:// T key
		m_bTextHidden ^= true;
		UpdateScreen();
		break;
	}
	return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int iControlWnd = LOWORD(lParam);
	if (iControlWnd == 0)
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
	else
	{
		/*Controls*/
	}

	return 0;
}
/*WM_TIMER*/
LRESULT CMainWindow::OnTimer(WPARAM wParam)
{
	switch (wParam)
	{
	case Timer::kText:
		AutoTexting();
		break;
	default:
		break;
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
		ShiftText(iScroll > 0);
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
		m_bSpeedHavingChanged = true;
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
	if (m_bSpeedHavingChanged == true)
	{
		m_bSpeedHavingChanged = false;
		return 0;
	}

	WORD usKey = LOWORD(wParam);

	if (usKey == MK_RBUTTON && m_bBarHidden)
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
	HMENU hMenuFolder = nullptr;
	HMENU hMenuAudio = nullptr;
	HMENU hMenuImage = nullptr;
	HMENU hMenuBar = nullptr;
	BOOL iRet = FALSE;

	if (m_hMenuBar != nullptr)return;

	hMenuFolder = ::CreateMenu();
	if (hMenuFolder == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuFolder, MF_STRING, Menu::kOpen, "Open");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuFolder, MF_STRING, Menu::kNextFolder, "Next");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuFolder, MF_STRING, Menu::kForeFolder, "Back");
	if (iRet == 0)goto failed;

	hMenuAudio = ::CreateMenu();
	if (hMenuAudio == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kLoop, "Loop");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kVolume, "Setting");
	if (iRet == 0)goto failed;

	hMenuImage = ::CreateMenu();
	iRet = ::AppendMenuA(hMenuImage, MF_STRING, Menu::kPauseImage, "Pause");
	if (iRet == 0)goto failed;

	hMenuBar = ::CreateMenu();
	if (hMenuBar == nullptr) goto failed;

	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFolder), "Folder");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuAudio), "Audio");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuImage), "Image");
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
	if (hMenuFolder != nullptr)
	{
		::DestroyMenu(hMenuFolder);
	}
	if (hMenuAudio != nullptr)
	{
		::DestroyMenu(hMenuAudio);
	}
	if (hMenuImage != nullptr)
	{
		::DestroyMenu(hMenuImage);
	}
	if (hMenuBar != nullptr)
	{
		::DestroyMenu(hMenuBar);
	}

}
/*フォルダ選択*/
void CMainWindow::MenuOnOpen()
{
	std::wstring wstrFolder = win_dialogue::SelectWorkFolder(m_hWnd);
	if (!wstrFolder.empty())
	{
		CreateFolderList(wstrFolder.c_str());
		SetPlayerFolder(wstrFolder);
	}
}
/*次フォルダに移動*/
void CMainWindow::MenuOnNextFolder()
{
	if (m_folders.empty())return;

	++m_nFolderIndex;
	if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = 0;
	SetPlayerFolder(m_folders[m_nFolderIndex]);
}
/*前フォルダに移動*/
void CMainWindow::MenuOnForeFolder()
{
	if (m_folders.empty())return;

	--m_nFolderIndex;
	if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = m_folders.size() - 1;
	SetPlayerFolder(m_folders[m_nFolderIndex]);
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

	::SetWindowTextW(m_hWnd, wstr.empty() ? m_window_name : wstr.c_str());
}
/*表示形式変更*/
void CMainWindow::SwitchWindowMode()
{
	if (!m_bPlayReady)return;

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

	m_bBarHidden ^= true;

	if (m_bBarHidden)
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
		m_pScenePlayer->SwitchSizeLore(m_bBarHidden);
	}

}
/*フォルダ一覧表作成*/
bool CMainWindow::CreateFolderList(const wchar_t* pwzFolderPath)
{
	m_folders.clear();
	m_nFolderIndex = 0;
	win_filesystem::GetFolderListAndIndex(pwzFolderPath, m_folders, &m_nFolderIndex);

	return m_folders.size() > 0;

}
/*再生フォルダ設定*/
void CMainWindow::SetPlayerFolder(const std::wstring& wstrFolderPath)
{
	std::wstring wstrParent;
	std::wstring wstrCurrent;

	size_t nPos = wstrFolderPath.find_last_of(L"\\/");
	if (nPos != std::wstring::npos)
	{
		wstrParent = wstrFolderPath.substr(0, nPos);
		wstrCurrent = wstrFolderPath.substr(nPos + 1);
	}

	if (wstrParent.empty())return;

	std::wstring wstrImageFolderPath;
	std::wstring wstrAudioFolderPath;
	nPos = wstrCurrent.find_first_of(L"0123456789");
	if (nPos != std::wstring::npos)
	{
		std::wstring wstrCardId = wstrCurrent.substr(nPos);

		constexpr const wchar_t swzKey[] = L"VoiceStoryCardcard";
		constexpr size_t nKeyLen = sizeof(swzKey) / sizeof(wchar_t) - 1;

		const auto ExtractAudioDirectory = [](const std::wstring& wstrAudioFolderPath)
			-> std::wstring
			{
				size_t nPos = wstrAudioFolderPath.find(swzKey);
				if (nPos == std::wstring::npos) return {};

				return wstrAudioFolderPath.substr(0, nPos + nKeyLen);
			};

		if (wstrCurrent.find(L"Stillstill") != std::wstring::npos)
		{
			std::wstring wstrKey = swzKey + wstrCardId;
			const auto& iter = std::find_if(m_folders.begin(), m_folders.end(),
				[&wstrKey](const std::wstring& wstr)
				{
					return wstr.find(wstrKey) != std::wstring::npos;
				});
			if (iter != m_folders.cend())
			{
				wstrImageFolderPath = wstrFolderPath;
				size_t nIndex = std::distance(m_folders.begin(), iter);
				wstrAudioFolderPath = ExtractAudioDirectory(m_folders[nIndex]);
			}
		}
		else
		{
			if (wstrCardId.size() > 6)wstrCardId.pop_back();

			std::wstring wstrKey = L"Stillstill" + wstrCardId;
			const auto& iter = std::find_if(m_folders.begin(), m_folders.end(),
				[&wstrKey](const std::wstring& wstr)
				{
					return wstr.find(wstrKey) != std::wstring::npos;
				});
			if (iter != m_folders.cend())
			{
				wstrAudioFolderPath = ExtractAudioDirectory(wstrFolderPath);
				size_t nIndex = std::distance(m_folders.begin(), iter);
				wstrImageFolderPath = m_folders[nIndex];
			}
		}

		CreateMessgaeList(wstrCardId.c_str(), wstrAudioFolderPath.c_str());
	}

	SetImages(wstrImageFolderPath.c_str());

	ChangeWindowTitle(m_bPlayReady ? wstrFolderPath.c_str() : nullptr);
}
/*文章一覧作成*/
void CMainWindow::CreateMessgaeList(const std::wstring& wstrCardId, const std::wstring& wstrAudioFolderPath)
{
	m_textData.clear();
	m_nTextIndex = 0;
	swmd::LoadScenario(wstrAudioFolderPath, wstrCardId, m_textData);

	UpdateText();
}
/*再生画像群設定*/
void CMainWindow::SetImages(const wchar_t* pwzImageFolderPath)
{
	std::vector<std::wstring> imageFiles;
	bool bRet = win_filesystem::CreateFilePathList(pwzImageFolderPath, L".jpg", imageFiles);
	if (bRet && m_pScenePlayer != nullptr)
	{
		m_bPlayReady = m_pScenePlayer->SetFiles(imageFiles);
	}

	ChangeWindowTitle(m_bPlayReady ? pwzImageFolderPath : nullptr);
}
/*再描画要求*/
void CMainWindow::UpdateScreen()
{
	::InvalidateRect(m_hWnd, nullptr, FALSE);
}
/*文章送り・戻し*/
void CMainWindow::ShiftText(bool bForward)
{
	if (m_textData.empty())return;

	if (bForward)
	{
		++m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
	}
	else
	{
		--m_nTextIndex;
		if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
	}
	UpdateText();
}
/*文章更新*/
void CMainWindow::UpdateText()
{
	if (m_nTextIndex < m_textData.size())
	{
		const adv::TextDatum& t = m_textData[m_nTextIndex];
		if (!t.wstrVoicePath.empty())
		{
			if (m_pMediaPlayer != nullptr)
			{
				m_pMediaPlayer->Play(t.wstrVoicePath.c_str());
			}

			::KillTimer(m_hWnd, Timer::kText);
		}
		else
		{
			constexpr unsigned int kTimerInterval = 2000;
			::SetTimer(m_hWnd, Timer::kText, kTimerInterval, nullptr);
		}
	}

	::InvalidateRect(m_hWnd, nullptr, FALSE);
}
/*IMFMediaEngineNotify::EventNotify*/
void CMainWindow::OnAudioPlayerEvent(unsigned long ulEvent)
{
	switch (ulEvent)
	{
	case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:

		break;
	case MF_MEDIA_ENGINE_EVENT_ENDED:
		AutoTexting();
		break;
	default:
		break;
	}
}
/*自動送り*/
void CMainWindow::AutoTexting()
{
	if (m_nTextIndex < m_textData.size() - 1)ShiftText(true);
}
