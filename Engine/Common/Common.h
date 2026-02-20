#pragma once
#pragma warning(disable: 4251)	// 달리 할 방도가 없다
#pragma warning(disable: 4172)	// 위험성을 인지한 상태로 사용한다

#define DLLEXPORT   __declspec(dllexport)
#define DLLIMPORT   __declspec(dllimport)

// MINT_API: DLL 내보내기/가져오기 매크로
// Engine 프로젝트 설정의 전처리기 정의에 'ENGINE_EXPORTS'가 포함되어야 합니다.
#if ENGINE_BUILD_DLL
#define MINT_API DLLEXPORT
#else
#define MINT_API DLLIMPORT
#endif
