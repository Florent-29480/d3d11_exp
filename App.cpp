#include "App.h"
#define WIDTH 400
#define HEIGHT 400

App::App()
	:
	wnd(WIDTH, HEIGHT, "d3d11_app")
{}

int App::Go()
{
	while (true)
	{
		if (const auto ecode = Window::ProcessMessages()) //ecode will be a bool set to true if it has a value
		{
			return *ecode;
		}
		DoFrame();
	}
}

void App::DoFrame()
{
	const float c = sin(timer.Peek()) / 2.0f + 0.5f;
	wnd.Gfx().ClearBuffer(c, c, 1.0f);
	wnd.Gfx().EndFrame();
}