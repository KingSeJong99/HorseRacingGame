#include "HorseRacing.h"
#include "Race/RaceOrganizer.h"
#include "Track/LineTrack.h"
#include "Render/Renderer.h"

#include <iostream>

// 싱글턴을 위한 메모리 공간 확보
horseracing::HorseRacing* horseracing::HorseRacing::instance_ = nullptr;

horseracing::HorseRacing::HorseRacing() {
	if (instance_ != nullptr) {
		std::cerr << "게임 로드 중 엔진 로딩에 실패했습니다!";
		__debugbreak();
	}
	instance_ = this;

	// Todo: screen_size_ 값을 어떻게 조절할 것인가?
	this->screen_size_.x = (float)this->setting.width;
	this->screen_size_.y = (float)this->setting.height;
}

horseracing::HorseRacing::~HorseRacing() {
	instance_ = nullptr;
}

void horseracing::HorseRacing::BeginPlay()
{
	// 데이터 로드 
	organizer_.LoadAllHorseData(L"horses.txt");

	// currentTrack_ 생성
	currentTrack_ = new LineTrack();

	// 생성된 트랙에게 말 정보를 넘겨준다
	currentTrack_->PrepareNewGame(organizer_);

	// Hack: 디버깅
	std::cout << "BeginPlay() 진행중....\n";

	// 트랙을 레벨로 정식 인정
	this->mainLevel = currentTrack_;

	// Hack: 디버깅
	std::cout << "BeginPlay() 끝....!\n";
}

void horseracing::HorseRacing::Tick(float deltaTime)
{
	Mint::Engine::Tick(deltaTime);
	
	// mainLevel이 정상적으로 할당되었다면
	if (mainLevel) {
		mainLevel->Tick(deltaTime);
	}

	else {
		std::cerr << "mainLevel 할당에 실패했습니다!";
		__debugbreak;
	}
}

void horseracing::HorseRacing::Draw()
{
	// 버퍼 지우기
	CHAR_INFO* buffer = renderer->GetFrameBuffer();
	
	// 지우기
	this->Clear(buffer, screen_size_.x, screen_size_.y);

	// 버퍼 전달하기
	if (mainLevel) {
		mainLevel->Draw(buffer, screen_size_.x, screen_size_.y);
	}

	// 히히 발싸!
	renderer->Present();
}
