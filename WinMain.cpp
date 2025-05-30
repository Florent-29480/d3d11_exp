#include <Windows.h>
#include <sstream>

#include "window.h"

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow )
{
	try
	{
		Window wnd(1160, 760, "d3d11_app");

		MSG msg;
		BOOL gResult;
		while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (wnd.kbd.KeyIsPressed(VK_SPACE))
			{
				MessageBox(nullptr, "Space", "Received Input !", MB_OK | MB_ICONEXCLAMATION);
			}
			if (wnd.mouse.LeftIsPressed())
			{
				MessageBox(nullptr, "Click !", "Received Input !", MB_OK | MB_ICONEXCLAMATION);
			}
		}
		if (gResult == -1)
		{
			return -1;
		}
		return msg.wParam;
	}
	catch (const ChiliException& e)
	{
		MessageBox(nullptr, e.what(),e.GetType(),MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e)
	{
		MessageBox(nullptr,e.what(),"Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBox(nullptr, "No details available","Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}

	return -1;
}