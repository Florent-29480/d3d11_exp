#include "Mouse.h"
#include "Windows.h"

std::pair<int, int> Mouse::GetPos() const noexcept
{
	return { x,y };
}

int Mouse::GetPosX() const noexcept
{
	return x;
}
int Mouse::GetPosY() const noexcept
{
	return y;
}
bool Mouse::IsInWindow() const noexcept
{
	return isInWindow;
}
bool Mouse::LeftIsPressed() const noexcept
{
	return leftIsPressed;
}
bool Mouse::RightIsPressed() const noexcept
{
	return rightIsPressed;
}
Mouse::Event Mouse::Read() noexcept
{
	if (buffer.size() > 0)
	{
		Mouse::Event e = buffer.front();
		buffer.pop();
		return e;
	}
	else
	{
		return Mouse::Event();
	}
}
bool Mouse::IsEmpty() const noexcept
{
	return buffer.empty();
}
void Mouse::Flush() noexcept
{
	buffer = std::queue<Mouse::Event>();
}
void Mouse::OnMouseMove(int newx, int newy) noexcept
{
	x = newx;
	y = newy;
	buffer.push(Mouse::Event(Mouse::Event::Type::Move, *this));
	TrimBuffer();
}
void Mouse::OnMouseEnter() noexcept
{
	isInWindow = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::Enter, *this));
	TrimBuffer();
}
void Mouse::OnMouseLeave() noexcept
{
	isInWindow = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::Leave, *this));
	TrimBuffer();
}
void Mouse::OnLeftPressed(int newx, int newy) noexcept
{
	x = newx;
	y = newy;
	leftIsPressed = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::LPress, *this));
	TrimBuffer();
}
void Mouse::OnRightPressed(int newx, int newy) noexcept
{
	x = newx;
	y = newy;
	rightIsPressed = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::RPress, *this));
	TrimBuffer();
}
void Mouse::OnLeftReleased(int newx, int newy) noexcept
{
	x = newx;
	y = newy;
	leftIsPressed = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::LRelease, *this));
	TrimBuffer();
}
void Mouse::OnRightReleased(int newx, int newy) noexcept
{
	x = newx;
	y = newy;
	rightIsPressed = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::RRealease, *this));
	TrimBuffer();
}
void Mouse::OnWheelUp(int x, int y) noexcept
{
	buffer.push(Mouse::Event(Mouse::Event::Type::WheelUp, *this));
	TrimBuffer();
}
void Mouse::OnWheelDown(int x, int y) noexcept
{
	buffer.push(Mouse::Event(Mouse::Event::Type::WheelDown, *this));
	TrimBuffer();
}
void Mouse::OnWheelDelta(int x, int y, int delta) noexcept
{
	wheelDeltaCarry += delta;
	//generate events by 120 increments (windows stuff)
	while (wheelDeltaCarry >= WHEEL_DELTA)
	{
		wheelDeltaCarry -= WHEEL_DELTA;
		OnWheelUp(x, y);
	}
	while (wheelDeltaCarry <= -WHEEL_DELTA)
	{
		wheelDeltaCarry += WHEEL_DELTA;
		OnWheelDown(x, y);
	}
}
void Mouse::TrimBuffer() noexcept
{
	while (buffer.size() > bufferSize)
	{
		buffer.pop();
	}
}
