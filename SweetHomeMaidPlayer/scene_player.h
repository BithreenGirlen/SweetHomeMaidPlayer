#ifndef SCENE_PLAYER_H_
#define SCENE_PLAYER_H_

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <atlbase.h>

#include <string>
#include <vector>

class CScenePlayer
{
public:
    CScenePlayer(HWND hWnd);
    ~CScenePlayer();

    bool SetFiles(const std::vector<std::wstring>& filePaths);
    bool DrawImage();
    void Display();

    void Next();
    void UpScale();
    void DownScale();
    void ResetScale();
    void SwitchSizeLore(bool bBarHidden);
    void SetOffset(int iX, int iY);
    void SpeedUp();
    void SpeedDown();
    bool SwitchPause();

    ID2D1Factory1* GetD2Factory()const { return m_pD2d1Factory1; }
    ID2D1DeviceContext* GetD2DeviceContext()const { return m_pD2d1DeviceContext; }
private:

    HWND m_hRetWnd = nullptr;

    enum Portion { kInterval = 32};
    enum Division {kFirstAnimation = 1, kSecondAnimation = 41};

    HRESULT m_hrComInit = E_FAIL;
    ID2D1Factory1* m_pD2d1Factory1 = nullptr;
    ID2D1DeviceContext* m_pD2d1DeviceContext = nullptr;
    IDXGISwapChain1* m_pDxgiSwapChain1 = nullptr;

    size_t m_nIndex = 0;
    std::vector<CComPtr<ID2D1Bitmap>> m_images;

    double m_dbScale = 1.0;
    int m_iXOffset = 0;
    int m_iYOffset = 0;
    long long m_interval = Portion::kInterval;

    bool m_bBarHidden = false;
    bool m_bPause = false;

    void Clear();
    bool LoadImageToMemory(const wchar_t* pwzFilePath);
    void Update();
    void ResizeWindow();
    void AdjustOffset(int iXAddOffset, int iYAddOffset);
    void ResizeBuffer();

    void StartThreadpoolTimer();
    void EndThreadpoolTimer();

    PTP_TIMER m_timer = nullptr;
    static void CALLBACK TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer);
};

#endif //SCENE_PLAYER_H_
