#include"cpu_timer.h"

Mint::CpuTimer::CpuTimer() : delta_time_(0.0)			
{
	QueryPerformanceFrequency(&frequency_);				// 하드웨어의 주파수를 얻는다
	QueryPerformanceCounter(&current_time_);			// 첫 시작을 위해 현재 값을 생성과 동시에 초기화한다
	previous_time_ = current_time_;						// 첫 델타타임이 튀는 것을 방지하기 위한 상태전이

	base_time_ = current_time_;
}

Mint::CpuTimer::~CpuTimer()
{
}

void Mint::CpuTimer::Tick()
{
	previous_time_ = current_time_;						// 이전에 관측된 cpu 시간을 이전 값에 전이한다

	QueryPerformanceCounter(&current_time_);			// 현재 cpu 시간을 갱신한다

	delta_time_ = double(current_time_.QuadPart - previous_time_.QuadPart) / frequency_.QuadPart;	// 델타타임 계산
}

void Mint::CpuTimer::Reset() {
	QueryPerformanceCounter(&base_time_);
}

double Mint::CpuTimer::GetTotalTime() const
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	return double(now.QuadPart - base_time_.QuadPart) / frequency_.QuadPart;
}

