#include "Color.h"

Mint::Color Mint::RgbToLegacyColor(int r, int g, int b)
{
	// 각 채널의 밝기를 기준으로 색상 비트를 결정한다!
			// 임계값 128은 임의이며, 조정될 수 있음.
	bool has_red = (r > 128);
	bool has_green = (g > 128);
	bool has_blue = (b > 128);

	// 전체적으로 밝은지 어두운지를 판단하는 기준
	// 임의의 수치이며 조정될 수 있음.
	bool is_bright = (r + g + b) > (128 * 3); // 세 채널의 평균이 128보다 크면 밝은 것으로 가정

	WORD color_code = 0;

	if (has_red) color_code |= FOREGROUND_RED;
	if (has_green) color_code |= FOREGROUND_GREEN;
	if (has_blue) color_code |= FOREGROUND_BLUE;

	// 어떤 색상도 지배적이지 않다면? (매우 어둡거나 회색조인 경우)
	if (color_code == 0) {
		if (is_bright) return Mint::Color::Gray;	// 그래도 밝으면 회색
		return Mint::Color::Black;	// 그렇지 않다면 검정
	}

	// 주 색상이 결정된 상태에서 전반적인 밝기를 고려하여 강조를 한다
	// 이때, 완전한 흰색이 아니면서 밝을 때만 INTENSITY를 추가해 밝은 색으로 만든다
	// 완전한 흰색은 이미 RGB를 합한 값이므로 INTENSITY를 더해 BrightWhite를 만들 수 있다
	if (is_bright) {
		color_code |= FOREGROUND_INTENSITY;
	}

	return static_cast<Mint::Color>(color_code);
}