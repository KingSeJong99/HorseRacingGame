#ifndef TEXTLAYOUT_H_
#define TEXTLAYOUT_H_

#include "Render/Renderer.h"
#include "MintUI_API.h"

#include <string>
#include <vector>


namespace UI
{
    // 텍스트 정렬 방식
    enum class Alignment
    {
        Left,
        Center,
        Right
    };

    // 문자 하나에 대한 모든 정보를 담는 조체
    // 
    struct SpriteCell {
        wchar_t character = L' ';
        Mint::Color foreground = Mint::Color::White;
        Mint::Color background = Mint::Color::Black;
    };

    // 화면에 출력할 AnsiArt 객체들을 배치할지 결정하는 클래스s
    class MINT_UI_API TextLayout
    {
    public:
        TextLayout(Mint::Renderer& renderer);

        // 지정된 위치에 사각형 테두리를 그리는 함수
        void DrawBox(int x, int y, int width, int height, const std::wstring& title = L"", Mint::Color borderColor = Mint::Color::White, Mint::Color bgColor = Mint::Color::Black, Mint::Color titleColor = Mint::Color::White, int sorting_order = 0);

        // 지정된 영역(box) 안에서 정렬된 텍스트를 그리는 함수
        void DrawTextAligned(int x, int y, int width, const std::wstring& text, UI::Alignment align = Alignment::Left, Mint::Color fgColor = Mint::Color::White, Mint::Color bgColor = Mint::Color::Black);

        // 단순 텍스트를 그리는 함수
        void DrawText(int x, int y, const std::wstring& text);

        // 여러 줄로 된 아스키 아트(스프라이트)를 그리는 함수
        void DrawSprite(int x, int y, const std::vector<std::wstring>& sprite, Mint::Color fg_color = Mint::Color::White, Mint::Color bg_color = Mint::Color::Black, int sorting_order = 0);

        // HP 바 같은 진행 상태를 표시하는 막대를 그리는 함수
        void DrawProgressBar(int x, int y, int width, int height, float percentage,
            wchar_t fillChar = L'█',
            Mint::Color fg_color = Mint::Color::White,
            Mint::Color bg_color = Mint::Color::Gray,
            int sorting_order = 0);

    private:
        // 이 클래스는 Renderer에 대한 참조를 들고,
        // 모든 그리기 요청을 최종적으로 Renderer의 Submit 함수로 전달한다
        Mint::Renderer& renderer_;
    };
}
#endif // TEXTLAYOUT_H_

