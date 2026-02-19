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
	organizer_.LoadAllHorseData(L"../Config/horses.json");

	// currentTrack_ 생성
	currentTrack_ = new LineTrack();

	// 생성된 트랙에게 말 정보를 넘겨준다
	currentTrack_->PrepareNewGame(organizer_);

	// 트랙을 레벨로 정식 인정
	this->mainLevel = currentTrack_;
}

void horseracing::HorseRacing::Tick(float deltaTime)
{
	Mint::Engine::Tick(deltaTime);
	
	// mainLevel이 정상적으로 할당되었다면
	if (mainLevel) {

		// 재시작 요청이 들어왔다면
		if (currentTrack_->ShouldRestart()) {
			currentTrack_->Reset();
			currentTrack_->PrepareNewGame(organizer_);
			currentTrack_->ClearRestartFlag();
		}

		

		mainLevel->Tick(deltaTime);
	}

	else {
		std::cerr << "mainLevel 할당에 실패했습니다!";
		__debugbreak;
	}
}

void horseracing::HorseRacing::Draw()
{
	// 버퍼 전달하기
	if (mainLevel) {
		renderer->Clear();
		mainLevel->Draw(*renderer, (int)screen_size_.x, (int)screen_size_.y);
		renderer->Present();
	}

	else {
		std::cerr << "mainLevel 할당에 실패했습니다!";
		__debugbreak;
	}
}
