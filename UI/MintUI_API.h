// DLL을 정의해서 사용하려는 경우
// dllexport 혹은 import를 지정하는 것이 중요하다.
// 지정이 끝났다면 프로젝트 속성 -> 전처리기 정의에 명시를 해주면 된다

#ifndef MINT_UI_API_H_
#define MINT_UI_API_H_

// --- 플랫폼별 심볼 노출 정의 ---
#if defined(_WIN32) || defined(_WIN64)
    // Windows 환경
#define MINT_DLLEXPORT __declspec(dllexport)
#define MINT_DLLIMPORT __declspec(dllimport)
#else
    // macOS / Linux 환경 (GCC/Clang)
    // 기본적으로 'default' 가시성을 주면 DLL(Shared Library) 밖으로 노출돼요.
#define MINT_DLLEXPORT __attribute__((visibility("default")))
#define MINT_DLLIMPORT __attribute__((visibility("default")))
#endif

// --- 프로젝트별 EXPORT / IMPORT 스위칭
#ifdef MINT_UI_EXPORTS
#define MINT_UI_API DLLEXPORT
#else
#define MINT_UI_API DLLIMPORT
#endif

#endif // MINT_UI_API_H_

