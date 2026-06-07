# {{TOPIC_KO}} 학습 저장소

{{DESCRIPTION}}

> **선행 학습**: (있다면 선행 practice 명시)

## 빠른 시작

.NET SDK {{DOTNET_VERSION_MAJOR}} 이상이 설치된 환경에서 각 모듈 폴더의 `.csproj`를 빌드합니다.

```powershell
dotnet run --project Phase1_{{FIRST_PHASE}}\01_{{FIRST_MODULE}}\01_{{FIRST_MODULE}}.csproj
```

{{NOTES}}

## 문서 구조

| 문서 | 용도 |
| --- | --- |
| `README.md` | 저장소 개요, 빌드 방법, 전체 목차 |
| `CLAUDE.md` | 에이전트 작업 지침 및 컨벤션 |
| `LEARNING_GUIDE.md` | 모듈별 학습 교안, 실습 절차, 확장 과제 |

## 전체 로드맵

{{PHASE_LIST}}

## 모듈 목차

{{MODULE_LIST}}

## 권장 학습 방식

1. `LEARNING_GUIDE.md`에서 해당 모듈의 목표와 관찰 포인트를 읽습니다.
2. `Program.cs`의 메서드 호출 순서를 따라갑니다.
3. 예외를 의도적으로 유발하고 오류 타입과 메시지를 분석합니다.
4. 확장 과제를 구현하고 `dotnet run`으로 결과를 검증합니다.
