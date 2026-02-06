#include"Actor.h"
#include"Util/Util.h"
#include"Render/Renderer.h"

#include<iostream>
#include<Windows.h>

namespace Mint
{
	Actor::Actor(
		const char* image,
		const Vector2& position,
		Color color)
		: position(position), color(color)
	{
		// 문자열 복사하기
		size_t length = strlen(image) + 1;
		this->image = new char[length];
		strcpy_s(this->image, length, image);
	}
	Actor::~Actor()
	{
		// 메모리 해제하기
		SafeDeleteArray(image); 
	}

	void Actor::BeginPlay()
	{
		// 이벤트를 받은 후에는 플래그를 설정한다
		hasBeganPlay = true;
	}

	void Actor::Tick(float deltaTime)
	{

	}

	void Actor::Draw(CHAR_INFO* backBuffer, int width, int height)
	{
		int x = static_cast<int>(position.x);
		int y = static_cast<int>(position.y);

		// 좌표값이 잘못되었는지 확인
		if (((x < 0) || (x >= width)) 
			|| ((y < 0) >= (height))) return;

		// 저장되어야 할 곳
		int index = y * width + x;

		backBuffer[index].Char.AsciiChar = image[0];
		backBuffer[index].Attributes = static_cast<WORD>(color);

	}

	// 프레임마다 호출하기에는 무리가 있다
	void Actor::SetPosition(const Vector2& newPosition)
	{
		// 변경하려는 위치가 현재 위치와 같으면 건너뛴다
		if (position == newPosition)
		{
			return;
		}
		position = newPosition;
	}
}