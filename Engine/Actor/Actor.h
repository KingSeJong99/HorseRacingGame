#pragma once

#include"common/RTTI.h"
#include"Math/Vector2.h"
#include"Math/Color.h"

namespace Mint
{
	// 전방 선언
	class Level;

	class MINT_API Actor : public RTTI
	{
		// RTTI 코드 추가
		RTTI_DECLARATIONS(Actor, RTTI)

	public:
		Actor(
			const char* image = " ",
			const Vector2& position = Vector2::Zero,
			Color color = Color::White
		);
		virtual ~Actor();

		// 게임 플레이 이벤트
		virtual void BeginPlay();
		virtual void Tick(float deltaTime);
		virtual void Draw(CHAR_INFO* backBuffer, int width, int height);

		// 위치 변경 및 읽기 함수
		void SetPosition(const Vector2& newPosition);
		inline Vector2 GetPosition() const { return position; }

		// 오너쉽 추가/읽기 함수
		inline void SetOwner(Level* newOwer) { owner = newOwer; }
		inline Level* GetOwner() const { return owner; }

		// Getter 함수
		inline bool HasBeganPlay() const { return hasBeganPlay; }
		inline bool IsActive() const { return isActive && !destroyRequested; }
		inline bool DestroyRequested() const { return destroyRequested; }
	
		inline int GetSortingOrder() const { return sortingOrder; }

	protected:
		// BeginPlay 이벤트가 이미 있는지 확인하는 변수
		bool hasBeganPlay = false;

		// 활성화이 되어있는지 확인하는 변수
		bool isActive = true;

		// 현재 프레임에 삭제 요청을 받았는지 확인하는 변수
		bool destroyRequested = false;

		// 그릴 문자(이미지)
		char* image = nullptr;

		// 색상
		Color color = Color::White;

		// 오너쉽(Ownership)
		Level* owner = nullptr;
		
		// 그리기 우선 순위(값이 크면 우선순위가 높다)
		int sortingOrder = 0;

	private:
		// 위치
		Vector2 position;
	};
}
