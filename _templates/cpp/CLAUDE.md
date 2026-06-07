# CLAUDE.md — {{PRACTICE_NAME}} 에이전트 지침

## 워크스페이스 개요

- **주제**: {{TOPIC_KO}}
- **언어**: {{LANGUAGE}} (C++17 이상)
- **빌드 시스템**: Visual Studio / MSBuild (`.vcxproj`)
- **대상 플랫폼**: x64 Windows
- **선행 학습**: {{PREREQUISITE}} (없으면 삭제)
- **관련 저장소**: {{RELATED_REPOS}} (없으면 삭제)

## 디렉토리 구조

```
{{PRACTICE_NAME}}/
{{PHASE_TREE}}
├── README.md
├── CLAUDE.md               # 이 파일
└── LEARNING_GUIDE.md
```

## 새 모듈 추가 방법

1. 해당 Phase 폴더 아래 모듈 폴더를 만듭니다.
   예: `Phase1_{{FIRST_PHASE}}/01_{{FIRST_MODULE}}/`
2. Visual Studio에서 빈 C++ 콘솔 프로젝트(`.vcxproj`)를 생성합니다.
3. 아래 `main.cpp` 초기화 템플릿을 기반으로 작성합니다.
4. Phase 폴더의 솔루션(`.sln`)에 프로젝트를 추가합니다.

## main.cpp 초기화 템플릿

```cpp
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
{{EXTRA_INCLUDES}}
#include <cstdio>

// 오류 출력 헬퍼
static void PrintError(const char* ctx) {
    DWORD err = GetLastError();
    char buf[256]{};
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, 0, buf, sizeof(buf), nullptr);
    printf("[ERROR] %s: %lu — %s\n", ctx, err, buf);
}

int main() {
    // --- 여기에 구현 ---
    return 0;
}
```

## 코딩 컨벤션

- Win32 API 호출 후 반환값을 검사하고 실패 시 `PrintError`로 출력합니다.
- 핸들은 `using` 블록 또는 `goto cleanup` 패턴으로 반드시 닫습니다.
- 성능 측정에는 `QueryPerformanceCounter` / `QueryPerformanceFrequency`를 사용합니다.
- 개념 비교 실험은 동일 파일 내 `#define USE_XXX` 분기로 처리합니다.
- {{EXTRA_CONVENTIONS}}

## 프로젝트 속성 기본값

```
구성 타입       : Application (.exe)
문자 집합       : Unicode
C++ 표준        : /std:c++17
추가 종속성     : {{EXTRA_LIBS}}
경고 수준       : /W4
```

## Phase별 학습 진행 현황

{{PHASE_STATUS_TABLE}}

## 주의 사항

{{NOTES}}
