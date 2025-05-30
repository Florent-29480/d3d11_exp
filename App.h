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
private:
	Window wnd;
	Timer timer;
};