#pragma once
#include "window.h"
#include "Timer.h"

class App
{
public:
	App();
	// Message Loop
	int Go();
private:
	void DoFrame();
	void UpdateRotation();
	void HandleInput();
private:
	enum class RenderMode
	{
		Triangle,
		Tetrahedron
	};

	Window wnd;
	Timer timer;
	RenderMode mode = RenderMode::Triangle;
	// Orientation du tétraèdre (radians) et état du glissé
	float yaw = 0.0f;
	float pitch = 0.0f;
	bool dragging = false;
	int lastX = 0;
	int lastY = 0;
};