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

	void Actor::Draw()
	{
		// 렌더러에 그리기를 요청한다
		// Renderer::Draw(position, color, image);

		// 렌더러에 데이터를 제출한다
		Renderer::Get().Submit(image, position, color, sortingOrder);
	}

	// 프레임마다 호출하기에는 무리가 있다
	void Actor::SetPosition(const Vector2& newPosition)
	{
		// 렌더러에 빈칸 그리기를 요청한다
		// Renderer::Draw(position, ' ');

		// 변경하려는 위치가 현재 위치와 같으면 건너뛴다
		if (position == newPosition)
		{
			return;
		}





		// 액터의 좌표로 콘솔에서의 좌표 위치를 이동한다
		// 액터의 현재 좌표로 콘솔에서의 좌표를 이동한다
						
		// 코드리뷰 후 삭제한다
		// COORD coord = {};
		// coord.X = static_cast<short>(position.x);
		// coord.Y = static_cast<short>(position.y);
		// 
		// SetConsoleCursorPosition(
		// 	GetStdHandle(STD_OUTPUT_HANDLE),
		// 	coord
		// );
		
		// 코드리뷰 후 삭제한다
		// Util::SetConsolePosition(position);

		// 해당 위치의 글자 값을 지우기(빈칸 그리기)
		// std::cout << ' ';

		// 새로운 위치 설정
		position = newPosition;
	}
}