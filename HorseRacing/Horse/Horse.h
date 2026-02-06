#ifndef HORSERACING_HORCE_HORSE_H_
#define HORSERACING_HORCE_HORSE_H_

#include "Actor/Actor.h"
#include <string>


namespace horseracing {

	// 경주마에 관련된 정보
	struct HorseStats {
		float max_speed = 0;
		float acceleration = 0;
		float stamina = 0;
	};

	class Horse : public Mint::Actor
	{
	public:
		struct HorseRaceData {
			// 일자 트랙에서만 움직이므로 y좌표는 고정, x좌표만 변경된다
			float position = 0;
			int lane_index;
			int current_rank;
			bool is_finished;
		};
		Horse(const HorseStats& stats, std::string name);
		~Horse() override;

		void Run(float delta_time, float track_width);

		// 말의 정보 넘기기
		inline const struct HorseRaceData& GetHorseRaceData() const { return race_data_; }
		inline float GetCurrentSpeed() const { return current_speed_; }

		// 말의 순위 변동 함수
		void SetRank(int changed_rank);
		void SetLane(int lane_index);
		void Reset();

	private:

		std::string name_;
		HorseStats stats_;
		HorseRaceData race_data_;

		float current_speed_ = 0.0f;
		bool is_racing_ = false;

		// 말의 달리기 기능
		void CalculatePhysics(float delta_time, float track_width);



	};
}

#endif // HORSERACING_HORSE_HORSE_H_

