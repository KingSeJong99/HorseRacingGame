#ifndef HORSERACING_RACE_RACEORGANIZER_H_
#define HORSERACING_RACE_RACEORGANIZER_H_

#include "Level/Level.h"
#include "Horse/Horse.h"


namespace horseracing {
	class Horse;

	class RaceOrganizer
	{
	public:
	struct HorseData {
		std::string name;
		int horse_id;
		HorseStats base_stats;
		int total_wins;
	};
		// 전체 50마리의 데이터를 로드한다
		void LoadAllHorseData(const std::string& file_name);

		// 경주에 나갈 말을 랜덤으로 뽑아준다!
		// Todo: std::shuffle 같은 알고리즘을 활용한다
		std::vector<horseracing::Horse*> OrganizeRace(int entryCount);

	private:
		std::vector<HorseData> all_horses_pool_;
	};
}
#endif HORSERACING_RACE_RACEORGANIZER_H_

