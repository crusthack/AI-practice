# _prompts — 에이전트용 작업 프롬프트

이 폴더의 파일들은 Claude Code 에이전트에게 전달할 준비가 된 프롬프트입니다.  
각 파일 상단의 `[변수 입력]` 섹션만 채워서 에이전트에게 붙여넣으면 됩니다.

## 파일 목록

| 파일 | 용도 |
| --- | --- |
| `01_new-practice-cpp.md` | **새 C++ practice 디렉토리 생성** — 폴더 구조, README, CLAUDE.md, LEARNING_GUIDE, .gitignore, Phase 폴더 |
| `02_new-practice-dotnet.md` | **새 C# practice 디렉토리 생성** — 폴더 구조, README, CLAUDE.md, LEARNING_GUIDE, .gitignore, Phase 폴더 |
| `03_add-module.md` | **기존 practice에 새 모듈 추가** — 모듈 폴더, 소스 파일, LEARNING_GUIDE 업데이트 |
| `04_review-and-improve.md` | **기존 practice 검토·고도화** — 코드 품질, 교안 완성도, 확장 과제 검토 |

## 사용 방법

1. 작업에 맞는 프롬프트 파일을 엽니다.
2. 상단 `[변수 입력]` 섹션의 값을 채웁니다.
3. 파일 전체 내용을 복사해 Claude Code 입력창에 붙여넣습니다.
4. 에이전트가 작업을 완료하면 `CATALOG.md`의 해당 항목을 업데이트합니다.

## 작업 완료 후 체크리스트

- [ ] `CATALOG.md` 상태 아이콘 업데이트 (⚪ → 🔵 → ✅)
- [ ] `CATALOG.md` 모듈 수 업데이트
- [ ] 새 practice라면 `.gitignore` 확인
- [ ] 새 practice라면 `CLAUDE.md`의 진행 현황 표 확인
