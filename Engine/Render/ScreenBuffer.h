#pragma once

#include"Math/Vector2.h"
#include<Windows.h>

namespace Mint
{
	// 더블 버퍼링에 사용할 Console Output 핸들을 매니징하는 클래스
	class ScreenBuffer
	{
	public:
		ScreenBuffer(const Vector2& screenSize);
		~ScreenBuffer();
		
		// console 버퍼를 지우는 함수
		void Clear();
		
		// 콘솔에 2차원 글자 배열을 그릴 때 사용하는 함수
		void Draw(CHAR_INFO* charInfo);
		
		// 버퍼 반환 Getter
		inline HANDLE GetBuffer() const {return buffer;}
	
	private:
		// 콘솔 출력 핸들
		HANDLE buffer = nullptr;
		
		// 화면 크기
		Vector2 screenSize;
	};
}
