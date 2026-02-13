#include "Renderer.h"
#include "ScreenBuffer.h"
#include "Util/Util.h"

namespace Mint 
{
	Renderer::Frame::Frame(int total_pixels)
		: char_info_array(new CHAR_INFO[total_pixels]()),
		sorting_order_array(new int[total_pixels]())
	{
	}
 
	Renderer::Frame::~Frame()
	{
		SafeDeleteArray(char_info_array);
		SafeDeleteArray(sorting_order_array);
	}
 
	void Renderer::Frame::Clear(const Vector2& screenSize)
	{
		const int width = screenSize.x;
		const int height = screenSize.y;
 
		for (int y = 0; y < height; ++y)
		{
			for (int x= 0; x < width; ++x)
			{
				// 배열 인덱스 구하기
				const int index = (y * width) + x;
				
				// 글자 값 및 속성 설정
				CHAR_INFO& info = char_info_array[index];
				info.Char.UnicodeChar = L' ';
				info.Attributes = 0;
				
				// 그리기 우선순위 초기화하기
				sorting_order_array[index] = -1;
			}
		}
	}

	Renderer::Renderer(const Vector2& screen_size)
		: screen_size_(screen_size)
	{
		// 프레임 객체 생성하기
		const int bufferCount = screen_size.x * screen_size.y;
		frame_ = new Frame(bufferCount);
		
		// 프레임 초기화
		frame_->Clear(screen_size);
		
		// 이중 버퍼 객체 생성 및 초기화!
		screen_buffers_[0] = new ScreenBuffer(screen_size);
		screen_buffers_[0]->Clear();
		
		screen_buffers_[1] = new ScreenBuffer(screen_size);
		screen_buffers_[1]->Clear();
 
		// 활성화 버퍼 설정하기
		Present();
	}
 
 
	Renderer::~Renderer()
	{
		SafeDelete(frame_);
		for(ScreenBuffer*& buffer : screen_buffers_)
		{
			SafeDelete(buffer);
		}
	}
 
	void Renderer::DrawCharacter(int x, int y, wchar_t ch, Color color, int sorting_order)
	{
		// DrawCharacter는 기본적으로 전경색을 설정하고 배경색은 Black으로 가정
		SetCell(x, y, ch, color, Color::Black, sorting_order);
	}

	void Renderer::SetCell(int x, int y, wchar_t ch, Color foreground, Color background, int sorting_order)
	{
		// 화면 경계 검사
		if (x < 0 || x >= screen_size_.x || y < 0 || y >= screen_size_.y) {
			return;
		}

		const int index = (y * screen_size_.x) + x;

		// sorting_order 값 판별하기
		if (frame_->sorting_order_array[index] > sorting_order) {
			return;
		}

		// 입력 된 코드가 멀티바이트인지 아닌지 판별한다
		int char_width = (ch > 127) ? 2 : 1;

		// CHAR_INFO의 Attributes는 전경색과 배경색을 비트 OR 연산으로 조합
		WORD attributes = (WORD)foreground | ((WORD)background << 4);

		// 만약 멀티바이트(유니코드)라면
		if (char_width == 2) {
			// 그리기 가능하다면 멀티바이트가 리딩/트레일링 바이트임을 알린다
			if (x + 1 < screen_size_.x) {
				frame_->char_info_array[index].Char.UnicodeChar = ch;
				frame_->char_info_array[index].Attributes = attributes | COMMON_LVB_LEADING_BYTE;
				frame_->sorting_order_array[index] = sorting_order;

				frame_->char_info_array[index + 1].Char.UnicodeChar = ch;
				frame_->char_info_array[index + 1].Attributes = attributes | COMMON_LVB_TRAILING_BYTE;
				frame_->sorting_order_array[index + 1] = sorting_order;
			}
		}
		// 아스키 코드라면
		else {
			frame_->char_info_array[index].Char.UnicodeChar = ch;
			frame_->char_info_array[index].Attributes = attributes;
			frame_->sorting_order_array[index] = sorting_order;
		}
	}

