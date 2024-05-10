#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>

#include "scene_player.h"
#include "mf_media_player.h"
#include "adv.h"
#include "d2_text_writer.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();
	bool Create(HINSTANCE hInstance);
	int MessageLoop();
	HWND GetHwnd()const { return m_hWnd;}
private:
	const std::wstring m_class_name = L"SweetHomeMaid player window";
	std::wstring m_window_name = L"SweetHomeMaid player";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	enum Menu{
		kOpen = 1, kNextFolder, kForeFolder,
		kLoop, kVolume,
		kPauseImage,
	};
	enum MenuBar
	{
		kFolder, kAudio, kImage
	};
	enum EventMessage
	{
		kAudioPlayer = WM_USER + 1
	};
	enum Timer
	{
		kText = 1,
	};
	POINT m_CursorPos{};
	bool m_bSpeedHavingChanged = false;

	HMENU m_hMenuBar = nullptr;
	bool m_bBarHidden = false;
	bool m_bPlayReady = false;
	bool m_bTextHidden = false;

	std::vector<std::wstring> m_folders;
	size_t m_nFolderIndex = 0;

	void InitialiseMenuBar();

	void MenuOnOpen();
	void MenuOnNextFolder();
	void MenuOnForeFolder();

	void MenuOnLoop();
	void MenuOnVolume();

	void MenuOnPauseImage();

	void ChangeWindowTitle(const wchar_t* pzTitle);
	void SwitchWindowMode();

	bool CreateFolderList(const wchar_t* pwzFolderPath);
	void SetPlayerFolder(const wchar_t* pwzFolderPath);
	void CreateMessgaeList(const wchar_t* pwzCardId, const wchar_t* pwzAudioFolderPath);
	void SetImages(const wchar_t* pwzImageFolderPath);

	void UpdateScreen();

	CScenePlayer* m_pScenePlayer = nullptr;

	CMfMediaPlayer* m_pMediaPlayer = nullptr;
	CD2TextWriter* m_pD2TextWriter = nullptr;
	std::wstring m_textFontFilePath = L"C:\\Windows\\Fonts\\yumindb.ttf";

	std::vector<adv::TextDatum> m_textData;
	size_t m_nTextIndex = 0;

	void ShiftText(bool bForward);
	void UpdateText();
	void OnAudioPlayerEvent(unsigned long ulEvent);
	void AutoTexting();
};

#endif //MAIN_WINDOW_H_