#include "HorseRacing.h"
#include "Race/RaceOrganizer.h"
#include "Track/LineTrack.h"

#include <iostream>

// 싱글턴을 위한 메모리 공간 확보
horseracing::HorseRacing* horseracing::HorseRacing::instance_ = nullptr;

horseracing::HorseRacing::HorseRacing() {
	if (instance_ != nullptr) {
		std::cerr << "게임 로드 중 엔진 로딩에 실패했습니다!";
		__debugbreak();
	}
	instance_ = this;
}

horseracing::HorseRacing::~HorseRacing() {
	instance_ = nullptr;
}

void horseracing::HorseRacing::BeginPlay()
{
	// 데이터 로드 
	organizer_.LoadAllHorseData("horses.txt");

	// currentTrack_ 생성
	currentTrack_ = new LineTrack();

	// 생성된 트랙에게 말 정보를 넘겨준다
	currentTrack_->PrepareNewGame(organizer_);

	// 트랙을 레벨로 정식 인정
	this->mainLevel = currentTrack_;
}

void horseracing::HorseRacing::Tick(float deltaTime)
{
}

void horseracing::HorseRacing::Draw()
{
}
