# Vulkan Learning Samples

Vulkan을 단계적으로 익히기 위한 Visual Studio C++ 샘플 모음이다. 기존 `practice-DirectX11`, `practice-DirectX12`와 같은 번호형 실습 구조를 유지하되 Vulkan의 명시적 객체 수명, 동기화, 디스크립터, 이미지 레이아웃 모델에 맞춰 커리큘럼을 재배치했다.

## Quick Start

1. Vulkan SDK를 설치하고 `VULKAN_SDK` 환경 변수가 설정되어 있는지 확인한다.
2. Visual Studio에서 `VulkanLearning.sln`을 연다.
3. 원하는 번호의 프로젝트를 시작 프로젝트로 설정하고 실행한다.

명령줄 빌드:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ".\VulkanLearning.sln" /p:Configuration=Debug /p:Platform=x64
```

## Structure

각 샘플은 독립 `.vcxproj`로 빌드되며 `main.cpp`는 공통 Vulkan 애플리케이션 셸을, `LearningStage.h`는 해당 실습의 고유 리소스와 렌더 훅을 담당한다.

자세한 순서는 `ROADMAP.md`, 현재 작업 컨텍스트는 `WORKSPACE_CONTEXT.md`를 기준으로 한다.
