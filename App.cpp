#include "App.h"

namespace
{
	constexpr int WIDTH = 400;
	constexpr int HEIGHT = 400;
	constexpr float rotationSpeed = 0.01f;   // radians par pixel
	constexpr float pitchLimit = 1.5707f;    // ~ pi/2
}

App::App()
	:
	wnd(WIDTH, HEIGHT, "d3d11_app")
{}

int App::Go()
{
	while (true)
	{
		if (const auto ecode = Window::ProcessMessages())
		{
			return *ecode;
		}
		DoFrame();
	}
}

void App::UpdateRotation()
{
	const auto [x, y] = wnd.mouse.GetPos();
	if (wnd.mouse.LeftIsPressed())
	{
		if (dragging)
		{
			yaw -= static_cast<float>(x - lastX) * rotationSpeed;
			pitch -= static_cast<float>(y - lastY) * rotationSpeed;
			if (pitch > pitchLimit)  pitch = pitchLimit;
			if (pitch < -pitchLimit) pitch = -pitchLimit;
		}
		dragging = true;
		lastX = x;
		lastY = y;
	}
	else
	{
		dragging = false;
	}
}

void App::HandleInput()
{
	// On vide toute la file d'événements à chaque frame
	while (!wnd.kbd.KeyIsEmpty())
	{
		const auto e = wnd.kbd.ReadKey();
		if (e.isPress() && e.GetCode() == VK_SPACE)
		{
			mode = (mode == RenderMode::Triangle)
				? RenderMode::Tetrahedron
				: RenderMode::Triangle;
		}
	}
}

void App::DoFrame()
{
	HandleInput();
	UpdateRotation();

	wnd.Gfx().ClearBuffer(0.08f, 0.08f, 0.12f);

	switch (mode)
	{
	case RenderMode::Triangle:
		wnd.Gfx().DrawTriangle();
		break;
	case RenderMode::Tetrahedron:
		wnd.Gfx().DrawTetrahedron(yaw, pitch);
		break;
	}

	wnd.Gfx().EndFrame();
}