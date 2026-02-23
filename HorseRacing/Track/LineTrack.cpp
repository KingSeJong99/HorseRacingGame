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

static bool up_pressed = false;
static bool down_pressed = false;
static bool enter_pressed = false;

horseracing::LineTrack::LineTrack() {
}

horseracing::LineTrack::~LineTrack() {
	for (auto horse : horses_) {
		delete horse;
	}
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
		hud.DrawTrack(render_layout, renderer);
		hud.DrawHorses(render_layout, horses_, renderer);
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
		return;
	}

	double total_time = timer_.GetTotalTime();
	this->Update(deltaTime, total_time);
}

void horseracing::LineTrack::LoadMap(const char* file_name) {
	track_width_ = 10.0f;
}

void horseracing::LineTrack::PrepareNewGame(RaceOrganizer& organizer) {
	for (auto horse : horses_) {
		delete horse;
	}
	horses_.clear();
	sorted_horses_.clear();

	this->LoadMap(nullptr);

	horses_ = organizer.OrganizeRace(8);
	sorted_horses_ = horses_;

	for (size_t i = 0; i < horses_.size(); ++i) {
		horses_[i]->SetLane(static_cast<int>(i + 1));
	}

	this->Reset();
}

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

void horseracing::LineTrack::Update(float delta_time, double total_time) {
	if (is_race_over_)	return;	

	for (auto horse : horses_) {
		const auto& data = horse->GetHorseRaceData();
		float prev_pos = data.position;

		horse->SetCurrentTotalTime(total_time);
		horse->Run(delta_time, track_width_);
		
		float curr_pos = data.position;
		bool horse_is_finished = data.is_finished;

		if (prev_pos < 0.25f && curr_pos >= 0.25f) {
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말이 1구역을 통과!");
		}
		if (prev_pos < 0.5f && curr_pos >= 0.5f) {
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말이 반환점을 통과!");
		}
		if (prev_pos < 0.75f && curr_pos >= 0.75f) {
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말이 결승선을 향해 질주!");
		}
		if ((prev_pos < 1.0f && curr_pos >= 1.0f) && (horse_is_finished)) {
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말 도착!");
			++finished_horse_count_;
		}
	}

	UpdateRanks();

	if (finished_horse_count_ == horses_.size()) {
		is_race_over_ = true;
		AddRaceLog(L"경기가 좋료되었습니다!!");

		if (!sorted_horses_.empty()) {
			AddRaceLog(std::to_wstring(sorted_horses_[0]->GetHorseRaceData().lane_index) + L"번 말이 우승을 차지합니다!");
		}			
	}
}

void horseracing::LineTrack::UpdateRanks()
{
	std::sort(sorted_horses_.begin(), sorted_horses_.end(), [](Horse* a, Horse* b) {
		const auto& dataA = a->GetHorseRaceData();
		const auto& dataB = b->GetHorseRaceData();

		if (dataA.is_finished != dataB.is_finished) {
			return dataA.is_finished > dataB.is_finished;
		}
		if (dataA.finish_time != dataB.finish_time) {
			return dataA.finish_time < dataB.finish_time;
		}
		if (dataA.position != dataB.position) {
			return dataA.position > dataB.position;
		}
		return a->GetCurrentSpeed() > b->GetCurrentSpeed();
		});

	for (size_t i = 0; i < sorted_horses_.size(); i++) {
		sorted_horses_[i]->SetRank(static_cast<int>(i + 1));
	}
}

void horseracing::LineTrack::AddRaceLog(const std::wstring& msg) {
	raceLogs_.push_back(msg);
	if (raceLogs_.size() > max_log_count_) {
		raceLogs_.erase(raceLogs_.begin());
	}
}

void horseracing::LineTrack::HandleInput() {
  if (is_race_over_) {
    if ((GetAsyncKeyState(VK_SPACE) & 0x8000) || (GetAsyncKeyState('R') & 0x8000)) {
      needs_restart_ = true;
      return;
    }
  }

  if (!is_racing_started_) {
    bool up_now = (GetAsyncKeyState(VK_UP) & 0x8000);
    if (up_now && !up_pressed) {
      selected_horse_idx_ = (selected_horse_idx_ - 1 + 8) % 8;
    }
    up_pressed = up_now;

    bool down_now = (GetAsyncKeyState(VK_DOWN) & 0x8000);
    if (down_now && !down_pressed) {
      selected_horse_idx_ = (selected_horse_idx_ + 1) % 8;
    }
    down_pressed = down_now;

    bool enter_now = (GetAsyncKeyState(VK_RETURN) & 0x8000);
    if (enter_now && !enter_pressed) {
      is_racing_started_ = true;
    }
    enter_pressed = enter_now;
  }
}
