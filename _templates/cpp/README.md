# {{TOPIC_KO}} 학습 저장소

{{DESCRIPTION}}

> **선행 학습**: (있다면 선행 practice 명시)

## 빠른 시작

Visual Studio가 설치된 환경에서 각 Phase 폴더의 솔루션 또는 개별 프로젝트를 빌드합니다.

```powershell
MSBuild Phase1_{{FIRST_PHASE}}\Phase1.sln /p:Configuration=Debug /p:Platform=x64
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
2. `main.cpp`에서 API 호출 순서를 따라갑니다.
3. 오류 발생 시 `GetLastError()` 또는 반환 코드를 확인합니다.
4. 확장 과제를 구현하고 전체 솔루션을 재빌드합니다.
