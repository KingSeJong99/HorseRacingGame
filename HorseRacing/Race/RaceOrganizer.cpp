#include "RaceOrganizer.h"
#include "Horse/HorseFactory.h"
#include <algorithm>
#include <random>
#include <chrono>

horseracing::RaceOrganizer::RaceOrganizer()
{
}

horseracing::RaceOrganizer::~RaceOrganizer()
{
}

// 파일 불러와서 말 입력하기
void horseracing::RaceOrganizer::LoadAllHorseData(const std::wstring& file_name)
{
	all_horses_pool_.reserve(50);
	// Todo: 파일 입력을 받아 말들의 정보를 로드해야함
	// all_horses_pool_.push_back({ "번개", {10.0f, 0.5f, 100.0f} });
	// all_horses_pool_.push_back({ "적토마", {12.0f, 0.4f, 80.0f} });
	// all_horses_pool_.push_back({ "거북이", {5.0f, 0.1f, 200.0f} });

	all_horses_pool_.push_back({ L"심볼리루돌프", {12.5f, 0.6f, 110.0f} });
	all_horses_pool_.push_back({ L"딥임팩트", {13.0f, 0.7f, 95.0f} });
	all_horses_pool_.push_back({ L"보드카", {11.8f, 0.5f, 105.0f} });
	all_horses_pool_.push_back({ L"다이와스칼렛", {12.0f, 0.55f, 100.0f} });
	all_horses_pool_.push_back({ L"골드쉽", {11.0f, 0.9f, 150.0f} }); // 스테미너 특화
	all_horses_pool_.push_back({ L"오구리캡", {11.5f, 0.65f, 120.0f} });
	all_horses_pool_.push_back({ L"라이스샤워", {10.5f, 0.4f, 130.0f} });
	all_horses_pool_.push_back({ L"하루우라라", {8.0f, 0.2f, 90.0f} }); // 깜찍한 능력치
	all_horses_pool_.push_back({ L"세이운스카이", {11.2f, 0.5f, 115.0f} });
	all_horses_pool_.push_back({ L"킹헤일로", {11.7f, 0.45f, 100.0f} });

	all_horses_pool_.push_back({ L"엘콘도르파사", {12.2f, 0.6f, 110.0f} });
	all_horses_pool_.push_back({ L"그래스원더", {11.9f, 0.55f, 115.0f} });
	all_horses_pool_.push_back({ L"스페셜위크", {12.1f, 0.5f, 105.0f} });
	all_horses_pool_.push_back({ L"메지로맥퀸", {10.8f, 0.4f, 140.0f} });
	all_horses_pool_.push_back({ L"토우카이테이오", {12.8f, 0.5f, 90.0f} });
	all_horses_pool_.push_back({ L"나리타브라이언", {13.2f, 0.7f, 100.0f} });
	all_horses_pool_.push_back({ L"에어그루브", {11.6f, 0.55f, 110.0f} });
	all_horses_pool_.push_back({ L"사일런스스즈카", {14.0f, 0.3f, 85.0f} }); // 초반 속도 특화
	all_horses_pool_.push_back({ L"미스터씨비", {12.0f, 0.6f, 110.0f} });
	all_horses_pool_.push_back({ L"비와하야히데", {11.4f, 0.5f, 120.0f} });

	all_horses_pool_.push_back({ L"박신오", {14.5f, 0.2f, 70.0f} }); // 단거리 폭주형
	all_horses_pool_.push_back({ L"스마트팔콘", {12.3f, 0.5f, 100.0f} });
	all_horses_pool_.push_back({ L"아그네스타키온", {12.7f, 0.6f, 95.0f} });
	all_horses_pool_.push_back({ L"맨하탄카페", {10.9f, 0.45f, 135.0f} });
	all_horses_pool_.push_back({ L"정글포켓", {11.8f, 0.55f, 110.0f} });
	all_horses_pool_.push_back({ L"티엠오페라오", {12.0f, 0.65f, 125.0f} });
	all_horses_pool_.push_back({ L"어드마이어베가", {12.4f, 0.5f, 95.0f} });
	all_horses_pool_.push_back({ L"나리타타이신", {12.6f, 0.45f, 90.0f} });
	all_horses_pool_.push_back({ L"위닝티켓", {11.5f, 0.5f, 110.0f} });
	all_horses_pool_.push_back({ L"마야노탑건", {11.2f, 0.7f, 130.0f} });

	all_horses_pool_.push_back({ L"흑풍", {13.5f, 0.4f, 100.0f} });
	all_horses_pool_.push_back({ L"백야", {10.5f, 0.3f, 140.0f} });
	all_horses_pool_.push_back({ L"질풍지각", {15.0f, 0.1f, 60.0f} });
	all_horses_pool_.push_back({ L"철갑기사", {9.0f, 0.8f, 180.0f} });
	all_horses_pool_.push_back({ L"캐논슈터", {13.8f, 0.35f, 85.0f} });
	all_horses_pool_.push_back({ L"황금날개", {12.0f, 0.5f, 105.0f} });
	all_horses_pool_.push_back({ L"새벽의함성", {11.5f, 0.45f, 115.0f} });
	all_horses_pool_.push_back({ L"밤의장막", {10.7f, 0.4f, 130.0f} });
	all_horses_pool_.push_back({ L"번개발구름", {13.2f, 0.55f, 95.0f} });
	all_horses_pool_.push_back({ L"바람의아들", {14.2f, 0.25f, 75.0f} });

	all_horses_pool_.push_back({ L"천하장사", {9.5f, 0.85f, 170.0f} });
	all_horses_pool_.push_back({ L"그림자술사", {11.3f, 0.5f, 110.0f} });
	all_horses_pool_.push_back({ L"광속질주", {14.8f, 0.15f, 65.0f} });
	all_horses_pool_.push_back({ L"대지의심장", {10.2f, 0.6f, 150.0f} });
	all_horses_pool_.push_back({ L"푸른섬광", {12.9f, 0.45f, 95.0f} });
	all_horses_pool_.push_back({ L"루비아이", {11.7f, 0.5f, 110.0f} });
	all_horses_pool_.push_back({ L"에메랄드대쉬", {12.1f, 0.55f, 105.0f} });
	all_horses_pool_.push_back({ L"썬더볼트", {13.6f, 0.4f, 80.0f} });
	all_horses_pool_.push_back({ L"미라클런", {11.0f, 0.75f, 145.0f} });
	all_horses_pool_.push_back({ L"끝판왕", {12.0f, 0.5f, 120.0f} });
}

// 알고리즘 헤더를 활용해 말들의 순서를 뒤섞는다
std::vector<horseracing::Horse*> horseracing::RaceOrganizer::OrganizeRace(int entryCount)
{
	// 원본 복사하기
	std::vector<HorseData> temp_pool = all_horses_pool_;

	// 시드 생성
	unsigned seed =
		std::chrono::system_clock::now().time_since_epoch().count();

	// 말 섞기
	std::shuffle(temp_pool.begin(), 
		temp_pool.end(), 
		std::default_random_engine(static_cast<unsigned int>(seed)));

	// 선발된 말 생성하기
	std::vector<Horse*> racing_horses;
	for (int i = 0; i < entryCount && i < temp_pool.size(); ++i) {
		Horse* newHorse = HorseFactory::CreateHorse(temp_pool[i].base_stats, temp_pool[i].name);
		racing_horses.push_back(newHorse);
	}

	return racing_horses;
}
