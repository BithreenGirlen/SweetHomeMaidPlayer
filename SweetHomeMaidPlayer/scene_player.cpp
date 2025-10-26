

#include "scene_player.h"

#include <math.h>
#include <shobjidl.h>
#include <wincodec.h>

#pragma comment (lib,"Windowscodecs.lib")
#pragma comment (lib,"D2d1.lib")
#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"dxguid.lib")

CScenePlayer::CScenePlayer(HWND hWnd)
	:m_hRetWnd(hWnd)
{
	m_hrComInit = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(m_hrComInit))return;

	CComPtr<ID3D11Device>pD3d11Device;
	HRESULT hr = ::D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_SINGLETHREADED, nullptr, 0, D3D11_SDK_VERSION,
		&pD3d11Device, nullptr, nullptr);
	if (FAILED(hr))return;

	CComPtr<IDXGIDevice1> pDxgDevice1;
	hr = pD3d11Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pDxgDevice1);
	if (FAILED(hr))return;

	hr = pDxgDevice1->SetMaximumFrameLatency(1);
	if (FAILED(hr))return;

	CComPtr<IDXGIAdapter> pDxgiAdapter;
	hr = pDxgDevice1->GetAdapter(&pDxgiAdapter);
	if (FAILED(hr))return;

	CComPtr<IDXGIFactory2> pDxgiFactory2;
	hr = pDxgiAdapter->GetParent(IID_PPV_ARGS(&pDxgiFactory2));
	if (FAILED(hr))return;

	hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2d1Factory1);
	if (FAILED(hr))return;

	CComPtr<ID2D1Device> pD2d1Device;
	hr = m_pD2d1Factory1->CreateDevice(pDxgDevice1, &pD2d1Device);
	if (FAILED(hr))return;

	hr = pD2d1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2d1DeviceContext);
	if (FAILED(hr))return;

	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 1;
	desc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

	hr = pDxgiFactory2->CreateSwapChainForHwnd(pDxgDevice1, hWnd, &desc, nullptr, nullptr, &m_pDxgiSwapChain1);
	if (FAILED(hr))return;

	m_pD2d1DeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	m_pD2d1DeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
	m_pD2d1DeviceContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);
	D2D1_RENDERING_CONTROLS sRenderings{};
	m_pD2d1DeviceContext->GetRenderingControls(&sRenderings);
	sRenderings.bufferPrecision = D2D1_BUFFER_PRECISION_8BPC_UNORM_SRGB;
	m_pD2d1DeviceContext->SetRenderingControls(sRenderings);
}

