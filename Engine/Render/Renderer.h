#pragma once

#include"Math/Vector2.h"
#include"Math/Color.h"
#include<Windows.h>
#include<vector>
#include<string>

namespace Mint
{
	// 콘솔 버퍼를 관리하는 클래스
	class ScreenBuffer;

	// DLL 내부에서만 사용하도록 한다
	// 더블 버퍼링을 지원하는 렌더러 클래스!
	class MINT_API Renderer
	{
	public:
		Renderer(const Vector2& screen_size);
		~Renderer();

		// 싱글톤 접근 함수
		static Renderer& Get();

		// 그리기 함수
		void Draw();

		// 그리는데 필요한 데이터를 제출하는 함수 - ASCII
		void Submit(
			const char* text,
			const Vector2& position,
			Color color = Color::White,
			int sorting_order = 0);

		// 그리는데 필요한 데이터를 제출하는 함수 - UNICODE
		void Submit(
			const wchar_t* text,
			const Vector2& position,
			Color color = Color::White,
			int sorting_order = 0);

		// 시각적 너비를 계산하는 정적 헬퍼 함수
		static int CalculateVisualWidth(const std::wstring& text);

		// 현재 사용할 버퍼를 반환하는 함수(Getter)
		ScreenBuffer* GetCurrentBuffer();
		
		inline CHAR_INFO* GetFrameBuffer() const { return frame_->char_info_array; }

		// 화면 크기를 반환하는 Getter 함수
		inline const Vector2& GetScreenSize() const { return screen_size_; }

		// 더블 버퍼링을 활용해 활성화 버퍼를 교환하는 함수
		void Present();

		// 화면을 지우는 함수
		void Clear();

	private:
		// 프레임 구조체 - 2차원 글자 배열의 항목이 될 구조체임
		struct Frame
		{
			Frame(int total_pixels);
			~Frame();

			// 지우기 함수. 차원 배열로 다루는 1차원 배열을 순회하면서
			// 빈 문자(' ')를 설정한다
			void Clear(const Vector2& screen_size);

			// 글자 값과 글자의 색상을 갖는 타입
			CHAR_INFO* char_info_array = nullptr;

			// 그리기 우선 순위 배열
			int* sorting_order_array = nullptr;
		};

		// 렌더링할 데이터
		struct RenderCommand
		{
			// 화면에 보여줄 문자열 값
			std::wstring text;

			// 좌표
			Vector2 position;

			// 색상
			Color color = Color::White;

			// 그리기 우선순위
			int sorting_order = 0;
		};

		// 화면 크기
		Vector2 screen_size_;

		// 관리할 프레임 객체
		Frame* frame_ = nullptr;

		// 이중 버퍼 배열
		ScreenBuffer* screen_buffers_[2] = {};

		// 현재 활성화된 버퍼 인덱스
		int current_buffer_index_ = 0;

		// 렌더 큐 (씬의 모든 그리기 명령을 모아두는 배열)
		std::vector<RenderCommand> render_queue_;

		// 단일 렌더 커맨드를 그리는 헬퍼 함수
		void DrawCommand(const RenderCommand& command);

		// 단일 문자를 그리는 헬퍼 함수
		void DrawCharacter(
			int x, int y,
			wchar_t ch,
			Color color,
			int sorting_order);
		
		// 지정된 위치에 단일 문자 및 색상 정보를 직접 설정하는 함수 (캔버스 역할)
		void SetCell(
			int x, int y,
			wchar_t ch,
			Color foreground,
			Color background,
			int sorting_order = 0);
	};
}