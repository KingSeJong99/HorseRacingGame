#include "Track/LineTrack.h"
#include "Race/RaceOrganizer.h"
#include "Render/Renderer.h"
#include "Math/Vector2.h"
#include "Timer/cpu_timer.h"
#include <algorithm>
#include <iostream>
#include <Windows.h>
#include "Engine/Engine.h"
#include "UI/HorseRaceHUD.h"



// 가로를 분할하기 위한 값
// int scoreboard_x = 27;

static bool up_pressed = false;
static bool down_pressed = false;
static bool enter_pressed = false;

horseracing::LineTrack::LineTrack() {
}

horseracing::LineTrack::~LineTrack() {
	
	for (auto horse : horses_) {
		delete horse;
	}

	// 소멸자기 때문에 생략이 가능하지만 명시적으로 작성함
	horses_.clear();
}

void horseracing::LineTrack::Draw(Mint::Renderer& renderer, int width, int height) {
	UI::TextLayout textLayout(renderer);
	HorseRaceHUD hud(textLayout);

	RenderLayout render_layout;
	render_layout.width = width;
	render_layout.height = height;
	render_layout.track_start_x = 2;
	render_layout.track_end_x = width / 2;
	render_layout.ui_start_x = render_layout.track_start_x + render_layout.track_end_x + 2;
	render_layout.ui_end_x = render_layout.ui_start_x + 20;
	render_layout.log_start_x = render_layout.ui_end_x + 4;
	render_layout.draw_y_start = 3;

	if (!is_racing_started_) {
		hud.DrawBettingMenu(render_layout, horses_, selected_horse_idx_);
	} else {
		hud.DrawTrack(render_layout);
		hud.DrawHorses(render_layout, horses_);
		hud.DrawRankUI(render_layout, horses_, sorted_horses_);
		hud.DrawRaceLogs(render_layout, raceLogs_);
	}
	if (is_race_over_) {
		hud.DrawScoreboard(render_layout, sorted_horses_);
	}
}

void horseracing::LineTrack::Tick(float deltaTime)
{
	HandleInput();

	if (!is_racing_started_) {
		// ↑, ↓ 키로 말을 선택한다 (0 ~ 7번을 순환하며 좌표를 매핑함)
		return;
	}

	double total_time = timer_.GetTotalTime();
	this->Update(deltaTime, total_time);
}

void horseracing::LineTrack::LoadMap(const char* file_name) {
	
	track_width_ = 10.0f;
}

// 경기에 출전하는 말들을 데려온다
void horseracing::LineTrack::PrepareNewGame(RaceOrganizer& organizer) {
	// 달리고 있던 말들 보내기
	for (auto horse : horses_) {
		delete horse;
	}
	horses_.clear();
	sorted_horses_.clear();

	this->LoadMap(nullptr); // 트랙 너비를 초기화하는 LoadMap 호출 추가

	// 선발된 말들에 대한 포인터 소유권을 얻음, Ownership!
	// 즉, 생성에 대한 권한(책임)도 갖게 되었다!
	horses_ = organizer.OrganizeRace(8);

	// 순위 체크용 벡터 데려오기
	sorted_horses_ = horses_;

	// 선수 입장
	for (size_t i = 0; i < horses_.size(); ++i) {
		// Horse 클래스에 SetlaneIndex 같은게 있다면 여기서 호출한다
		horses_[i]->SetLane(static_cast<int>(i + 1));
	}

	// Todo: 디버깅용으로 시작하자마자 초기화하도록함
	this->Reset();
}

// 경기 시작을 위한 값 초기화
void horseracing::LineTrack::Reset() {
	sorted_horses_ = horses_;
	finished_horse_count_ = 0;
	is_race_over_ = false;
	is_racing_started_ = false;

	for (auto horse : horses_) {
		horse->Reset();
	}

	timer_.Reset();
}

// 오타마타들이 움직이기 시작한다
void horseracing::LineTrack::Update(float delta_time, double total_time) {
	// 무의미한 반복 방지
	if (is_race_over_)	return;	

	for (auto horse : horses_) {
		const auto& data = horse->GetHorseRaceData();
		float prev_pos = data.position;

		horse->SetCurrentTotalTime(total_time);

		horse->Run(delta_time, track_width_);
		
		float curr_pos = data.position;
		bool horse_is_finished = data.is_finished;

		// 말이 트랙의 절반을 지나친 경우
		if (prev_pos < 0.25f && curr_pos >= 0.25f) {
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말이 1구역을 통과!");
		}

		// 말이 트랙의 절반을 지나친 경우
		if (prev_pos < 0.5f && curr_pos >= 0.5f) {
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말이 반환점을 통과!");
		}

		// 말이 트랙의 절반을 지나친 경우
		if (prev_pos < 0.75f && curr_pos >= 0.75f) {
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말이 결승선을 향해 질주!");
		}

		// 말이 트랙의 끝에 도달한 경우
		if ((prev_pos < 1.0f && curr_pos >= 1.0f) && (horse_is_finished)) {
			double total_time = timer_.GetTotalTime();
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말 도착!");

			++finished_horse_count_;
		}
	}

	UpdateRanks();

	// 경기 종료 판단
	if (finished_horse_count_ == horses_.size()) {
		is_race_over_ = true;
		for (auto horse : horses_) {
			checking_position_ = horse->GetHorseRaceData().position;

			// 비정상적인 경기 종료 감지
			if (checking_position_ < 1.0f) {
				std::cerr << "비정상적인 경기 종료가 발생했습니다!\n";

				// Todo: 엔진에서 자체 로그 시스템 만들기
				std::clog << " finished-horse_count : " << finished_horse_count_ << "\n";
				__debugbreak();
			}	 
		}
		
		AddRaceLog(L"경기가 좋료되었습니다!!");

		// UpdateRanks를 통해 1등이 sort된 sorted_horses 활용
		if (!sorted_horses_.empty()) {
			AddRaceLog(std::to_wstring(sorted_horses_[0]->GetHorseRaceData().lane_index) + L"번 말이 우승을 차지합니다!");
		}			
	}
}