CScenePlayer::~CScenePlayer()
{
	EndThreadpoolTimer();

	if (m_pDxgiSwapChain1 != nullptr)
	{
		m_pDxgiSwapChain1->Release();
		m_pDxgiSwapChain1 = nullptr;
	}

	if (m_pD2d1DeviceContext != nullptr)
	{
		m_pD2d1DeviceContext->Release();
		m_pD2d1DeviceContext = nullptr;
	}

	if (m_pD2d1Factory1 != nullptr)
	{
		m_pD2d1Factory1->Release();
		m_pD2d1Factory1 = nullptr;
	}

	if (SUCCEEDED(m_hrComInit))
	{
		::CoUninitialize();
	}
}
/*ファイル設定*/
bool CScenePlayer::SetFiles(const std::vector<std::wstring>& filePaths)
{
	Clear();
	for (const std::wstring& filePath : filePaths)
	{
		LoadImageToMemory(filePath.c_str());
	}
	ResetScale();

	return m_images.size() > 0;
}
/*描画*/
bool CScenePlayer::DrawImage()
{
	if (m_images.empty() || m_nIndex >= m_images.size() || m_pD2d1DeviceContext == nullptr || m_pDxgiSwapChain1 == nullptr)
	{
		return false;
	}

	ID2D1Bitmap* p = m_images[m_nIndex].p;
	if (p == nullptr)return false;

	D2D1_SIZE_U s = p->GetPixelSize();
	FLOAT fScale = static_cast<FLOAT>(::round(m_dbScale * 1000) / 1000);

	CComPtr<ID2D1Effect> pD2d1Effect;
	HRESULT hr = m_pD2d1DeviceContext->CreateEffect(CLSID_D2D1Scale, &pD2d1Effect);
	pD2d1Effect->SetInput(0, p);
	hr = pD2d1Effect->SetValue(D2D1_SCALE_PROP_CENTER_POINT, D2D1::Vector2F(static_cast<FLOAT>(m_iXOffset), static_cast<FLOAT>(m_iYOffset)));
	hr = pD2d1Effect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(fScale, fScale));

	m_pD2d1DeviceContext->BeginDraw();
	m_pD2d1DeviceContext->DrawImage(pD2d1Effect, D2D1::Point2F(0.f, 0.f), D2D1::RectF(static_cast<FLOAT>(m_iXOffset), static_cast<FLOAT>(m_iYOffset), s.width * fScale, s.height * fScale), D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC, D2D1_COMPOSITE_MODE_SOURCE_COPY);
	m_pD2d1DeviceContext->EndDraw();

	if (!m_bPause)
	{
		if (m_nIndex > Division::kFirstAnimation && m_nIndex < Division::kSecondAnimation)
		{
			StartThreadpoolTimer();
			++m_nIndex;
			if (m_nIndex >= Division::kSecondAnimation)m_nIndex = Division::kFirstAnimation + 1LL;
		}
		else if (m_nIndex >= Division::kSecondAnimation && m_nIndex < m_images.size() - 1)
		{
			StartThreadpoolTimer();
			++m_nIndex;
			if (m_nIndex >= m_images.size() - 1)m_nIndex = Division::kSecondAnimation + 1LL;
		}
	}

	return true;
}
/*転写*/
void CScenePlayer::Display()
{
	if (m_pDxgiSwapChain1 != nullptr)
	{
		DXGI_PRESENT_PARAMETERS params{};
		m_pDxgiSwapChain1->Present1(1, 0, &params);
	}
}
/*次画像*/
void CScenePlayer::Next()
{
	if (m_nIndex > Division::kFirstAnimation && m_nIndex < Division::kSecondAnimation)
	{
		if (m_bPause)
		{
			++m_nIndex;
			if (m_nIndex >= Division::kSecondAnimation)m_nIndex = Division::kFirstAnimation + 1LL;
		}
		else
		{
			EndThreadpoolTimer();
			m_nIndex = Division::kSecondAnimation + 1LL;
		}
	}
	else if (m_nIndex >= Division::kSecondAnimation && m_nIndex < m_images.size() - 1)
	{
		if (m_bPause)
		{
			++m_nIndex;
			if (m_nIndex >= m_images.size() - 1)m_nIndex = Division::kSecondAnimation + 1LL;
		}
		else
		{
			EndThreadpoolTimer();
			m_nIndex = m_images.size() - 1;
		}
	}
	else
	{
		if (!m_bPause)
		{
			EndThreadpoolTimer();
			++m_nIndex;
		}
	}
	if (m_nIndex >= m_images.size())m_nIndex = 0;
	Update();
}
/*拡大*/
void CScenePlayer::UpScale()
{
	if (m_dbScale < 3.99)
	{
		m_dbScale += 0.05;
		ResizeWindow();
	}
}
/*縮小*/
void CScenePlayer::DownScale()
{
	if (m_dbScale > 0.51)
	{
		m_dbScale -= 0.05;
		ResizeWindow();
	}
}
/*原寸表示*/
void CScenePlayer::ResetScale()
{
	m_dbScale = 1.0;
	m_iXOffset = 0;
	m_iYOffset = 0;
	m_interval = Portion::kInterval;
	ResizeWindow();
}
/*窓枠寸法計算法切り替え*/
void CScenePlayer::SwitchSizeLore(bool bBarHidden)
{
	m_bBarHidden = bBarHidden;
	ResizeWindow();
}
/*原点位置移動*/
void CScenePlayer::SetOffset(int iX, int iY)
{
	AdjustOffset(iX, iY);
	Update();
}
/*コマ送り加速*/
void CScenePlayer::SpeedUp()
{
	if (m_interval > 1)
	{
		--m_interval;
	}
}
/*コマ送り減速*/
void CScenePlayer::SpeedDown()
{
	if (m_interval < 1000)
	{
		++m_interval;
	}
}
/*一時停止切り替え*/
bool CScenePlayer::SwitchPause()
{
	m_bPause ^= true;
	return m_bPause;
}
/*消去*/
void CScenePlayer::Clear()
{
	m_images.clear();
	m_nIndex = 0;
}
/*画像ファイル取り込み*/
bool CScenePlayer::LoadImageToMemory(const wchar_t* pzFilePath)
{
	CComPtr<IWICImagingFactory> pWicImageFactory;
	HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImageFactory));
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapDecoder> pWicBitmapDecoder;
	hr = pWicImageFactory->CreateDecoderFromFilename(pzFilePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pWicBitmapDecoder);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapFrameDecode> pWicFrameDecode;
	hr = pWicBitmapDecoder->GetFrame(0, &pWicFrameDecode);
	if (FAILED(hr))return false;

	CComPtr<IWICFormatConverter> pWicFormatConverter;
	hr = pWicImageFactory->CreateFormatConverter(&pWicFormatConverter);
	if (FAILED(hr))return false;

	pWicFormatConverter->Initialize(pWicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmap> pWicBitmap;
	hr = pWicImageFactory->CreateBitmapFromSource(pWicFormatConverter, WICBitmapCacheOnDemand, &pWicBitmap);
	if (FAILED(hr))return false;

	CComPtr<ID2D1Bitmap> pD2d1Bitmap;
	hr = m_pD2d1DeviceContext->CreateBitmapFromWicBitmap(pWicBitmap, D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)), &pD2d1Bitmap);
	if (FAILED(hr))return false;

	m_images.push_back(std::move(pD2d1Bitmap));

	return true;
}
/*画面更新*/
void CScenePlayer::Update()
{
	if (m_hRetWnd != nullptr)
	{
		::InvalidateRect(m_hRetWnd, NULL, TRUE);
	}
}
/*窓枠寸法調整*/
void CScenePlayer::ResizeWindow()
{
	if (!m_images.empty() && m_hRetWnd != nullptr)
	{
		RECT rect;
		if (!m_bBarHidden)
		{
			::GetWindowRect(m_hRetWnd, &rect);
		}
		else
		{
			::GetClientRect(m_hRetWnd, &rect);
		}

		ID2D1Bitmap* p = m_images[0].p;
		if (p == nullptr)return;
		D2D1_SIZE_U s = p->GetPixelSize();

		int iX = static_cast<int>(::round(s.width * (m_dbScale * 1000) / 1000));
		int iY = static_cast<int>(::round(s.height * (m_dbScale * 1000) / 1000));
		rect.right = iX + rect.left;
		rect.bottom = iY + rect.top;
		if (!m_bBarHidden)
		{
			::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, TRUE);
			::SetWindowPos(m_hRetWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
		}
		else
		{
			RECT rc;
			::GetWindowRect(m_hRetWnd, &rc);
			::MoveWindow(m_hRetWnd, rc.left, rc.top, rect.right, rect.bottom, TRUE);
		}

		ResizeBuffer();
		AdjustOffset(0, 0);
		Update();
	}

}
/*原点位置調整*/
void CScenePlayer::AdjustOffset(int iXAddOffset, int iYAddOffset)
{
	if (!m_images.empty() && m_hRetWnd != nullptr)
	{
		ID2D1Bitmap* p = m_images[0].p;
		if (p == nullptr)return;
		D2D1_SIZE_U s = p->GetPixelSize();

		int iX = static_cast<int>(::round(s.width * (m_dbScale * 1000) / 1000));
		int iY = static_cast<int>(::round(s.height * (m_dbScale * 1000) / 1000));

		RECT rc;
		::GetClientRect(m_hRetWnd, &rc);

		int iClientWidth = rc.right - rc.left;
		int iClientHeight = rc.bottom - rc.top;

		int iXOffsetMax = iX > iClientWidth ? static_cast<int>(::floor((iX - iClientWidth) / ((m_dbScale * 1000) / 1000))) : 0;
		int iYOffsetMax = iY > iClientHeight ? static_cast<int>(::floor((iY - iClientHeight) / ((m_dbScale * 1000) / 1000))) : 0;

		m_iXOffset += iXAddOffset;
		if (m_iXOffset < 0) m_iXOffset = 0;
		if (m_iXOffset > iXOffsetMax)m_iXOffset = iXOffsetMax;
		m_iYOffset += iYAddOffset;
		if (m_iYOffset < 0) m_iYOffset = 0;
		if (m_iYOffset > iYOffsetMax)m_iYOffset = iYOffsetMax;
	}

}
/*原版寸法変更*/
void CScenePlayer::ResizeBuffer()
{
	if (m_pDxgiSwapChain1 != nullptr && m_pD2d1DeviceContext != nullptr && m_hRetWnd != nullptr)
	{
		m_pD2d1DeviceContext->SetTarget(nullptr);

		RECT rc;
		::GetClientRect(m_hRetWnd, &rc);
		HRESULT hr = m_pDxgiSwapChain1->ResizeBuffers(0, rc.right - rc.left, rc.bottom - rc.top, DXGI_FORMAT_B8G8R8A8_UNORM, 0);

		CComPtr<IDXGISurface> pDxgiSurface;
		hr = m_pDxgiSwapChain1->GetBuffer(0, IID_PPV_ARGS(&pDxgiSurface));

		CComPtr<ID2D1Bitmap1> pD2d1Bitmap1;
		hr = m_pD2d1DeviceContext->CreateBitmapFromDxgiSurface(pDxgiSurface, nullptr, &pD2d1Bitmap1);

		m_pD2d1DeviceContext->SetTarget(pD2d1Bitmap1);
	}
}
/*コマ送り開始*/
void CScenePlayer::StartThreadpoolTimer()
{
	if (m_timer != nullptr)return;

	m_timer = ::CreateThreadpoolTimer(TimerCallback, this, nullptr);
	if (m_timer != nullptr)
	{
		FILETIME FileDueTime{};
		ULARGE_INTEGER ulDueTime{};
		ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL * 10 * 1000 * m_interval));
		FileDueTime.dwHighDateTime = ulDueTime.HighPart;
		FileDueTime.dwLowDateTime = ulDueTime.LowPart;
		::SetThreadpoolTimer(m_timer, &FileDueTime, 0, 0);
	}

}
/*コマ送り終了*/
void CScenePlayer::EndThreadpoolTimer()
{
	if (m_timer != nullptr)
	{
		::SetThreadpoolTimer(m_timer, nullptr, 0, 0);
		::WaitForThreadpoolTimerCallbacks(m_timer, TRUE);
		::CloseThreadpoolTimer(m_timer);
		m_timer = nullptr;
	}
}
/*コマ送り処理スレッドプール*/
void CScenePlayer::TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer)
{
	CScenePlayer* pThis = static_cast<CScenePlayer*>(Context);
	if (pThis != nullptr)
	{
		pThis->Update();

		FILETIME FileDueTime{};
		ULARGE_INTEGER ulDueTime{};
		ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL * 10 * 1000 * pThis->m_interval));
		FileDueTime.dwHighDateTime = ulDueTime.HighPart;
		FileDueTime.dwLowDateTime = ulDueTime.LowPart;

		::SetThreadpoolTimer(Timer, &FileDueTime, 0, 0);
	}

}