#pragma once

#include"Common/common.h"

namespace Mint
{
	class MINT_API Input
	{
		friend class Engine;

		// 데이터
		// down(누르기), up(떼기), hold(유지)
		struct KeyState
		{
			// 현재 키가 눌렸는가?
			bool isKeyDown = false;

			// 이전에 키가 눌렸었는가?
			bool wasKeyDown = false;
		};
	public:
		// 입력을 확인하는 함수들
		// 이전에 입력이 안됐지만 현재 입력이 됐으면 한 번 호출한다
		bool GetKeyDown(int keyCode);

		// 이전에 입력이 됐었지만 현재 입력이 취소됐으면 한 번 호출한다
		bool GetKeyUp(int keyCode);

		// 현재 눌려있으면 반복적으로 호출한다
		bool GetKey(int keyCode);

		// 전역적으로 접근하는 함수
		static Input& Get();

	private:
		Input();
		~Input();
		
		// 입력 처리 함수
		void ProcessInput();

		// 기존 입력 저장 함수
		void SavePreviousInputStates();

		// 키 상태 저장용 배열
		KeyState keyStates[255] = { };

		// 전역적으로 접근하도록 만들기 위한 전역 변수
		static Input* instance;
	};
}