// 말들의 실시간 랭크 갱신 함수
void horseracing::LineTrack::UpdateRanks()
{
	
	std::sort(sorted_horses_.begin(), sorted_horses_.end(), [](Horse* a, Horse* b) {
		const auto& dataA = a->GetHorseRaceData();
		const auto& dataB = b->GetHorseRaceData();

		// 1. 완주했는지가 우선
		if (dataA.is_finished != dataB.is_finished) {
			return dataA.is_finished > dataB.is_finished;
		}

		// 위치가 똑같을 수 있을 때, 억울한 준우승을 방지하기 위한 규칙
		if (dataA.finish_time != dataB.finish_time) {
			return dataA.finish_time < dataB.finish_time;
		}

		// 단순한 위치만으로 말들의 위치 비교하기
		if (dataA.position != dataB.position) {
			return dataA.position > dataB.position;
		}

		return a->GetCurrentSpeed() > b->GetCurrentSpeed();
		});

	// 열심히 정렬한 순위를 말들에게 전해준다
	for (size_t i = 0; i < sorted_horses_.size(); i++) {
		// 0번째 인덱스가 1등이므로 i + 1을 해준다
		sorted_horses_[i]->SetRank(static_cast<int>(i + 1));
	}
}

// 로그 출력 함수
void horseracing::LineTrack::AddRaceLog(const std::wstring& msg) {
	raceLogs_.push_back(msg);
	if (raceLogs_.size() > max_log_count_) {
		raceLogs_.erase(raceLogs_.begin());		// 오래된 메시지를 삭제한다
	}
}

void horseracing::LineTrack::HandleInput() {

	// if (is_race_over_) {
	// 	// 키 입력 확인하기, r을 누르면 재시작
	// 	if (GetAsyncKeyState(VK_SPACE) & 0x8000 || GetAsyncKeyState('R') & 0x8000) {
	// 		needs_restart_ = true;
	// 		return;
	// 	}
	// }
	// 
	// if (!is_racing_started_) {
	// 	// // ↑, ↓ 키로 말을 선택한다 (0 ~ 7번을 순환하며 좌표를 매핑함)
	// 	if (GetAsyncKeyState(VK_UP) & 0x8000) {
	// 		selected_horse_idx_ = (selected_horse_idx_ - 1 + 8) % 8;
	// 		return;
	// 	}
	// 	else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
	// 		selected_horse_idx_ = (selected_horse_idx_ + 1) % 8;
	// 		return;
	// 	}
	// 
	// 	// 엔터키로 선택 확정 및 경주 시작
	// 	if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
	// 		is_racing_started_ = true;
	// 		return;
	// 	}
	// }

	// 키의 이전 상태를 기억하기 위한 정적 변수

  if (is_race_over_) {
	  // 키 입력 확인하기, r을 누르면 재시작
    if ((GetAsyncKeyState(VK_SPACE) & 0x8000) || (GetAsyncKeyState('R') & 0x8000)) {
      needs_restart_ = true;
      return;
    }
  }

  if (!is_racing_started_) {
    // --- 위쪽 방향키 체크 ---
    bool up_now = (GetAsyncKeyState(VK_UP) & 0x8000);
    if (up_now && !up_pressed) { // 이전에 안 눌렸는데 지금 눌린 경우
      selected_horse_idx_ = (selected_horse_idx_ - 1 + 8) % 8;
    }
    up_pressed = up_now; // 현재 상태 저장

    // --- 아래쪽 방향키 체크 ---
    bool down_now = (GetAsyncKeyState(VK_DOWN) & 0x8000);
    if (down_now && !down_pressed) {
      selected_horse_idx_ = (selected_horse_idx_ + 1) % 8;
    }
    down_pressed = down_now;

    // --- 엔터키 체크 ---
    bool enter_now = (GetAsyncKeyState(VK_RETURN) & 0x8000);
    if (enter_now && !enter_pressed) {
      is_racing_started_ = true;
    }
    enter_pressed = enter_now;
  }
}

