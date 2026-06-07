# _templates — 재사용 가능한 파일 템플릿

새 practice 디렉토리를 만들 때 이 폴더의 템플릿을 복사해 사용합니다.

## 구조

```
_templates/
├── cpp/            # C++ / Visual Studio / MSBuild 기반 practice용
│   ├── CLAUDE.md
│   ├── README.md
│   └── .gitignore
└── dotnet/         # C# / .NET SDK 기반 practice용
    ├── CLAUDE.md
    ├── README.md
    └── .gitignore
```

## 플레이스홀더 규칙

모든 템플릿 파일에서 `{{...}}` 형식의 문자열은 실제 값으로 교체해야 합니다.

| 플레이스홀더 | 설명 | 예시 |
| --- | --- | --- |
| `{{PRACTICE_NAME}}` | 디렉토리 이름 | `practice-Algorithm` |
| `{{TOPIC_KO}}` | 주제 (한국어) | `알고리즘` |
| `{{TOPIC_EN}}` | 주제 (영어) | `Algorithm` |
| `{{DESCRIPTION}}` | 한 줄 설명 | `정렬·탐색·그래프 알고리즘 실습` |
| `{{LANGUAGE}}` | 사용 언어 | `C++17` |
| `{{EXTRA_LIBS}}` | 추가 링커 라이브러리 | `ws2_32.lib` |
| `{{PHASE_LIST}}` | Phase 목록 (표 형식) | (여러 행) |
| `{{MODULE_LIST}}` | 모듈 목록 (표 형식) | (여러 행) |
| `{{DOTNET_VERSION}}` | .NET 버전 | `net8.0` |
| `{{NUGET_PACKAGES}}` | NuGet 패키지 목록 | `Dapper, Newtonsoft.Json` |
| `{{NOTES}}` | 주의 사항 | (자유 형식) |

## 사용 방법

1. 대상 언어의 폴더(`cpp/` 또는 `dotnet/`)를 복사합니다.
2. 각 파일의 `{{...}}` 플레이스홀더를 실제 값으로 교체합니다.
3. `CATALOG.md`의 해당 행을 업데이트합니다.

에이전트를 통해 자동으로 생성할 때는 `_prompts/` 의 프롬프트를 사용하세요.
