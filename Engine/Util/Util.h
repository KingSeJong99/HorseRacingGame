#pragma once

#include"Math/Vector2.h"
#include"Math/Color.h"

using namespace Mint;

// 헤더파일에 구현이 되어있어 동일 이름의 함수가 겹쳐버렸다. 이를 해결하기 위해서
// 헤더와 소스코드를 분리해야하지만 inline을 붙여버렸다. inline을 붙이게 될 경우
// 함수는 하나의 구문이 되어버리기 때문에 링커의 감시를 가라로 피할 수 있다!
// 즉, 이 함수가 공용이 아니라는 것을 알려야 할 필요가 있다

// 헬퍼(Helper) 기능을 제공함
namespace Util
{
	// 콘솔 커서 위치 이동(설정)하는 함수
	inline void SetConsolePosition(const Vector2& position)
	{
		SetConsoleCursorPosition(
			GetStdHandle(STD_OUTPUT_HANDLE),
			static_cast<COORD>(position)
		);
	}

	//콘솔 텍스트 설정 함수
	inline void SetConsoleTextColor(Color color)
	{
		SetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE), static_cast<unsigned short>(color));
	}

	// 커서 끄기
	inline void TurnOffCursor()
	{
		// 커서를 꺼버린다
		CONSOLE_CURSOR_INFO info = {};
		GetConsoleCursorInfo(
			GetStdHandle(STD_OUTPUT_HANDLE), &info);

		info.bVisible = false;
		SetConsoleCursorInfo(
			GetStdHandle(STD_OUTPUT_HANDLE), &info);

	}

	// 커서 켜기
	inline void TurnOnCursor()
	{
		// 꺼진 커서를 다시 킨다 
		CONSOLE_CURSOR_INFO info = {};
		GetConsoleCursorInfo(
			GetStdHandle(STD_OUTPUT_HANDLE), &info);

		info.bVisible = true;
		SetConsoleCursorInfo(
			GetStdHandle(STD_OUTPUT_HANDLE), &info);

	}
}

// 메모리 정리 함수 추가하기
template<typename T>
void SafeDelete(T*& t)
{
	if (t)
	{
		delete t;
		t = nullptr;
	}
}

template<typename T>
void SafeDeleteArray(T*& t)
{
	if (t)
	{
		delete[] t;
		t = nullptr;
	}
}