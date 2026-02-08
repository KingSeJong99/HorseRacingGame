#pragma once

#include"Common/common.h"
#include<Windows.h>

namespace Mint
{
	class MINT_API Vector2
	{
	public :
		Vector2();
		Vector2(float x, float y);
		~Vector2();

		// 문자열로 변환해서 반환하는 함수
		const char* ToString();

		Vector2 operator+(const Vector2& other) const;
		Vector2 operator-(const Vector2& other) const;

		bool operator==(const Vector2& other) const;
		bool operator!=(const Vector2& other) const;

		// 형변환 연산자 오버로딩
		operator COORD() const;

		// 벡터의 기본 값
		static Vector2 Zero;
		static Vector2 One;
		static Vector2 Up;
		static Vector2 Right;

		float x = 0;
		float y = 0;

	private:
		// 벡터 값을 문자열로 변환할 때 사용할 변수.
		char* string = nullptr;
	};
}
