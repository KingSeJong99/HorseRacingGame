#ifndef HORSERACING_HORSE_HORSEFACTORY_H_
#define HORSERACING_HORSE_HORSEFACTORY_H_

#include "Horse/Horse.h"

namespace horseracing {
	// 공장 패턴, 생상만 하므로 소멸은 다른 곳에서 진행된다.
	class HorseFactory
	{
	public:
		// 데이터를 받아서 Horse 객체를 생성한 후 반환한다
		// Factory는 기능만 있으면 되므로 static 함수를 사용한다
		static Horse* CreateHorse(const HorseStats& stats, const std::string& name);
	};
}
#endif HORSERACING_HORSE_HORSEFACTORY_H_

