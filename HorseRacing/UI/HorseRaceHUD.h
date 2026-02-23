#ifndef HORSERACEHUD_H_
#define HORSERACEHUD_H_

#include <string>
#include <vector>
#include "TextLayout.h"
#include "Math/Color.h"

namespace horseracing {
	class Horse;

	// UI 레이아웃을 위한 구조체 (LineTrack과 공유)
	struct RenderLayout {
		int track_start_x = 2;
		int track_end_x = 0;
		int ui_start_x = 0;
		int ui_end_x = 0;
		int log_start_x = 0;
		int draw_y_start = 3;
		int width = 0;
		int height = 0;
	};

	class HorseRaceHUD {
	public :
		HorseRaceHUD(UI::TextLayout& layout);

		// 각 상황에 맞는 UI 그리기 함수들
		void DrawBettingMenu(const RenderLayout& render_layout, const std::vector<Horse*>& horses, int selected_idx);
		void DrawTrack(const RenderLayout& render_layout, Mint::Renderer& renderer);
		void DrawHorses(const RenderLayout& render_layout, const std::vector<Horse*>& horses, Mint::Renderer& renderer);
		void DrawRankUI(const RenderLayout& render_layout, const std::vector<Horse*>& horses, const std::vector<Horse*>& sorted_horses);
		void DrawRaceLogs(const RenderLayout& render_layout, const std::vector<std::wstring>& logs);
		void DrawScoreboard(const RenderLayout& render_layout, const std::vector<Horse*>& sorted_horses);

	private:
		UI::TextLayout& ui_layout_;

		// 문자열 처리를 위한 유틸리티 (UI 전용)
		const wchar_t* GetMedalEmoji(int rank);
		int GetVisualWidth(const std::wstring& text);
		std::wstring PadRight(std::wstring text, int targetWidth);
	};
}


#endif // HORSERACEHUD_H_