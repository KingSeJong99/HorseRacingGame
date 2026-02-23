#include "HorseRaceHUD.h"
#include "Horse/Horse.h"
#include <Windows.h>

namespace horseracing {

    HorseRaceHUD::HorseRaceHUD(UI::TextLayout& layout) : ui_layout_(layout) {}

    void HorseRaceHUD::DrawBettingMenu(const RenderLayout& render_layout, const std::vector<Horse*>& horses, int selected_idx) {
        const int kMenuWidth = 104; // 전광판과 동일한 값으로 일관성을 맞춤.
        int start_x = (render_layout.width - kMenuWidth) / 2;
        int start_y = (render_layout.height - 15) / 2; // 중앙 배치를 위한 Y 좌표

        // 1. 메뉴 박스 그리기
        ui_layout_.DrawBox(start_x, start_y, kMenuWidth, 15, L"BETTING : SELECT YOUR HORSE", Mint::Color::White, Mint::Color::Black, Mint::Color::White);

        // 2. 헤더 그리기
        // DrawBox 내부에서 이미 테두리를 그렸으므로 내용만 채운다.
        // 좌표 조정이 필요함. DrawBox는 sx, sy부터 시작하므로
        // 헤더는 sx + 2 정도부터 시작
        int content_sx = start_x + 2;
        int content_sy = start_y + 2;

        ui_layout_.DrawTextAligned(content_sx, content_sy, kMenuWidth - 8, L"NO.    HORSE NAME          SPD     STM     TRAIT",
            UI::Alignment::Left, Mint::Color::BrightWhite);
        ui_layout_.DrawTextAligned(content_sx, content_sy + 1, kMenuWidth - 8, L"──────────────────────────────────────────────────",
            UI::Alignment::Left, Mint::Color::White);

        // 3. 8마리의 스탯 리스트 출력
        for (int i = 0; i < 8 && i < horses.size(); ++i) {
            auto horse = horses[i];
            std::wstring name_str = PadRight(horse->GetName(), 16);
            std::wstring spd_str = PadRight(std::to_wstring(static_cast<int>(horse->GetMax_speed())), 4);
            std::wstring stm_str = PadRight(std::to_wstring(static_cast<int>(horse->GetStamina())), 4);

            // 선택된 말은 노란색으로 강조

            Mint::Color color = (i == selected_idx) ? Mint::Color::BrightYellow : Mint::Color::Gray;
            wchar_t row[256];
            swprintf_s(row, L"%d.    %s    %s    %s    None", i + 1, name_str.c_str(), spd_str.c_str(), stm_str.c_str());

            ui_layout_.DrawTextAligned(content_sx, content_sy + 2 + i, kMenuWidth - 8, row, UI::Alignment::Left, color);
        }
    }

    void HorseRaceHUD::DrawTrack(const RenderLayout& render_layout, Mint::Renderer& renderer) {
        for (int i = 0; i <= 8; ++i) {
            int y = render_layout.draw_y_start + (i * 2);
            if (y >= render_layout.height) break;

            for (int x = 0; x < render_layout.track_end_x; ++x) {
                int draw_x = render_layout.track_start_x + x;
                if (x == 0 || x == render_layout.track_end_x - 1) {
                    renderer.SetCell(draw_x, y, L'|', Mint::Color::BrightBlue, Mint::Color::Black);
                }
                else {
                    renderer.SetCell(draw_x, y, L'-', Mint::Color::BrightGreen, Mint::Color::Black);
                }
            }
        }
    }

    void HorseRaceHUD::DrawHorses(const RenderLayout& render_layout, const std::vector<Horse*>& horses, Mint::Renderer& renderer) {
        for (auto horse : horses) {
            const auto& data = horse->GetHorseRaceData();
            int x = render_layout.track_start_x + (int)(data.position * (render_layout.track_end_x - 1));
            int y = render_layout.draw_y_start + ((data.lane_index - 1) * 2) + 1;

            if (x < 0 || x >= render_layout.track_end_x || y < 0 || y >= render_layout.height) {
                continue;
            }
            renderer.SetCell(x, y, L'M', Mint::Color::BrightRed, Mint::Color::Black);
        }
    }

    void HorseRaceHUD::DrawRankUI(const RenderLayout& render_layout, const std::vector<Horse*>& horses, const std::vector<Horse*>& sorted_horses) {
        Horse* actual_leading_horse = sorted_horses.empty() ? nullptr : sorted_horses[0];

        for (int i = 0; i < horses.size(); ++i) {
            auto horse = horses[i];
            int rank = horse->GetHorseRaceData().current_rank;
            int lane_index = horse->GetHorseRaceData().lane_index;
            std::wstring name = horse->GetName();
            const wchar_t* medal = GetMedalEmoji(rank);

            std::wstring rank_text = std::wstring(medal) + L" " + std::to_wstring(lane_index) + L". " + name;
            Mint::Color color = (horse == actual_leading_horse) ? Mint::Color::BrightYellow : Mint::Color::White;

            int y = render_layout.draw_y_start + (i * 2) + 1;
            int current_ui_x = render_layout.ui_start_x;

            ui_layout_.DrawTextAligned(current_ui_x, y, render_layout.ui_end_x - current_ui_x, rank_text, UI::Alignment::Left, color);
        }
    }

