# CLAUDE.md — {{PRACTICE_NAME}} 에이전트 지침

## 워크스페이스 개요

- **주제**: {{TOPIC_KO}}
- **언어**: C# 12 (.NET {{DOTNET_VERSION}})
- **빌드 시스템**: `dotnet CLI` + `.csproj`
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
2. `dotnet new console -n <ModuleName> -o <ModuleName>` 으로 프로젝트를 생성합니다.
3. 아래 `Program.cs` 초기화 템플릿을 기반으로 작성합니다.
4. 필요한 NuGet 패키지를 추가합니다.

## 사용 NuGet 패키지

```
{{NUGET_PACKAGES}}
```

## Program.cs 초기화 템플릿

```csharp
{{PROGRAM_CS_TEMPLATE}}
```

## 코딩 컨벤션

- 모든 I/O 작업은 `async`/`await` 를 사용합니다.
- `IDisposable` / `IAsyncDisposable` 리소스는 반드시 `using` 블록으로 해제합니다.
- 외부 입력은 경계에서 유효성을 검사하고, 내부 호출은 신뢰합니다.
- `Program.cs`는 개념을 단계적으로 보여주는 로컬 메서드 분리 구조로 작성합니다.
- {{EXTRA_CONVENTIONS}}

## 프로젝트 파일 기본값 (.csproj)

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>Exe</OutputType>
    <TargetFramework>{{DOTNET_VERSION}}</TargetFramework>
    <Nullable>enable</Nullable>
    <ImplicitUsings>enable</ImplicitUsings>
  </PropertyGroup>
</Project>
```

## Phase별 학습 진행 현황

{{PHASE_STATUS_TABLE}}

## 주의 사항

{{NOTES}}
