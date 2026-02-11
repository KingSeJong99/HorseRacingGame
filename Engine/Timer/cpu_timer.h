#pragma once
#include "Common/RTTI.h"

#include<Windows.h>
namespace Mint {
	class MINT_API CpuTimer
	{
	public:
		CpuTimer();
		~CpuTimer();

		void Tick();

		void Reset();

		inline double GetDeltaTime() const { return delta_time_; };

		// 특정 구간으로 부터 흐른 시간을 반환함(기록용)
		double GetTotalTime() const;

	private:

		// 주파수를 저장하는 변수
		LARGE_INTEGER frequency_;

		// delta_time_을 구하기 위한 피연산자1
		LARGE_INTEGER previous_time_;

		// delta_time_을 구하기 위한 피연산자2
		LARGE_INTEGER current_time_;

		// 특정 시작 지점을 저장할 변수
		LARGE_INTEGER base_time_;

		// 두 시간의 차를 저장하는 변수
		double delta_time_;
	};
}


