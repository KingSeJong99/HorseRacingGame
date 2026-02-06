#include"Engine.h"
#include"Level/Level.h"
#include"Core/Input.h"
#include"Util/Util.h"
#include"Render/Renderer.h"
#include<iostream>
#include<Windows.h>

namespace Mint
{
	Engine* Engine::instance = nullptr;

	Engine::Engine()
	{
		instance = this;

		// 입력 관리자 생성
		input = new Input();

		// 설정 파일 로드
		LoadSetting();

		// 렌더러 객체 생성하기
		renderer = new Renderer(Vector2(setting.width, setting.height));

		// 커서를 끈다
		Util::TurnOffCursor();
	}

	Engine::~Engine()
	{
		// 메인 레벨을 제거한다
		if (mainLevel)
		{
			delete mainLevel;
			mainLevel = nullptr;
		}

		// 입력 관리자 제거
		if (input)
		{
			delete input;
			input = nullptr;
		}

		// 렌더러 객체 제거하기
		SafeDelete(renderer);
	}

	void Engine::Run()
	{
		// cpu의 주파수로 초를 구하기.
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);

		// 프레임 계산용 변수
		int64_t currentTime = 0;
		int64_t previousTime = 0;

		// 하드웨어 타이머로 시간 구하기
		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);

		// 엔진 시작 직전이므로 두 시간 값은 동일하게 초기화되어있다
		currentTime = time.QuadPart;
		previousTime = currentTime;

		// 기준 프레임(단위를 초로)
		// float targetFrameRate = 15.0f;

		setting.framerate
			= setting.framerate == 0.0f ? 60.f : setting.framerate;
		float oneFrameTime = 1.0f / setting.framerate;

		// 엔진 루프(게임루프)
		while (!isQuit)
		{
			// 현재 시간 구하기
			QueryPerformanceCounter(&time);
			currentTime = time.QuadPart;

			// 프레임 시간 계산
			float deltaTime
				= static_cast<float>(currentTime - previousTime);

			deltaTime = deltaTime
				/ static_cast<float>(frequency.QuadPart);

			// 고정 프레임 기법
			if (deltaTime >= oneFrameTime)
			{
				input->ProcessInput();

				// 프레임 처리 !!
				// Tick(1.0f / 60.0f);
				BeginPlay();
				Tick(deltaTime);
				Draw();

				// 이전 시간 값 갱신하기
				previousTime = currentTime;

				input->SavePreviousInputStates();

				// 레벨에 요청된 추가 및 제거를 처리한다
				if (mainLevel)
				{
					mainLevel->ProcessAddAndDestroyActors();
				}
			}
		}

		// 정리
		Shutdown();
	}

	void Engine::QuitEngine()
	{
		isQuit = true;
	}

	void Engine::SetNewLevel(Level* newLevel)
	{
		// 기존의 Level이 있는지 확인한다
		// 있으면 기존의 Level을 제거한다
		// 원래는 전환시 바로 전환하면 안된다
		// TODO : 임시코드
		if (mainLevel)
		{
			delete mainLevel;
			mainLevel = nullptr;
		}

		// 레벨 설정
		mainLevel = newLevel;
	}

	Engine& Engine::Get() 
	{
		// 예외처리
		if (!instance)
		{
			std::cout << "Error: Engine::Get(). instance is null\n";
			__debugbreak;
		}
		return *instance;
	}

	void Engine::Shutdown()
	{
		// 정리 작업
		std::cout << "엔진이 죽고 말았다....\n";

		// 꺼진 커서를 킨다
		Util::TurnOnCursor();
	}

	void Engine::LoadSetting()
	{
		// 엔진을 설정하는 파일 열기
		FILE* file = nullptr;
		fopen_s(&file, "../Config/Setting.txt", "rt");
		
		// 파일을 찾지 못했을 때 예외처리하기
		if (!file)
		{
			std::cout << "Failed to open engine setting file.\n";
			std::cin.get();
			__debugbreak();
			return;
		}

		char buffer[2048] = {};
		
		// 파일에서 읽기.
		size_t readSize = 
			fread(buffer, sizeof(char), 2048, file);
		
		// 문자열 자르기 (파싱)
		// 첫 번째 문자열을 분리할 때는 첫 파라미터를 전달한다
		char* context = nullptr;
		char* token = nullptr;
		token = strtok_s(buffer, "\n", &context);

		// 반복해서 자르기
		while (token)
		{
			// 설정 텍스트에서 파라미터 이름만 읽기
			char header[10] = {};
			// 문자열 읽기 함수 활용하기
			// 이때 "%s"로 읽으면 "스페이스가 있으면 거기까지 읽음"
			sscanf_s(token, "%s", header, 10);

			// 문자열 비교 및 값 읽기
			if (strcmp(header, "framerate") == 0)
			{
				sscanf_s(token, "framerate = %f", &setting.framerate);
			}
			else if (strcmp(header, "width") == 0)
			{
				sscanf_s(token, "width = %d", &setting.width);
			}
			else if (strcmp(header, "height") == 0)
			{
				sscanf_s(token, "height = %d", &setting.height);
			}

			// 개행 문자로 문자열 분리
			token = strtok_s(nullptr, "\n", &context);
		}

		// 문자열 포맷 활용해서 데이터 추출하기
		// sscanf_s(buffer, "framerate = %f", &setting.framerate);
		// 파일을 닫는다
		fclose(file);
	}

	void Engine::BeginPlay()
	{
		// Level이 존재한다면 이벤트를 전달
		if (!mainLevel)
		{
			std::cout << "mainLevel is empty.\n";
			return;
		}
		
		mainLevel->BeginPlay();
	}
	void Engine::Tick(float deltaTime)
	{
		// std::cout
		// 	<< "DeltaTime: " << deltaTime
		// 	<< ", FPS: " << (1.0f / deltaTime) << "\n";

		// Level에 Event 보내기
		// 예외처리임
		if (!mainLevel)
		{
			std::cout << "Error: Engine::Tick(). mainLevel is empty\n";
			return;
		}

		mainLevel->Tick(deltaTime);
	}

	void Engine::Draw()
	{
		// 레벨에 이벤트 흘리기
		// 예외처리하기
		if (!mainLevel)
		{
			std::cout << "Error: Engine::Draw(). mainLevel is empty\n";
			return;
		}
		// 레벨의 모든 액터가 렌더 데이터를 제출한다
		mainLevel->Draw();

		// 렌더러에 그리기 명령을 전달한다
		renderer->Draw();
	}
}