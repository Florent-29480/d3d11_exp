#include "App.h"
#define WIDTH 800
#define HEIGHT 600

App::App()
	:
	wnd(WIDTH, HEIGHT, "d3d11_app")
{}

int App::Go()
{
	MSG msg;
	BOOL gResult;
	while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//check for errors
	if (gResult == -1)
	{
		throw CHWND_LAST_EXCEPT();
	}
	//wParam passed to PostQuitMessage
	return msg.wParam;
}

void App::DoFrame()
{

}