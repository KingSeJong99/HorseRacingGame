#pragma once

#include"Common/Common.h"
#include<Windows.h>

// SetConsoleTextAttribute

namespace Mint
{
	// 콘솔에 텍스트 색상 등을 지정할 때 사용할 색상 열거형이다
	// 기존의 색상 코드에서 최대 표현 가능한 16색 팔레트로 확장함
	// 전경뿐만 아니라 Background 색상도 고려해야 한다!
	enum class MINT_API Color : WORD
	{
		Black			= 0,
		Blue			= FOREGROUND_BLUE,
		Green			= FOREGROUND_GREEN,
		Cyan			= FOREGROUND_BLUE | FOREGROUND_GREEN,
		Red				= FOREGROUND_RED,
		Magenta			= FOREGROUND_RED | FOREGROUND_BLUE,
		Yellow			= FOREGROUND_RED | FOREGROUND_GREEN,
		White			= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		Gray			= FOREGROUND_INTENSITY, // 회색은 종종 INTENSITY로 단독 사용하기도 한다
		BrightBlue		= FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		BrightGreen		= FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		BrightCyan		= FOREGROUND_BLUE | FOREGROUND_GREEN,
		BrightRed		= FOREGROUND_RED | FOREGROUND_INTENSITY,
		BrightMagenta	= FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		BrightYellow	= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		BrightWhite		= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	};

	// 색상 양자화(Color Quantization)
	// 원본의 rgb값의 근사치를 console에 출력 가능한 값으로 파싱하기
	Color RgbToLegacyColor(int r, int g, int b);
}