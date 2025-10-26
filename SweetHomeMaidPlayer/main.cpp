
#include <shellscalingapi.h>

#include "framework.h"
#include "main_window.h"

#pragma comment (lib, "Shcore.lib")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    ::SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);

    int iRet = 0;
    CMainWindow* pMainWindow = new CMainWindow();
    if (pMainWindow != nullptr)
    {
        bool bRet = pMainWindow->Create(hInstance);
        if (bRet)
        {
            ::ShowWindow(pMainWindow->GetHwnd(), nCmdShow);
            iRet = pMainWindow->MessageLoop();
        }

        delete pMainWindow;
    }

    return iRet;
}
