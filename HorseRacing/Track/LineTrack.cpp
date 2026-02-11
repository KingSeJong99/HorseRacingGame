#include "Track/LineTrack.h"
#include "Race/RaceOrganizer.h"
#include "Render/Renderer.h"
#include "Math/Vector2.h"
#include "Timer/cpu_timer.h"
#include <algorithm>
#include <iostream>
#include <Windows.h>



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

void horseracing::LineTrack::Draw(CHAR_INFO* backBuffer, int width, int height) {

	// Draw 함수 내에 배치하면 동적으로 화면이 바뀔 때 반응이 가능하다
	RenderLayout render_layout;

	render_layout.width = width;
	render_layout.height = height;

	// 트랙의 너비
	render_layout.track_start_x = 2;
	render_layout.track_end_x = width / 2;

	// UI는 무조건 트랙이 끝나고 4칸 뒤에 시작해서 20칸만 차지한다
	render_layout.ui_start_x = render_layout.track_start_x + render_layout.track_end_x + 2;
	render_layout.ui_end_x = render_layout.ui_start_x + 20;

	// LOG는 UI가 끝나고 4칸 뒤에 시작한다
	render_layout.log_start_x = render_layout.ui_end_x + 4;
	render_layout.draw_y_start = 3;

	// 레퍼런스 가져오기
	Mint::Renderer& renderer = Mint::Renderer::Get();

	if (!is_racing_started_) {
		DrawBettingMenu(backBuffer, render_layout);

	} else {
		DrawTrack(backBuffer, width, height, render_layout);
		DrawHorses(backBuffer, width, height, render_layout);
		DrawRankUI(backBuffer, width, height, render_layout);
		DrawRaceLogs(backBuffer, width, height, render_layout);
	}
	if (is_race_over_) {
		DrawScoreboard(backBuffer, render_layout);
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

void horseracing::LineTrack::DrawTrack(CHAR_INFO* backBuffer, int width, int height, RenderLayout layout) {
	// 트랙 그리기 (track_col_end_x까지만 그린다)
	for (int i = 0; i <= 8; ++i) { // 각 레인의 y 좌표
		int y = layout.draw_y_start + (i * 2);
		if (y >= height) break;

		for (int x = 0; x < layout.track_end_x; ++x) {
			int draw_x = layout.track_start_x + x;
			int idx = (y * width) + draw_x;

			if (idx >= width * height) continue;
			// 출발선 및 트랙 컬럼의 끝
			if (x == 0 || x == layout.track_end_x - 1) {
				backBuffer[idx].Char.UnicodeChar = L'|';
				backBuffer[idx].Attributes = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
			}
			else {
				backBuffer[idx].Char.UnicodeChar = L'-';
				backBuffer[idx].Attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			}
		}
	}
}

void horseracing::LineTrack::DrawHorses(CHAR_INFO* backBuffer, int width, int height, RenderLayout layout) {
	// 말 그리기 (말은 트랙 영역 내에서만 그린다)
	for (auto horse : horses_) {
		const auto& data = horse->GetHorseRaceData();

		// 1. 위치 계산 (정수형 인덱스로 변환)
		// 말의 초기 위치를 트랙 컬럼 너비에 맞춰 매핑
		int x = layout.track_start_x + (int)(data.position * (layout.track_end_x - 1));
		int y = layout.draw_y_start + ((data.lane_index - 1) * 2) + 1;

		// 2. 화면 범위 밖으로 나가는지 체크 (방어 코드!, track_col_end_x 사용)
		if (x < 0 || x >= layout.track_end_x || y >= 0 || y < height) {
			// 3. [핵심] 버퍼에 직접 기록!
			int index = y * width + x;
			backBuffer[index].Char.UnicodeChar = L'M';
			backBuffer[index].Attributes = FOREGROUND_RED | FOREGROUND_INTENSITY; // 밝은 빨강
		}
	}
}

void horseracing::LineTrack::DrawRankUI(CHAR_INFO* backBuffer, int width, int height, RenderLayout layout) {
	// UI 출력하기 (점수판)
	// 실제 1등 말을 찾기 위해 전체 말들을 확인한다.
	horseracing::Horse* actual_leading_horse = sorted_horses_.empty() ? nullptr : sorted_horses_[0];
	// horseracing::Horse* actual_leading_horse = nullptr;
	// float max_pos = -1.0f;
	// for (auto horse_ptr : horses_) { // 원본 horses_ 벡터를 사용한다.
	// 	if (horse_ptr->GetHorseRaceData().position > max_pos) {
	// 		max_pos = horse_ptr->GetHorseRaceData().position;
	// 		actual_leading_horse = horse_ptr;
	// 	}
	// }

	for (int i = 0; i < horses_.size(); ++i) {
		auto horse = horses_[i];
		int rank = horse->GetHorseRaceData().current_rank;
		int lane_index = horse->GetHorseRaceData().lane_index;
		std::wstring name = horse->GetName();
		const wchar_t* medal = GetMedalEmoji(rank);		// 순위에 따라 메달 지급!

		// 순위 표시
		// 'i + 1' 대신 말 객체에서 현재 순위를 가져온다.
		std::wstring rank_text = std::wstring(medal) + L" " + std::to_wstring(lane_index) + L". " + name; // current_rank 유지
		WORD color = (horse == actual_leading_horse) ?
			(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY) :
			(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

		int y = layout.draw_y_start + (i * 2) + 1;
		int current_ui_x = layout.ui_start_x;

		for (wchar_t ch : rank_text) {
			int char_width = (ch > 127) ? 2 : 1;

			if (current_ui_x + char_width >= layout.log_start_x - 1 || current_ui_x + char_width >= width) break;

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
}

void horseracing::LineTrack::DrawRaceLogs(CHAR_INFO* backBuffer, int width, int height, RenderLayout layout) {
	// Todo: 그럴 일은 없겠지만, 말들이 8라인을 넘게 달려선다면. 지금 처럼 수동이 아닌
	// (말 + 1) * 2를 한 값을 적용해야 할 것이다
	const int MAX_LOG_LINES = 16;
	const int LOG_Y_END = layout.draw_y_start + MAX_LOG_LINES - 1;		// 로그의 Y좌표 맥시멈

	// 최신 로그만 가져오기 위한 인덱스 계산
	int num_logs = static_cast<int>(raceLogs_.size());
	int display_count = (num_logs > MAX_LOG_LINES) ? MAX_LOG_LINES : num_logs;

	// 1. 로그 영역 청소 (이전 프레임 잔상 제거)
	for (int row = 0; row < MAX_LOG_LINES; ++row) {
		int y = layout.draw_y_start + row;
		for (int x = layout.log_start_x; x < width - 1; ++x) {
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
		int current_log_x = layout.log_start_x;
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
				}
				else {
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

void horseracing::LineTrack::DrawScoreboard(CHAR_INFO* backBuffer, const RenderLayout& layout) {
	// 52 * 2
	const int kScoreBoardWidth = 104;
	const int kScoreBoardHeight = 5 + static_cast<int>(sorted_horses_.size());

	// 전광판은 중앙에 설치할 것이기 때문에 중앙의 좌표를 구한다
	int sx = ((layout.width - kScoreBoardWidth) / 2) + 6;
	int sy = (layout.height - kScoreBoardHeight) / 2;

	// 전광판 청소하기
	for (int y = 0; y < kScoreBoardHeight; ++y) {
		for (int x = 0; x < kScoreBoardWidth; ++x) {
			int draw_x = sx + x;
			int draw_y = sy + y;

			// 버퍼 범위를 벗어나지 않도록 방어 코드를 작성합니다.
			if (draw_x >= 0 && draw_x < layout.width && draw_y >= 0 && draw_y < layout.height) {
				int idx = draw_y * layout.width + draw_x;
				backBuffer[idx].Char.UnicodeChar = L' ';
				backBuffer[idx].Attributes = 0;
			}
		}
	}

	// 2. 테두리 및 헤더 그리기
	// 
	// swprintf_s를 사용하여 문자열을 미리 만들음
	DrawTextToBuffer(backBuffer, layout, sx, sy, L"┌──────────────────────────────────────────────────┐", 0x0007);
	DrawTextToBuffer(backBuffer, layout, sx, sy + 1, L"│                [ FINAL RESULTS ]                 │", 0x0007);
	DrawTextToBuffer(backBuffer, layout, sx, sy + 2, L"├──────┬────────────────────┬───────────┬──────────┤", 0x0007);
	DrawTextToBuffer(backBuffer, layout, sx, sy + 3, L"│ RANK │     HORSE NAME     │   TIME    │  REMARK  │", 0x0007);
	DrawTextToBuffer(backBuffer, layout, sx, sy + 4, L"├──────┼────────────────────┼───────────┼──────────┤", 0x0007);

	// 3. 순위 데이터 출력 (sorted_horses_ 기준)
	for (int i = 0; i < sorted_horses_.size(); ++i) {
		auto horse = sorted_horses_[i];
		const auto& data = horse->GetHorseRaceData();
		std::wstring paddedName = PadRight(horse->GetName(), 18);			// 유니코드 기준 9칸까지 할당

		wchar_t row[256];
		swprintf_s(row, L"│  %d   │ %s │  %6.2fs  │          │",
			i + 1, paddedName.c_str(), data.finish_time);

		// 1등은 노란색(금색), 나머지는 흰색
		WORD color = (i == 0) ? (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY) : (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		DrawTextToBuffer(backBuffer, layout, sx, sy + 5 + i, row, color);
	}

	DrawTextToBuffer(backBuffer, layout, sx, sy + kScoreBoardHeight - 1, L"└──────┴────────────────────┴───────────┴──────────┘", 0x0007);
}

void horseracing::LineTrack::DrawTextToBuffer(CHAR_INFO* buffer, const RenderLayout& layout, int x, int y, const std::wstring& text, WORD color) {
	int current_x = x;
	for (wchar_t ch : text) {
		int width = (ch > 127) ? 2 : 1;

		// 글자가 화면을 넘어가지 못하도록 한다
		if (current_x + width > layout.width) break;

		int idx = y * layout.width + current_x;
		if (width == 2) {
			buffer[idx].Char.UnicodeChar = ch;
			buffer[idx].Attributes = color | COMMON_LVB_LEADING_BYTE;

			// trailing 바이트를 채우기 위한 체크
			if (current_x + 1 < layout.width) {
				buffer[idx + 1].Char.UnicodeChar = ch;
				buffer[idx + 1].Attributes = color | COMMON_LVB_TRAILING_BYTE;
			}
		}
		else {
			buffer[idx].Char.UnicodeChar = ch;
			buffer[idx].Attributes = color;
		}
		current_x += width;
	}
}

void horseracing::LineTrack::DrawBettingMenu(CHAR_INFO* backBuffer, const RenderLayout& layout) {
	const int kMenuWidth = 104; // 전광판과 동일한 값으로 일관성을 맞춤.
	int sx = (layout.width - kMenuWidth) / 2;
	int sy = (layout.height - 15) / 2; // 중앙 배치를 위한 Y 좌표

	// 1. 메뉴 제목 및 헤더 그리기
	DrawTextToBuffer(backBuffer, layout, sx, sy, L"┌──────────────────────────────────────────────────┐", 0x000F);
	DrawTextToBuffer(backBuffer, layout, sx, sy + 1, L"│              [ BETTING : SELECT YOUR HORSE ]     │", 0x000F);
	DrawTextToBuffer(backBuffer, layout, sx, sy + 2, L"├──────┬──────────────────┬──────┬──────┬──────────┤", 0x000F);
	DrawTextToBuffer(backBuffer, layout, sx, sy + 3, L"│ NO.  │    HORSE NAME    │ SPD  │ STM  │  TRAIT   │", 0x000F);
	DrawTextToBuffer(backBuffer, layout, sx, sy + 4, L"├──────┼──────────────────┼──────┼──────┼──────────┤", 0x000F);

	// 2. 8마리의 스탯 리스트 출력
	for (int i = 0; i < 8; ++i) {
		auto horse = horses_[i];
		std::wstring name_str = PadRight(horse->GetName(), 16);
		std::wstring spd_str = PadRight(std::to_wstring(static_cast<int>(horse->GetMax_speed())), 4);
		std::wstring stm_str = PadRight(std::to_wstring(static_cast<int>(horse->GetStamina())), 4);
		std::wstring trait_str = PadRight(L"None", 8);

		// 선택된 말은 노란색(또는 배경색 반전)으로 강조
		WORD color = (i == selected_horse_idx_) ? 0x000E : 0x0007;

		wchar_t row[256];
		// 포맷팅, ASCII 코드를 제외한 非ASCII 코드는 폭을 인식 못하기 때문에
		// 문자열은 %S로 제한 출력한다
		swprintf_s(row, L"│  %d   │ %s │ %s │ %s │ %s │",
			i + 1, name_str.c_str(),
			spd_str.c_str(), stm_str.c_str(), trait_str.c_str()); // 특징 데이터 연결

		DrawTextToBuffer(backBuffer, layout, sx, sy + 5 + i, row, color);
	}
	DrawTextToBuffer(backBuffer, layout, sx, sy + 13, L"└──────┴──────────────────┴──────┴──────┴──────────┘", 0x000F);
}

int horseracing::LineTrack::GetVisualWidth(const std::wstring& text) {
	int width = 0;
	for (wchar_t ch : text) {
		// 문자열의 폭을 아스키 코드와 非아스키코드로 구분하여 구한다
		width += (ch > 127) ? 2 : 1;	
	}
	return width;
}

std::wstring horseracing::LineTrack::PadRight(std::wstring text, int targetWidth) {
	int currentWidth = GetVisualWidth(text);

	// 채워져야 할 값 = 유니코드 - 아스키 코드의 차이 만큼
	int paddingNeeded = targetWidth - currentWidth;

	if (paddingNeeded > 0) {
		// 부족한 칸 수 만큼 공백을 추가한다
		text.append(paddingNeeded, L' ');
	}
	return text;
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

