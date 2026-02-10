#include "LineTrack.h"
#include "Race/RaceOrganizer.h"
#include "Render/Renderer.h"
#include "Math/Vector2.h"
#include <algorithm>
#include <iostream>
#include <Windows.h>



// 가로를 분할하기 위한 값
int scoreboard_x = 27;



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

	// 트랙의 너비
	const int TRACK_START = 2;
	const int TRACK_END = width / 2;

	// UI는 무조건 트랙이 끝나고 4칸 뒤에 시작해서 20칸만 차지한다
	const int UI_START = TRACK_START + TRACK_END + 2;
	const int UI_END = UI_START + 20;

	// LOG는 UI가 끝나고 4칸 뒤에 시작한다
	const int LOG_START = UI_END + 4;
	const int DRAW_Y_START = 3;

	// 레퍼런스 가져오기
	Mint::Renderer& renderer = Mint::Renderer::Get();

	// 트랙 그리기 (track_col_end_x까지만 그린다)
	for (int i = 0; i <= 8; ++i) { // 각 레인의 y 좌표
		int y = DRAW_Y_START + (i * 2);
		if (y >= height) break;
	
		for (int x = 0; x < TRACK_END; ++x) {
			int draw_x = TRACK_START + x;
			int idx = (y * width) + draw_x;

			if (idx >= width * height) continue;
			// 출발선 및 트랙 컬럼의 끝
			if (x == 0 || x == TRACK_END - 1) {
				backBuffer[idx].Char.UnicodeChar = L'|';
				backBuffer[idx].Attributes = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
			}
			else {
				backBuffer[idx].Char.UnicodeChar = L'-';
				backBuffer[idx].Attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			}
		}
	}

	// 말 그리기 (말은 트랙 영역 내에서만 그린다)
	for (auto horse : horses_) {
		const auto& data = horse->GetHorseRaceData();

		// 1. 위치 계산 (정수형 인덱스로 변환)
		// 말의 초기 위치를 트랙 컬럼 너비에 맞춰 매핑
		int x = TRACK_START + (int)(data.position * (TRACK_END - 1));
		int y = DRAW_Y_START + ((data.lane_index - 1) * 2) + 1;

		// 2. 화면 범위 밖으로 나가는지 체크 (방어 코드!, track_col_end_x 사용)
		if (x < 0 || x >= TRACK_END || y >= 0 || y < height) {
			// 3. [핵심] 버퍼에 직접 기록!
			int index = y * width + x;
			backBuffer[index].Char.UnicodeChar = L'M';
			backBuffer[index].Attributes = FOREGROUND_RED | FOREGROUND_INTENSITY; // 밝은 빨강
		}
	}

	// UI 출력하기 (점수판)
	// 실제 1등 말을 찾기 위해 전체 말들을 확인한다.
	horseracing::Horse* actualLeadingHorse = nullptr;
	float max_pos = -1.0f;
	for (auto horse_ptr : horses_) { // 원본 horses_ 벡터를 사용한다.
		if (horse_ptr->GetHorseRaceData().position > max_pos) {
			max_pos = horse_ptr->GetHorseRaceData().position;
			actualLeadingHorse = horse_ptr;
		}
	}

	for (int i = 0; i < horses_.size(); ++i) {
		auto horse = horses_[i];
		int rank = horse->GetHorseRaceData().current_rank;
		int lane_index = horse->GetHorseRaceData().lane_index;
		std::wstring name = horse->GetName();
		const wchar_t* medal = GetMedalEmoji(rank);		// 순위에 따라 메달 지급!

		// 순위 표시
		// 'i + 1' 대신 말 객체에서 현재 순위를 가져온다.
		std::wstring rank_text = std::wstring(medal) + L" " + std::to_wstring(lane_index) + L". " + name; // current_rank 유지
		WORD color = (horse == actualLeadingHorse) ? 
			(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY) : 
			(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);


         int y = DRAW_Y_START + (i * 2) + 1;
         int current_ui_x = UI_START;

		 for (wchar_t ch : rank_text) {
			 int char_width = (ch > 127) ? 2 : 1;

			 if (current_ui_x + char_width >= LOG_START - 1 || current_ui_x + char_width >= width) break;

			 if (y < height) {
				 int idx = y * width + current_ui_x;

				 if (char_width == 2) {
					 // 1. 첫 번째 칸 (Leading)
					 backBuffer[idx].Char.UnicodeChar = ch;
					 backBuffer[idx].Attributes = color | COMMON_LVB_LEADING_BYTE;

					 // 2. 두 번째 칸 (Trailing) - 데이터를 복사하되 속성만 다르게
					 if (current_ui_x + 1 < width) {
						 backBuffer[idx + 1].Char.UnicodeChar = ch; // 같은 글자를 넣어야 함
						 backBuffer[idx + 1].Attributes = color | COMMON_LVB_TRAILING_BYTE;
					 }
				 }
				 else {
					 // 일반 영문/숫자
					 backBuffer[idx].Char.UnicodeChar = ch;
					 backBuffer[idx].Attributes = color;
				 }
			 }
			 current_ui_x += char_width;
		 }
	}

	// 로그 영역 설정 (최대 7줄 제한)
	// Todo: 그럴 일은 없겠지만, 말들이 8라인을 넘게 달려선다면. 지금 처럼 수동이 아닌
	// (말 + 1) * 2를 한 값을 적용해야 할 것이다
	const int MAX_LOG_LINES = 16;
	const int LOG_Y_END = DRAW_Y_START + MAX_LOG_LINES - 1;		// 로그의 Y좌표 맥시멈

	// 최신 7개 로그만 가져오기 위한 인덱스 계산
	int num_logs = static_cast<int>(raceLogs_.size());
	int display_count = (num_logs > MAX_LOG_LINES) ? MAX_LOG_LINES : num_logs;
	
	// 1. 로그 영역 청소 (이전 프레임 잔상 제거)
	for (int row = 0; row < MAX_LOG_LINES; ++row) {
		int y = DRAW_Y_START + row;
		for (int x = LOG_START; x < width - 1; ++x) {
			int idx = y * width + x;
			if (idx < width * height) {
				backBuffer[idx].Char.UnicodeChar = L' ';
				backBuffer[idx].Attributes = 0;
			}
		}
	}

	// 일반적인 채팅창처럼 로그를 아래에서 위로 올려보내기
	for (int i = 0; i < display_count; ++i) {
		// 가장 최신의 로그 인덱스는 num_logs - 1이다
		const std::wstring& msg = raceLogs_[num_logs - 1 - i];
		
		// y좌표 계산하기
		int y = LOG_Y_END - i;



		// 2. 로그 출력 및 말줄임표 처리
		int current_log_x = LOG_START;
		const int TRUNCATE_LIMIT = width - 4; // 화면 오른쪽 끝 경계
		bool is_truncated = false;

		for (wchar_t ch : msg) {
			int char_width = (ch > 127) ? 2 : 1;

			// 다음 글자를 찍었을 때 한계를 넘는지 확인
			if (current_log_x + char_width > TRUNCATE_LIMIT) {
				is_truncated = true;
				break;
			}
{
			int idx = y * width + current_log_x;
			if (char_width == 2) {
				// 한글: Leading/Trailing 속성으로 간격 벌어짐 방지
				backBuffer[idx].Char.UnicodeChar = ch;
				backBuffer[idx].Attributes = FOREGROUND_INTENSITY | COMMON_LVB_LEADING_BYTE;
				backBuffer[idx + 1].Char.UnicodeChar = ch;
				backBuffer[idx + 1].Attributes = FOREGROUND_INTENSITY | COMMON_LVB_TRAILING_BYTE;
			} else {
				// 영문/숫자 처리
				backBuffer[idx].Char.UnicodeChar = ch;
				backBuffer[idx].Attributes = FOREGROUND_INTENSITY;
			}
			current_log_x += char_width;
		}
	}

		// 3. 잘렸다면 끝에 '...' 붙이기
		if (is_truncated) {
			for (int dot = 0; dot < 3; ++dot) {
				int dot_idx = y * width + current_log_x + dot;
				if (dot_idx < width * height) {
					backBuffer[dot_idx].Char.UnicodeChar = L'.';
					backBuffer[dot_idx].Attributes = FOREGROUND_INTENSITY;
				}
			}
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
		if (prev_pos < 0.25f && curr_pos >= 0.25f) {
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말이 1구역을 통과!");
		}

		// 말이 트랙의 절반을 지나친 경우
		if (prev_pos < 0.5f && curr_pos >= 0.5f) {
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말이 반환점을 통과!");
		}

		// 말이 트랙의 절반을 지나친 경우
		if (prev_pos < 0.75f && curr_pos >= 0.75f) {
			AddRaceLog(std::to_wstring(data.lane_index) + L"번 말이 마지막을 향해 질주!");
		}

		// 말이 트랙의 끝에 도달한 경우
		if ((prev_pos < 1.0f && curr_pos >= 1.0f) && (horse_is_finished)) {
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

const wchar_t* horseracing::LineTrack::GetMedalEmoji(int rank) {
	switch (rank) {
		case 1: return L"(G)";
		case 2: return L"(S)";
		case 3: return L"(B)";
		case 4: return L"   ";
		case 5: return L"   ";
		case 6: return L"   ";
		case 7: return L"   ";
		case 8: return L"   ";
	}
}
