#include "TextLayout.h"
#include "Render/Renderer.h" // Mint::Renderer 사용을 위해 포함
#include "Math/Color.h"
#include <algorithm> // std::clamp 사용을 위해 추가

namespace UI
{
	TextLayout::TextLayout(Mint::Renderer& renderer)
		: renderer_(renderer) // 참조 멤버 초기화
	{
	}

	void TextLayout::DrawBox(int x, int y, int width, int height, const std::wstring& title, Mint::Color border_color, Mint::Color bg_color, Mint::Color title_color, int sorting_order)
	{
		if (width < 2 || height < 2) return; // 박스를 그릴 최소 크기 검사

		// 1. 네 귀퉁이 그리기
		renderer_.SetCell(x, y, L'┏', border_color, bg_color, 100);
		renderer_.SetCell(x + width - 1, y, L'┓', border_color, bg_color, 100);
		renderer_.SetCell(x, y + height - 1, L'┗', border_color, bg_color, 100);
		renderer_.SetCell(x + width - 1, y + height - 1, L'┛', border_color, bg_color, 100);

		// 2. 가로선 그리기 (상단, 하단)
		for (int i = 1; i < width - 1; ++i) {
			renderer_.SetCell(x + i, y, L'━', border_color, bg_color, 100);
			renderer_.SetCell(x + i, y + height - 1, L'━', border_color, bg_color, 100);
		}

		// 3. 세로선 그리기 (좌측, 우측)
		for (int i = 1; i < height - 1; ++i) {
			renderer_.SetCell(x, y + i, L'┃', border_color, bg_color, 100);
			renderer_.SetCell(x + width - 1, y + i, L'┃', border_color, bg_color, 100);
		}

		// 4. 제목 그리기 (비어있지 않다면)
		if (!title.empty()) {
			// 제목이 너무 길면 잘라내기
			int max_title_width = width - 4;
			if (max_title_width < 1) return;

			std::wstring final_title = title;
			if (Mint::Renderer::CalculateVisualWidth(final_title) > max_title_width)
			{
				// 간단한 잘라내기 로직, 더 정교하게 만들 수 있음
				// 예를들면 ...로 바꾼다거나 등
				while (Mint::Renderer::CalculateVisualWidth(final_title) > max_title_width)
				{
					final_title.pop_back();
				}
			}
			// 제목은 Submit으로 직접 그려서 색상 지정
			renderer_.Submit(final_title.c_str(), Mint::Vector2(static_cast<float>(x + 2), static_cast<float>(y)), title_color, sorting_order);
		}
	}

	void TextLayout::DrawTextAligned(int x, int y, int width, const std::wstring& text, Alignment align, Mint::Color fg_color, Mint::Color bg_color)
	{
		int text_width = Mint::Renderer::CalculateVisualWidth(text);
		int final_x = x;

		// 2. 정렬 방식에 따른 시작 X 좌표 계산
		switch (align) {
			case UI::Alignment::Left:
				// 기본값 x를 사용
				break;
			case UI::Alignment::Center:
				final_x = x + (width - text_width) / 2;
				break;
			case UI::Alignment::Right:
				final_x = x + width - text_width;
				break;
		}

		// final_x가 화면 범위 밖으로 나가지 않도록 클램프 (선택적)
		// 현재 Renderer의 Submit은 클리핑을 자체적으로 처리하므로 여기서는 생략
		
		// TextLayout은 Submit으로 전달된 bg_color를 처리할 수 없으므로 무시됨
		renderer_.Submit(text.c_str(), Mint::Vector2(final_x, y), fg_color);
	}

	void TextLayout::DrawText(int x, int y, const std::wstring& text)
	{
		// DrawText는 기본적으로 정렬 없이 왼쪽 상단부터 그림
		renderer_.Submit(text.c_str(), Mint::Vector2(x, y));
	}

	// HACK: 단색 스프라이트에 해당하는 함수.
	// 픽셀 당 다른 색상을 가진 경우는 아직 다루지 않음
	void TextLayout::DrawSprite(int x, int y, const std::vector<std::wstring>& sprite, Mint::Color fg_color, Mint::Color bg_color, int sorting_order)
	{
		// 2차원 행렬이므로 행 구분하기
		for (int j = 0; j < sprite.size(); ++j) {
			const std::wstring& line = sprite[j];
			int current_draw_x = x; // 각 줄의 시작 X 좌표 (시각적 너비 고려)

			// j행의 값들을 찍어내기
			for (int i = 0; i < line.length(); ++i) { // 문자열 인덱스
				wchar_t ch = line[i];
				
				// 투명 문자는 그리지 않고 건너뛴다. (배경이 비치도록)
				if (ch == L' ') { 
					// 멀티 바이트 때문에 ch가 128 이상이라면 2칸을 이동한다.
					current_draw_x += Mint::Renderer::CalculateVisualWidth(std::wstring(1, ch));
					continue; 
				}

				renderer_.SetCell(current_draw_x, y + j, ch, fg_color, bg_color, sorting_order);
				
				// 다음 문자를 그릴 x 좌표 업데이트 (시각적 너비 고려)
				current_draw_x += Mint::Renderer::CalculateVisualWidth(std::wstring(1, ch));
			}
		}
	}

	void TextLayout::DrawProgressBar(int x, int y, int width, int height, float percentage,
		wchar_t fillChar, Mint::Color fg_color, Mint::Color bg_color, int sorting_order)
	{
		// 0.0f에서 1.0f 사이로 percentage 값 정규화
		// clamp 함수는 C++17부터 사용 가능
		percentage = std::clamp(percentage, 0.0f, 1.0f);

		// 채워질 칸의 개수 계산
		int filled_count = static_cast<int>(width * percentage);
		
		// 각 줄마다 프로그레스 바 그리기
		for (int j = 0; j < height; ++j) {
			int current_draw_x = x; // 각 줄의 시작 x 좌표 초기화

			for (int i = 0; i < width; ++i) { // 막대 너비만큼 반복
				wchar_t char_to_draw;
				Mint::Color current_fg_color = fg_color;
				Mint::Color current_bg_color = bg_color;

				if (i < filled_count) {
					char_to_draw = fillChar; // 채워진 부분
				}
				else {
					char_to_draw = L'░'; // 비어있는 부분은 옅은 음영
					current_fg_color = bg_color; // 비어있는 부분의 전경색은 배경색으로 설정
				}
				
				// SetCell 인자 순서: x, y, ch, fg, bg, sorting
				renderer_.SetCell(current_draw_x, y + j, char_to_draw, current_fg_color, current_bg_color, sorting_order);
				
				// 다음 문자를 그릴 x 좌표 업데이트 (시각적 너비 고려)
				current_draw_x += Mint::Renderer::CalculateVisualWidth(std::wstring(1, char_to_draw));
			}
		}
	}
}