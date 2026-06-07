# 프롬프트: 새 C++ practice 디렉토리 생성

> 이 파일 전체를 Claude Code에 붙여넣어 사용합니다.  
> `[변수 입력]` 섹션만 실제 값으로 채운 뒤 전달하세요.

---

## [변수 입력]

```
PRACTICE_NAME   = practice-Algorithm          # 디렉토리 이름 (practice- 접두사 포함)
TOPIC_KO        = 알고리즘                    # 주제 한국어
TOPIC_EN        = Algorithm                  # 주제 영어
DESCRIPTION     = 정렬·탐색·그래프 등 핵심 알고리즘을 C++로 직접 구현하며 시간/공간 복잡도를 분석하는 실습 모음
LANGUAGE        = C++17
EXTRA_LIBS      = (없음)
PREREQUISITE    = (없음)
RELATED_REPOS   = (없음)

PHASES:
  Phase1_Sorting     : 정렬 알고리즘 (버블·선택·삽입·합병·퀵·힙)
  Phase2_Search      : 탐색 알고리즘 (이진 탐색, BFS, DFS, 다익스트라)
  Phase3_DP          : 동적 프로그래밍 (메모이제이션, 타뷸레이션)
  Phase4_Graph       : 그래프 알고리즘 (MST, 위상 정렬, 플로이드-워셜)
  Phase5_Advanced    : 고급 주제 (세그먼트 트리, 트라이, 유니온-파인드)

MODULES_PER_PHASE:
  Phase1: BubbleSort, SelectionSort, InsertionSort, MergeSort, QuickSort, HeapSort, RadixSort
  Phase2: BinarySearch, LinearSearch, BFS, DFS, Dijkstra, AStar
  Phase3: Fibonacci_Memo, Knapsack, LCS, LIS, MatrixChain
  Phase4: Kruskal, Prim, TopologicalSort, BellmanFord, FloydWarshall
  Phase5: SegmentTree, FenwickTree, Trie, UnionFind, SuffixArray

NOTES:
  - 각 모듈은 단독 실행 가능한 콘솔 프로그램으로 작성합니다.
  - 입력은 하드코딩된 예시 데이터를 사용합니다 (파일 입출력 불필요).
  - 각 알고리즘 실행 후 결과와 실행 시간(QueryPerformanceCounter)을 출력합니다.
  - STL 알고리즘(std::sort 등)과 직접 구현의 결과를 비교합니다.
```

---

## 에이전트 지시사항

위 변수들을 사용해 아래 작업을 수행해주세요.

### 1. 폴더 구조 생성

`C:\Users\crust\Documents\AI-practice\` 아래에 `PRACTICE_NAME` 폴더를 만들고, 각 Phase 폴더와 `.gitkeep` 파일을 생성합니다.

```
practice-Algorithm/
├── Phase1_Sorting/          (.gitkeep)
├── Phase2_Search/           (.gitkeep)
├── Phase3_DP/               (.gitkeep)
├── Phase4_Graph/            (.gitkeep)
├── Phase5_Advanced/         (.gitkeep)
├── README.md
├── CLAUDE.md
├── LEARNING_GUIDE.md
└── .gitignore
```

### 2. README.md 작성

`C:\Users\crust\Documents\AI-practice\_templates\cpp\README.md` 를 기반으로, 변수들을 채워 작성합니다.

포함 내용:
- 저장소 설명 (1~2 문장)
- 빠른 시작 (MSBuild 명령)
- 전체 로드맵 표 (Phase 이름 | 학습 주제)
- 모듈 목차 (Phase별 모듈 설명)
- 권장 학습 방식

### 3. CLAUDE.md 작성

`C:\Users\crust\Documents\AI-practice\_templates\cpp\CLAUDE.md` 를 기반으로 작성합니다.

포함 내용:
- 워크스페이스 개요
- 디렉토리 구조 트리
- 새 모듈 추가 방법
- `main.cpp` 초기화 템플릿 (주제에 맞게 조정)
- 코딩 컨벤션 (주제별 특이사항 추가)
- 프로젝트 속성 기본값
- Phase별 진행 현황 표 (모두 `미시작`)
- 주의 사항

### 4. LEARNING_GUIDE.md 작성

모든 모듈에 대해 아래 형식으로 학습 교안을 작성합니다.

```
### <모듈 이름>

- **목표**: 이 모듈에서 학습할 핵심 개념
- **관찰**: 실행 후 확인해야 할 동작이나 수치
- **핵심 구현 포인트**: 구현 시 주의할 알고리즘적 포인트
- **시간 복잡도**: O(?) 최선/평균/최악
- **공간 복잡도**: O(?)
- **확장 과제**: 심화 구현이나 비교 실험
```

### 5. .gitignore 생성

`C:\Users\crust\Documents\AI-practice\_templates\cpp\.gitignore` 를 복사합니다.

### 6. CATALOG.md 업데이트

`C:\Users\crust\Documents\AI-practice\CATALOG.md` 에서 해당 practice 행의 상태를 `⚪` → `🔵` 로 변경하고 Phase 수와 모듈 수를 기입합니다.

---

## 품질 기준

- README·CLAUDE·LEARNING_GUIDE 세 파일 모두 작성 완료
- 모든 Phase 폴더와 .gitkeep 존재
- LEARNING_GUIDE에 모든 모듈의 학습 교안 포함
- CATALOG.md 업데이트 완료
