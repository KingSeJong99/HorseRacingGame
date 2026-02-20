#pragma once

// 4251 메시지를 차단한다!
#pragma warning(disable: 4251)

// STL(Standard Tamplate Library - C++에서 기본적으로 제공하는 라이브러리)
// 크기가 알아서 변경되는 동적배열
#include"common/RTTI.h"
#include<vector>
#include <Windows.h>

namespace Mint
{
	// Actor 클래스 전방선언
	class Actor;
	class Renderer;

	// 역할 : 레벨에 있는 모든 물체(Actor)를 관리한다.
	class MINT_API Level : public RTTI
	{
		RTTI_DECLARATIONS(Level, RTTI)

	public:
		Level();
		virtual ~Level();

		// 게임 플레이 이벤트
		virtual void BeginPlay();
		virtual void Tick(float deltaTime);
		virtual void Draw(Renderer& renderer, int width, int height);

		// 액터 추가 함수
		void AddNewActor(Actor* newActor);

		// 액터 추가/제거 처리 함수
		void ProcessAddAndDestroyActors();

	protected:
		// 액터 배열을 추가한다
		std::vector<Actor*> actors;

		// 실행 중에 추가 요청된 액터의 배열
		// actors 배열은 이미 Tick에서 활동중이므로 임시로 저장한다!
		std::vector<Actor*> addRequestedActors;
	};

}
