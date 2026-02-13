#ifndef TEXTLAYOUT_H_
#define TEXTLAYOUT_H_

#include <string>
#include <vector>
#include "Render/Renderer.h"

namespace UI
{
    // 텍스트 정렬 방식
    enum class Alignment
    {
        Left,
        Center,
        Right
    };

    class TextLayout
    {
    public:
        TextLayout(Mint::Renderer& renderer);

        // 지정된 위치에 사각형 테두리를 그리는 함수
        void DrawBox(int x, int y, int width, int height, const std::wstring& title = L"");

        // 지정된 영역(box) 안에서 정렬된 텍스트를 그리는 함수
        void DrawTextAligned(int x, int y, int width, const std::wstring& text, Alignment align = Alignment::Left);

        // 단순 텍스트를 그리는 함수
        void DrawText(int x, int y, const std::wstring& text);

        // 여러 줄로 된 아스키 아트(스프라이트)를 그리는 함수
        void DrawSprite(int x, int y, const std::vector<std::wstring>& sprite);

        // HP 바 같은 진행 상태를 표시하는 막대를 그리는 함수
        void DrawProgressBar(int x, int y, int width, float percentage,
            wchar_t fillChar = L'█',
            Mint::Color fgColor = Mint::Color::White,
            Mint::Color bgColor = Mint::Color::Gray);

    private:
        // 이 클래스는 Renderer에 대한 참조를 들고,
        // 모든 그리기 요청을 최종적으로 Renderer의 Submit 함수로 전달한다
        Mint::Renderer& renderer_;
    };
}
#endif // TEXTLAYOUT_H_

