#include "Renderer.h"
#include "ScreenBuffer.h"
#include "Util/Util.h"

namespace Mint 
{
	Renderer::Frame::Frame(int bufferCount)
	{
		// 배열 생성 및 초기화
		charInfoArray = new CHAR_INFO[bufferCount];
		memset(charInfoArray, 0, sizeof(CHAR_INFO) * bufferCount);
		
		sortingOrderArray = new int[bufferCount];
		memset(sortingOrderArray, 0, sizeof(int) * bufferCount);
	}
 
	Renderer::Frame::~Frame()
	{
		SafeDeleteArray(charInfoArray);
		SafeDeleteArray(sortingOrderArray);
	}
 
	void Renderer::Frame::Clear(const Vector2& screenSize)
	{
		// 2차원 배열로 다루는 1차원 배열을 순회하면서
		// 빈 문자(' ')를 설정한다
		const int width = screenSize.x;
		const int height = screenSize.y;
 
		for (int y = 0; y < height; ++y)
		{
			for (int x= 0; x < width; ++x)
			{
				// 배열 인덱스 구하기
				const int index = (y * width) + x;
				
				// 글자 값 및 속성 설정
				CHAR_INFO& info = charInfoArray[index];
				info.Char.UnicodeChar = L' ';
				info.Attributes = 0;
				
				// 그리기 우선순위 초기화하기
				sortingOrderArray[index] = -1;
			}
		}
	}

	//  =================================프레임

	//  정적 변수 초기화
	Renderer* Renderer::instance = nullptr;

	Renderer::Renderer(const Vector2& screenSize)
		: screenSize(screenSize)
	{
		instance = this;
		
		// 프레임 객체 생성하기
		const int bufferCount = screenSize.x * screenSize.y;
		frame = new Frame(bufferCount);
		
		// 프레임 초기화
		frame->Clear(screenSize);
		
		// 이중 버퍼 객체 생성 및 초기화!
		screenBuffers[0] = new ScreenBuffer(screenSize);
		screenBuffers[0]->Clear();
		
		screenBuffers[1] = new ScreenBuffer(screenSize);
		screenBuffers[1]->Clear();
 
		// 활성화 버퍼 설정하기
		Present();
	}
 
 
	Renderer::~Renderer()
	{
		SafeDelete(frame);
		for(ScreenBuffer*& buffer : screenBuffers)
		{
			SafeDelete(buffer);
		}
	}
 
	void Renderer::Draw()
	{
		// 화면 지우기
		Clear();


		// 렌더큐 순회하면서 그리기
		// 전제조건: 레벨의 모든 액터가 렌더러에 Submit을 완료해야한다
		for (const RenderCommand& command : renderQueue)
		{
			// 화면에 그릴 텍스트가 없으면 건너뜀
			if (!command.text)
			{
				continue;
			}

			// 세로 기준 화면 벗어났는지 확인
			if (command.position.y < 0
				|| command.position.y >= screenSize.y)
			{
				continue;
			}

			// 화면에 그릴 문자열 길이
			const int length = static_cast<int>(strlen(command.text));

			// 안그려도 되면 건너 뛰기
			if (length <= 0)
			{
				continue;
			}

			// x 좌표 기준으로 화면에서 벗어났는지 확인하기
			// 위치는 왼쪽 기준: "abcde"
			const int startX = command.position.x;
			const int endX = command.position.x + length - 1;

			if (endX < 0 || startX >= screenSize.x)
			{
				continue;
			}

			// 시작 위치
			const int visibleStart = startX < 0 ? 0 : startX;
			const int visibleEnd
				= endX >= screenSize.x ? screenSize.x - 1 : endX;

			// 문자열 설정
			for (int x = visibleStart; x <= visibleEnd; ++x)
			{
				// 문자열 인덱스
				const int sourceIndex = x - startX;

				// 프레임 (2차원 문자 배열) 인덱스
				const int index
					= (command.position.y * screenSize.x) + x;

				// 그리기 우선순위 비교
				if (frame->sortingOrderArray[index]
			> command.sortingOrder)
				{
					continue;
				}

				// 데이터 기록하기
				frame->charInfoArray[index].Char.UnicodeChar
					= command.text[sourceIndex];
				frame->charInfoArray[index].Attributes
					= (WORD)command.color;

				// 우선순위 업데이트하기
				frame->sortingOrderArray[index]
					= command.sortingOrder;
			}
		}

		// 그리기.
		GetCurrentBuffer()->Draw(frame->charInfoArray);

		// 버퍼 교환하기
		Present();

		// 렌더 큐 비우기
		renderQueue.clear();
	}

