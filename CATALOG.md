# AI-practice 전체 카탈로그

이 저장소는 CS(Computer Science) 분야별 학습 실습 공간입니다.  
새 practice를 만들 때는 `_prompts/` 의 프롬프트를, 파일 뼈대는 `_templates/` 를 사용합니다.

---

## 상태 범례

| 아이콘 | 의미 |
| --- | --- |
| ✅ | 코드 구현 완료 (모듈 다수 포함) |
| 🔵 | 폴더·문서 설정 완료, 구현 대기 |
| ⚪ | 계획만 있음 (폴더 미생성) |

---

## 현재 실습 목록

### 시스템 프로그래밍

| 디렉토리 | 언어 | 상태 | 모듈 수 | 요약 |
| --- | --- | --- | --- | --- |
| `practice-Win32` | C++ | ✅ | 44 | Win32 API, COM, 보안, Winsock, IPC, 디버깅, Hook |
| `practice-OpertingSystem` | C++ | 🔵 | 0/20 | 프로세스·스레드·메모리·동기화·I/O·IPC 심화 |
| `practice-Network` | C++ | 🔵 | 0/18 | Winsock2, IO 모델, IOCP, 프로토콜 구현, TLS |

### 그래픽스

| 디렉토리 | 언어 | 상태 | 모듈 수 | 요약 |
| --- | --- | --- | --- | --- |
| `practice-DirectX11` | C++ | ✅ | 27 | D3D11 렌더링 파이프라인, 셰이더, 텍스처, 조명 |
| `practice-DirectX12` | C++ | ✅ | 27 | D3D12 커맨드 큐, 디스크립터 힙, 동기화 |
| `practice-OpenGL` | C++ | ✅ | 27 | OpenGL 코어, VAO/VBO, GLSL, 프레임버퍼 |
| `practice-Vulkan` | C++ | ✅ | 27 | Vulkan 인스턴스, 렌더패스, 파이프라인, 메모리 |

### .NET / C\#

| 디렉토리 | 언어 | 상태 | 모듈 수 | 요약 |
| --- | --- | --- | --- | --- |
| `practice-dotnet` | C# | ✅ | 46 | C# 기초, OOP, LINQ, 비동기, 컬렉션, 테스팅, HTTP |
| `practice-Database` | C# | 🔵 | 0/20 | ADO.NET, SQLite, EF Core, Dapper, 고급 패턴 |

### Windows 운영

| 디렉토리 | 언어 | 상태 | 모듈 수 | 요약 |
| --- | --- | --- | --- | --- |
| `practice-Windows` | PowerShell | ✅ | 10 | CMD, PowerShell, 파일시스템, 레지스트리, 네트워크 |

---

## 향후 추가 예정 (계획)

아래 주제들은 구현 전 상태입니다. 새로 시작할 때 `_prompts/` 프롬프트를 사용하세요.

### 알고리즘 & 자료구조

| 디렉토리 (예정) | 언어 | 요약 |
| --- | --- | --- |
| `practice-Algorithm` | C++ | 정렬, 탐색, 그리디, DP, 백트래킹, 그래프 알고리즘 |
| `practice-DataStructures` | C++ 또는 C# | 연결 리스트, 트리, 힙, 해시 테이블, 그래프 직접 구현 |

### 소프트웨어 공학

| 디렉토리 (예정) | 언어 | 요약 |
| --- | --- | --- |
| `practice-DesignPatterns` | C# 또는 C++ | GoF 23 패턴 — 생성, 구조, 행동 |
| `practice-Refactoring` | C# | 코드 냄새 식별, 리팩터링 기법, 테스트 보호망 |
| `practice-SystemDesign` | Markdown | 분산 시스템, 캐시, 메시지 큐, API 설계 분석 |

### 저수준 / 이론

| 디렉토리 (예정) | 언어 | 요약 |
| --- | --- | --- |
| `practice-Assembly` | MASM / NASM | x64 레지스터, 스택 프레임, SIMD 기초 |
| `practice-Compiler` | C++ 또는 C# | 렉서·파서·AST·코드 생성 미니 컴파일러 |
| `practice-ComputerArchitecture` | C++ + 시뮬레이터 | 파이프라인, 캐시 구조, 분기 예측 시뮬레이션 |
| `practice-Cryptography` | C++ 또는 C# | 대칭키, 공개키, 해시, TLS 핸드셰이크 직접 구현 |

### GPU / 병렬 컴퓨팅

| 디렉토리 (예정) | 언어 | 요약 |
| --- | --- | --- |
| `practice-CUDA` | C++ + CUDA | GPU 커널, 메모리 계층, 병렬 리덕션 |
| `practice-ComputeShader` | HLSL / GLSL | D3D12 또는 Vulkan Compute shader 실습 |
| `practice-Concurrency` | C++ 또는 C# | Lock-Free, 채널, 작업 기반 병렬, 메모리 모델 |

### 인프라 / DevOps

| 디렉토리 (예정) | 언어 | 요약 |
| --- | --- | --- |
| `practice-Docker` | Dockerfile / PS | 컨테이너 빌드, 네트워크, 볼륨, Compose |
| `practice-Linux` | Bash (WSL) | 쉘 스크립트, 프로세스 관리, 파일시스템, 네트워크 |

---

## 새 practice 만들기

1. `_prompts/` 에서 언어에 맞는 프롬프트 파일을 선택합니다.
2. 프롬프트 상단의 `[변수]` 항목을 채워 에이전트에게 전달합니다.
3. 에이전트가 `_templates/` 를 기반으로 디렉토리를 생성합니다.
4. 이 `CATALOG.md`의 해당 행을 업데이트합니다.

자세한 절차는 `_prompts/README.md` 를 참조합니다.
