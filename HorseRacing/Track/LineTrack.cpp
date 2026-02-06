#include "LineTrack.h"
#include <algorithm>
#include <iostream>

horseracing::LineTrack::LineTrack() {

}

horseracing::LineTrack::~LineTrack() {
	
	for (auto horse : horses_) {
		delete horse;
	}

	

	// 소멸자기 때문에 생략이 가능하지만 명시적으로 작성함
	horses_.clear();
}

void horseracing::LineTrack::Draw() {
}

void horseracing::LineTrack::LoadMap(const char* file_name) {
}

// 경기 시작을 위한 값 초기화
void horseracing::LineTrack::Reset()
{
	sorted_horses_ = horses_;
	finished_horse_count_ = 0;
	is_race_over_ = false;

	for (auto horse : horses_) {
		horse->Reset();
	}
}

// 오타마타들이 움직이기 시작한다
void horseracing::LineTrack::Update(float delta_time) {
	// 무의미한 반복 방지
	if (is_race_over_)	return;	

	for (auto horse : horses_) {
		const auto& data = horse->GetHorseRaceData();
		float prev_pos = data.position;

		horse->Run(delta_time, track_width_);
		
		float curr_pos = data.position;
		bool horse_is_finished = data.is_finished;

		// 말이 트랙의 절반을 지나친 경우
		if (prev_pos < 0.5f && curr_pos >= 0.5f) {
			AddRaceLog(std::to_string(data.lane_index) + "번 말이 반환점을 통과합니다!");
		}

		// 말이 트랙의 끝에 도달한 경우
		if ((prev_pos < 1.0f && curr_pos >= 1.0f) && (horse_is_finished)) {
			AddRaceLog(std::to_string(data.lane_index) + "번 말이 도착합니다!");
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
		
		AddRaceLog("경기가 좋료되었습니다!!");

		// UpdateRanks를 통해 1등이 sort된 sorted_horses 활용
		if (!sorted_horses_.empty()) {
			AddRaceLog(std::to_string(sorted_horses_[0]->GetHorseRaceData().lane_index) + "번 말이 우승을 차지합니다!");
		}			
	}
}

// 말들의 실시간 랭크 갱신 함수
void horseracing::LineTrack::UpdateRanks()
{
	
	std::sort(sorted_horses_.begin(), sorted_horses_.end(), [](Horse* a, Horse* b) {
		// 단순한 위치만으로 말들의 위치 비교하기
		if (a->GetHorseRaceData().position != b->GetHorseRaceData().position) {
			return a->GetHorseRaceData().position > b->GetHorseRaceData().position;
		}
		// 위치가 똑같을 수 있을 때, 억울한 준우승을 방지하기 위한 규칙
		return a->GetCurrentSpeed() > b->GetCurrentSpeed();
		});

	// 열심히 정렬한 순위를 말들에게 전해준다
	for (size_t i = 0; i < sorted_horses_.size(); i++) {
		// 0번째 인덱스가 1등이므로 i + 1을 해준다
		sorted_horses_[i]->SetRank(static_cast<int>(i + 1));

	}
}

// 로그 출력 함수
void horseracing::LineTrack::AddRaceLog(const std::string& msg) {
	raceLogs_.push_back(msg);
	if (raceLogs_.size() > max_log_count_) {
		raceLogs_.erase(raceLogs_.begin());		// 오래된 메시지를 삭제한다
	}
}
