#ifndef HORSERACING__HORSERACING_HORCERACING_H_
#define HORSERACING__HORSERACING_HORCERACING_H_

#include "Engine/Engine.h"
#include <vector>

namespace horseracing {
	// Engine의 실체
	class HorseRacing : public Mint::Engine
	{

	public:
		HorseRacing();
		~HorseRacing();
	
		virtual void BeginPlay() override;
	
		// 업데이트 함수.
		virtual void Tick(float deltaTime) override;
	
		// 그리기 함수. (Draw/Render).
		virtual void Draw() override;

	private:
		static HorseRacing* instance_;
	};
}

#endif HORSERACING__HORSERACING_HORCERACING_H_


