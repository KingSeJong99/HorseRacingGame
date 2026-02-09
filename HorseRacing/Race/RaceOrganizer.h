#ifndef HORSERACING_RACE_RACEORGANIZER_H_
#define HORSERACING_RACE_RACEORGANIZER_H_

#include "Level/Level.h"
#include "Horse/Horse.h"

namespace horseracing {
	struct HorseData {
		std::wstring name;
		// 최대속력, 가속도, 스태미너
		HorseStats base_stats;
		int horse_id;
		int total_wins;
		int total_appearances;
	};
	
	class RaceOrganizer
	{
	public:
		RaceOrganizer();
		~RaceOrganizer();

		// 전체 50마리의 데이터를 로드한다
		void LoadAllHorseData(const std::wstring& file_name);

		// 경주에 나갈 말을 랜덤으로 뽑아준다!
		std::vector<horseracing::Horse*> OrganizeRace(int entryCount);

	private:
		// 말 보관소(?)
		std::vector<HorseData> all_horses_pool_;

	};
}
#endif HORSERACING_RACE_RACEORGANIZER_H_