	void Renderer::DrawCommand(const RenderCommand& command)
	{
		// 화면에 그릴 텍스트가 없거나 y축 클리핑하기
		if (command.text.empty() || command.position.y < 0 || command.position.y >= screen_size_.y)
		{
			return;
		}

		int target_x = command.position.x;
		const int target_y = command.position.y;

		for (const wchar_t ch : command.text) {
			// 입력 된 코드가 멀티바이트인지 아닌지 판별한다
			int char_width = (ch > 127) ? 2 : 1;
			
			// x축의 경계 검사
			if (target_x >= screen_size_.x) {
				break;
			}

			// x축의 경계 검사
			if (target_x < 0) {
				target_x += char_width;
				continue;
			}
			
			// 실제 그리기 함수 호출
			DrawCharacter(target_x, target_y, ch, command.color, command.sorting_order);

			target_x += char_width;
		}
	}

	void Renderer::Draw()
	{
		Clear();

		// 렌더큐 순회하면서 그리기
		// 전제조건: 레벨의 모든 액터가 렌더러에 Submit을 완료해야한다
		for (const RenderCommand& command : render_queue_)
		{
			DrawCommand(command);
		}

		// 버퍼 교환하기
		Present();

		// 렌더 큐 비우기
		render_queue_.clear();
	}

	void Renderer::Clear()
	{
		// 1. 프레임(2차원 배열 데이터) 지우기
		frame_->Clear(screen_size_);

		// 2. 콘솔 버퍼 지우기
		GetCurrentBuffer()->Clear();
	}

	// 아스키코드용 Submit 함수. 변환하여 유니코드용으로 보내진다
	void Renderer::Submit(const char* text, const Vector2& position, Color color, int sorting_order)
	{
		// 입력을 제대로 받았는지 확인하기
		if (text == nullptr) return;

		// char*를 wstring으로 변환하기
		// 매 Submit 호출마다 이 변환이 일어나므로 잦은 ASCII 문자열 제출 시 성능 저하를 유발할 수 있다.
		std::string s(text);
		std::wstring ws(s.begin(), s.end());

		// 변환된 ws를 사용하여 Unicode용 Submit 호출하기
		Submit(ws.c_str(), position, color, sorting_order);
	}

	// 유니코드용 Submit 함수
	void Renderer::Submit(const wchar_t* text, const Vector2& position, Color color, int sorting_order) {
		// 입력을 제대로 받았는지 확인하기
		if (text == nullptr) return;

		// 렌더 데이터 생성 후 큐에 추가하기
		RenderCommand command = {};
		command.text = text;
		command.position = position;
		command.color = color;
		command.sorting_order = sorting_order;

		// std::move로 복사하기
		render_queue_.emplace_back(std::move(command));
	}

	int Renderer::CalculateVisualWidth(const std::wstring& text)
	{
		int width = 0;
		for (wchar_t ch : text) {
			// 입력 된 코드가 멀티바이트인지 아닌지 판별한다
			width += (ch > 127) ? 2 : 1;
		}
		return width;
	}

	
	void Renderer::Present()
	{
		auto* target_buffer = GetCurrentBuffer();

		// 버퍼가 정상적으로 있는지 확인하기
		if (!target_buffer) return;

		HANDLE h_out = target_buffer->GetBuffer();

		COORD buffer_size = { (short)screen_size_.x, (short)screen_size_.y };
		COORD buffer_coord = { 0, 0 };
		SMALL_RECT write_region = { 0, 0, (short)(screen_size_.x - 1), (short)(screen_size_.y - 1) };

		// WinAPI의 반환값을 변수에 담기
		BOOL success = WriteConsoleOutput(
			h_out,
			GetFrameBuffer(),
			buffer_size,
			buffer_coord,
			&write_region
		);

		// Win API 설정이 올바르게 됐는지 확인하기
		if (!success) {
			DWORD error = GetLastError(); 
			__debugbreak(); 
		}

		// 버퍼에 저장된 내용을 화면에 전달하게 하기
		SetConsoleActiveScreenBuffer(h_out);
		current_buffer_index_ = 1 - current_buffer_index_;
	}
	
	ScreenBuffer* Renderer::GetCurrentBuffer()
	{
		return screen_buffers_[current_buffer_index_];
	}
}