    void HorseRaceHUD::DrawRaceLogs(const RenderLayout& render_layout, const std::vector<std::wstring>& logs) {
        // Todo: 그럴 일은 없겠지만, 말들이 8라인을 넘게 달려선다면. 지금 처럼 수동이 아닌
        // (말 + 1) * 2를 한 값을 적용해야 할 것이다
        const int MAX_LOG_LINES = 16;
        const int LOG_Y_END = render_layout.draw_y_start + MAX_LOG_LINES - 1;		// 로그의 Y좌표 맥시멈

        // 최신 로그만 가져오기 위한 인덱스 계산
        int num_logs = static_cast<int>(logs.size());
        int display_count = (num_logs > MAX_LOG_LINES) ? MAX_LOG_LINES : num_logs;

        // 1. 로그 영역 청소 (DrawBox로 덮어쓰기)
        // 배경색(Black)으로 채워서 이전 프레임 잔상 제거
        ui_layout_.DrawBox(render_layout.log_start_x, render_layout.draw_y_start,
            render_layout.width - render_layout.log_start_x, MAX_LOG_LINES,
            L"", Mint::Color::Black, Mint::Color::Black, Mint::Color::Black);

        // 일반적인 채팅창처럼 로그를 아래에서 위로 올려보내기
        for (int i = 0; i < display_count; ++i) {
            // 가장 최신의 로그 인덱스는 num_logs - 1이다
            const std::wstring& msg = logs[num_logs - 1 - i];

            // y좌표 계산하기
            int y = LOG_Y_END - i;

            // 2. 로그 출력
            ui_layout_.DrawTextAligned(render_layout.log_start_x, y,
                render_layout.width - render_layout.log_start_x - 4, // 여백 좀 줌
                msg, UI::Alignment::Left, Mint::Color::Gray);
        }
    }

    void HorseRaceHUD::DrawScoreboard(const RenderLayout& render_layout, const std::vector<Horse*>& sorted_horses) {
        // 52 * 2
        const int kScoreBoardWidth = 104;
        const int kScoreBoardHeight = 5 + static_cast<int>(sorted_horses.size());

        // 전광판은 중앙에 설치할 것이기 때문에 중앙의 좌표를 구한다
        int sx = ((render_layout.width - kScoreBoardWidth) / 2) + 6;
        int sy = (render_layout.height - kScoreBoardHeight) / 2;

        // 1. 박스 그리기 (테두리 + 배경)
        ui_layout_.DrawBox(sx, sy, kScoreBoardWidth, kScoreBoardHeight + 2, L"FINAL RESULTS", Mint::Color::White, Mint::Color::Black, Mint::Color::White);

        // 2. 헤더 그리기
        int content_sx = sx + 2;
        int content_sy = sy + 2;

        ui_layout_.DrawTextAligned(content_sx, content_sy, kScoreBoardWidth - 4, L"RANK     HORSE NAME          TIME         REMARK", UI::Alignment::Left, Mint::Color::White);
        ui_layout_.DrawTextAligned(content_sx, content_sy + 1, kScoreBoardWidth - 4, L"──────────────────────────────────────────────────", UI::Alignment::Left, Mint::Color::White);

        // 3. 순위 데이터 출력 (sorted_horses_ 기준)
        for (int i = 0; i < sorted_horses.size(); ++i) {
            auto horse = sorted_horses[i];
            const auto& data = horse->GetHorseRaceData();
            std::wstring paddedName = PadRight(horse->GetName(), 18);			// 유니코드 기준 9칸까지 할당

            wchar_t row[256];
            swprintf_s(row, L"  %d      %s    %6.2fs",
                i + 1, paddedName.c_str(), data.finish_time);

            // 1등은 노란색(금색), 나머지는 흰색
            Mint::Color color = (i == 0) ? Mint::Color::Yellow : Mint::Color::Gray;
            ui_layout_.DrawTextAligned(content_sx, content_sy + 2 + i, kScoreBoardWidth - 4, row, UI::Alignment::Left, color);
        }
    }

    const wchar_t* HorseRaceHUD::GetMedalEmoji(int rank) {
        switch (rank) {
        case 1: return L"(G)";
        case 2: return L"(S)";
        case 3: return L"(B)";
        default: return L"   ";
        }
    }

    int horseracing::HorseRaceHUD::GetVisualWidth(const std::wstring& text) {
        int width = 0;
        for (wchar_t ch : text) {
            // 문자열의 폭을 아스키 코드와 非아스키코드로 구분하여 구한다
            width += (ch > 127) ? 2 : 1;
        }
        return width;
    }

    std::wstring HorseRaceHUD::PadRight(std::wstring text, int targetWidth) {
        int currentWidth = GetVisualWidth(text);

        // 채워져야 할 값 = 유니코드 - 아스키 코드의 차이 만큼
        int paddingNeeded = targetWidth - currentWidth;

        if (paddingNeeded > 0) {
            // 부족한 칸 수 만큼 공백을 추가한다
            text.append(paddingNeeded, L' ');
        }
        return text;
    }
}   // namespace horseracing