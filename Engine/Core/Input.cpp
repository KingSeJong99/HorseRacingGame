#include "Input.h"
#include<Windows.h>
#include<iostream>

namespace Mint 
{
	// 전역 변수 초기화
	Input* Input::instance = nullptr;

	Input::Input()
	{
		// 객체가 초기화되면 자기 자신의 주소를 저장
		instance = this;
	}
	Input::~Input()
	{
	}
	bool Input::GetKeyDown(int keyCode)
	{
		return keyStates[keyCode].isKeyDown
			&& !keyStates[keyCode].wasKeyDown;
	}

	bool Input::GetKeyUp(int keyCode)
	{
		return !keyStates[keyCode].isKeyDown
			&& keyStates[keyCode].wasKeyDown;
	}

	bool Input::GetKey(int keyCode)
	{
		return keyStates[keyCode].isKeyDown;
	}


	Input& Input::Get()
	{
		// 싱글턴(Singleton) 패턴을 적용한다
		// 이 함수는 Contents 프로젝트에서 접근하기 때문에
		// 엔진은 이미 초기화가 완료된 상태이다.
		if (!instance)
		{
			std::cout << "Error: input::Get(). instance is null\n";

			// 디버그 모드에서만 동작하는 명령어로, 자동으로 중단점이 걸린다 !!
			__debugbreak();
		}

		// Lazy-Pattern from effective C++
		// 생성을 초기에 하지 아지 않고 필요시에 한다!
		// static Input instance;

		return *instance;
	}

	void Input::ProcessInput()
	{
		// 키 마다 입력을 읽는다
		// (중요)운영체제가 제공하는 기능을 사용할 수 밖에 없다 !!
		for (int ix = 0; ix < 255; ++ix)
		{
			keyStates[ix].isKeyDown
				= (GetAsyncKeyState(ix) & 0x8000) > 0 ? true : false;
		}
	}

	void Input::SavePreviousInputStates()
	{
		// 현재 입력 값을 이전 입력 값으로 저장한다
		// 키 마다 입력을 읽는다
		// (중요)운영체제가 제공하는 기능을 사용할 수 밖에 없다 !!
		for (int ix = 0; ix < 255; ++ix)
		{
			keyStates[ix].wasKeyDown
				= keyStates[ix].isKeyDown;
		}
	}
}