	Renderer& Renderer::Get()
	{
		if (!instance)
		{
			MessageBoxA(
				nullptr,
				"Renderer::Get() - instance is null",
				"Error",
				MB_OK
			);

			__debugbreak();
		}

		return *instance;
	}

	void Renderer::Clear()
	{
		// 화면 지우기
		// 1. 프레임(2차원 배열 데이터) 지우기
		frame->Clear(screenSize);

		// 2. 콘솔 버퍼 지우기
		GetCurrentBuffer()->Clear();
	}

	void Renderer::Submit(
		 const char* text,
		 const Vector2& position,
		 Color color,
		 int sortingOrder)
	{
		 // 렌더 데이터 생성 후 큐에 추가하기
		 RenderCommand command = {};
		 command.text = text;
		 command.position = position;
		 command.color = color;
		 command.sortingOrder = sortingOrder;

		 renderQueue.emplace_back(command);
	}

	
	void Renderer::Present()
	{
		// // 그림을 그린 버퍼의 정보를 가져온다
		// auto* targetBuffer = GetCurrentBuffer();
		// 
		// COORD bufferSize = { (short)screenSize.x, (short)screenSize.y };
		// COORD bufferCoord = { 0, 0 };
		// SMALL_RECT writeRegion = { 0, 0, (short)(screenSize.x - 1), (short)(screenSize.y - 1) };
		// 
		// WriteConsoleOutput(
		// 	targetBuffer->GetBuffer(),
		// 	GetFrameBuffer(),
		// 	bufferSize,
		// 	bufferCoord,
		// 	&writeRegion
		// );
		// 
		// // 버퍼 교환.
		// SetConsoleActiveScreenBuffer(targetBuffer->GetBuffer());
		// 
		// // 인덱스 교체
		// currentBufferIndex = 1 - currentBufferIndex;

		auto* target_buffer = GetCurrentBuffer();
		if (!target_buffer) return; // [체크 1] 버퍼 객체 자체가 없나?

		HANDLE h_out = target_buffer->GetBuffer(); // [체크 2] 핸들이 유효한가?

		COORD buffer_size = { (short)screenSize.x, (short)screenSize.y };
		COORD buffer_coord = { 0, 0 };
		SMALL_RECT write_region = { 0, 0, (short)(screenSize.x - 1), (short)(screenSize.y - 1) };

		// [체크 3] WinAPI의 반환값을 변수에 담아!
		BOOL success = WriteConsoleOutput(
			h_out,
			GetFrameBuffer(),
			buffer_size,
			buffer_coord,
			&write_region
		);

		// [핵심] 여기서 브레이크포인트를 걸고 success가 TRUE인지 FALSE인지 봐!
		if (!success) {
			DWORD error = GetLastError(); // 실패했다면 이유(에러코드)를 알려줄 거야.
			__debugbreak(); // 여기서 멈추면 범인은 WinAPI 설정 오류!
		}

		SetConsoleActiveScreenBuffer(h_out);
		currentBufferIndex = 1 - currentBufferIndex;
	}
	
	ScreenBuffer* Renderer::GetCurrentBuffer()
	{
		return screenBuffers[currentBufferIndex];
	}
	
	
}