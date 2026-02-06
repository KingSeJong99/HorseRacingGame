#include "HorseRacing.h"
#include <iostream>

// 싱글턴을 위한 메모리 공간 확보
horseracing::HorseRacing* horseracing::HorseRacing::instance_ = nullptr;

horseracing::HorseRacing::HorseRacing() {
	if (instance_ != nullptr) {
		std::cerr << "게임 로드 중 엔진 로딩에 실패했습니다!";
		__debugbreak();
	}
	instance_ = nullptr;
}

horseracing::HorseRacing::~HorseRacing() {
	instance_ = nullptr;
}