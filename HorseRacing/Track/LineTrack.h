#pragma once

#include "Level/Level.h"
#include "Horse/Horse.h"
#include "Timer/cpu_timer.h"
#include "TextLayout.h" // UI 헤더 추가

namespace horseracing {
	class RaceOrganizer;

	class LineTrack : public Mint::Level
	{
		RTTI_DECLARATIONS(LineTrack, Level)

	public:
		enum class RaceState {
			kReady,				// 대기상태
			kRacing,			// 경기 중
			kFinished,			// 경기 종료
			kWaitReset			// 입력 대기
		};

		LineTrack();

		~LineTrack();

		// 이벤트 함수 오버라이드
		// 맵을 그린다
		virtual void Draw(Mint::Renderer& renderer, int width, int height) override;

		virtual void Tick(float deltaTime) override;

		void PrepareNewGame(RaceOrganizer& organizer);
		void Reset();

		Mint::CpuTimer& GetTimer() { return timer_; }

		inline bool ShouldRestart() const { return needs_restart_; }
		inline void ClearRestartFlag() { needs_restart_ = false; }

	private:
		
		// 게임에서 사용할 맵을 로드하는 함수
		void LoadMap(const char* fileName);


		void Update(float deltaTime, double total_time);

		void UpdateRanks();

		void AddRaceLog(const std::wstring& msg);

		void HandleInput();


		Mint::CpuTimer timer_;

		// 말 객체들의 포인터를 벡터에 담기
		std::vector<horseracing::Horse*> horses_;
		
		// 시스템 로그 찍기
		// 예를들어 n번 말이 선두로 달리고 있습니다!
		std::vector<std::wstring> raceLogs_;
		const int max_log_count_ = 20;

		RaceState current_state_ = RaceState::kReady;

		// 맵의 구간을 나타내는 변수
		float track_width_ = 1.0f;

		// 얕은 복사 !! UpdateRanks를 위한 전용 변수 
		std::vector<Horse*> sorted_horses_;

		// 경기종료 판단을 위한 변수
		float checking_position_ = 0.0f;

		// UpdateRanks에 관련된 변수
		int checking_ranking_ = 0;

		// 경기 종료 판단을 위한 변수
		int finished_horse_count_ = 0;
		
		// 경기 종료를 위한 플래그
		bool is_race_over_ = false;

		// 재시작을 위한 플래그
		bool needs_restart_ = false;

		bool is_racing_started_ = false;
		int selected_horse_idx_ = 0;
	};
}


