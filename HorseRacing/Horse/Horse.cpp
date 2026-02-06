#include "Horse.h"

// 이미 저장되어 있는 이름을 덮어쓰기보다 가져온다!
// Todo: 생성자 변경으로 인한 로직 수정 필요함
horseracing::Horse::Horse(const HorseStats& stats, std::string name)
	: name_(std::move(name)),
	current_speed_(0.0f),
	is_racing_(false) {

}

// STL 사용에 따른 디폴트 소멸자 사용
horseracing::Horse::~Horse() = default;

void horseracing::Horse::Run(float delta_time, float track_width) {
	if (!is_racing_) return;

	CalculatePhysics(delta_time, track_width);
}

void horseracing::Horse::SetRank(int changed_rank)
{
	race_data_.current_rank = changed_rank;
}

// 말의 상태를 초기화해 경기에 참가할 수 있도록 한다
void horseracing::Horse::Reset()
{
	current_speed_ = 0.0f;
	is_racing_ = true;

	race_data_.current_rank = -1;
	race_data_.position = 0;
	race_data_.is_finished = false;
}

void horseracing::Horse::CalculatePhysics(float delta_time, float track_width) {
	// HorseStats 등을 활용해 current_speed를 변화시킨다
	current_speed_ += stats_.acceleration * delta_time;

	// 말의 폭주 억제
	if (current_speed_ > stats_.max_speed) {
		current_speed_ = stats_.max_speed;
	}

	// 위치 업데이트( 0.0 ~ 1.0으로 정규화 하기 위해 트랙 길이로 나눔)
	float distance_moved = current_speed_ * delta_time;
	race_data_.position += distance_moved / track_width;
	
	// 결승선 통과 처리
	if (race_data_.position >= 1.0f) {
		race_data_.position = 1.0f;
		race_data_.is_finished = true;
		is_racing_ = false;
	}
}
