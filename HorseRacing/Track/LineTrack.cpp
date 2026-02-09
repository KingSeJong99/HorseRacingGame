#include "LineTrack.h"
#include "Race/RaceOrganizer.h"
#include "Render/Renderer.h"
#include "Math/Vector2.h"
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

void horseracing::LineTrack::Draw(CHAR_INFO* backBuffer, int width, int height) {

	// Hack: 디버깅, 그리기가 잘 되는지 확인용
	// backBuffer[0].Char.AsciiChar = '!';
	// backBuffer[0].Attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	
	// 화면 구역 나누기(경기장과 UI)
	int ui_divider_x = (int)(width * 0.7f);

	// 레퍼런스 가져오기
	Mint::Renderer& renderer = Mint::Renderer::Get();

	for (int i = 0; i <= 8; ++i) {
		int y = i * 2 + 1;		// 각 레인의 y 좌표
		if (y >= height) break;
	
		// 정해진 구역까지만 그리세여
		for (int x = 0; x < ui_divider_x; ++x) {
			int idx = (y * width) + x;
		
			// 출발선 
			if (x == 0) {
				backBuffer[idx].Char.UnicodeChar = L'|';
				backBuffer[idx].Attributes = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
			}
			else if (x == width - 1) {
				backBuffer[idx].Char.UnicodeChar = L'|';
				backBuffer[idx].Attributes = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
			}
			else {
				backBuffer[idx].Char.UnicodeChar = L'-';
				backBuffer[idx].Attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			}
		}
	}

	// 말 그리기
	for (auto horse : horses_)
	{
		const auto& data = horse->GetHorseRaceData();

		// 1. 위치 계산 (정수형 인덱스로 변환)
		int x = (int)(data.position * (width - 1));
		int y = (int)(data.lane_index * 1) * 2;

		// 2. 화면 범위 밖으로 나가는지 체크 (방어 코드!)
		if (x < 0 || x >= ui_divider_x || y < 0 || y >= height) continue;

		// 3. [핵심] Submit 대신 버퍼에 직접 기록!
		int index = y * width + x;
		backBuffer[index].Char.UnicodeChar = L'M';
		backBuffer[index].Attributes = FOREGROUND_RED | FOREGROUND_INTENSITY; // 밝은 빨강
	}

	// UI 출력하기
	int scoreboard_x = 27;	// Hack: 임시 값.
	for (int i = 0; i < sorted_horses_.size(); ++i) {
		auto horse = sorted_horses_[i];
		std::wstring name = horse->GetName();		

		// 순위 표시
		std::wstring rank_text = std::to_wstring(i + 1) + L". " + name;
		WORD color = (i == 0) ? 
			(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY) : 
			(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);


		for (int char_idx = 0; char_idx < rank_text.length(); ++char_idx) {
			int x = scoreboard_x + char_idx;
			int y = i * 2 + 1;
			
			if (x < width && y < height) {
				int idx = y * width + x;
				backBuffer[idx].Char.UnicodeChar = rank_text[char_idx];
				backBuffer[idx].Attributes = color;
			}
		}
	}

	// 로그 출력하기
	int log_start_y = 18;
	for (int i = 0; i < raceLogs_.size() && i < 2; ++i) {
		const std::wstring& msg = raceLogs_[i];
		for (int x = 0; x < msg.length() && x < width; ++x) {
			int idx = (log_start_y + i) * width + x;
			backBuffer[idx].Char.UnicodeChar = msg[x];
			backBuffer[idx].Attributes = FOREGROUND_INTENSITY;
		}
	}
}

void horseracing::LineTrack::Tick(float deltaTime)
{
	this->Update(deltaTime);
}

void horseracing::LineTrack::LoadMap(const char* file_name) {
	
	track_width_ = 1000.0f;
}

void horseracing::LineTrack::RenderToBuffer(CHAR_INFO* buffer, int width, int height) {

	for (auto horse : horses_) {
		const auto& data = horse->GetHorseRaceData();
		int x = static_cast<int>(data.position * (width - 2));
		int y = data.lane_index * 2;
		int idx = y * width + x;

		if (idx >= 0 && idx < width * height) {
			buffer[idx].Char.UnicodeChar = L'M';
			buffer[idx].Attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		}
	}
}

// 경기에 출전하는 말들을 데려온다
void horseracing::LineTrack::PrepareNewGame(RaceOrganizer& organizer) {
	// 달리고 있던 말들 보내기
	for (auto horse : horses_) {
		delete horse;
	}
	horses_.clear();
	sorted_horses_.clear();

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
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말이 반환점을 통과합니다!");
		}

		// 말이 트랙의 끝에 도달한 경우
		if ((prev_pos < 1.0f && curr_pos >= 1.0f) && (horse_is_finished)) {
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말이 도착합니다!");
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
void horseracing::LineTrack::AddRaceLog(const std::wstring& msg) {
	raceLogs_.push_back(msg);
	if (raceLogs_.size() > max_log_count_) {
		raceLogs_.erase(raceLogs_.begin());		// 오래된 메시지를 삭제한다
	}
}
