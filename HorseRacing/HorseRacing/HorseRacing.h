#pragma once

#include "Engine/Engine.h"
#include "Race/RaceOrganizer.h"

#include <vector>

namespace horseracing {
	class LineTrack;
	
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

		// Initialize를 위한 객체 생성
		horseracing::RaceOrganizer organizer_;
		horseracing::LineTrack* currentTrack_ = nullptr;
	};
}


