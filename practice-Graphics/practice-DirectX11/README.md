# DirectX 11 Learning Samples

DirectX 11을 단계적으로 익히기 위한 Visual Studio C++ 샘플 모음이다. 27개 프로젝트가 기초 장치 초기화부터 Deferred Rendering까지 점진적으로 난이도를 높인다.

---

## 빠른 시작

Visual Studio에서 `PracticeD3D11.slnx`를 열고 원하는 번호의 프로젝트를 **시작 프로젝트**로 설정한 뒤 `F5`로 실행한다.

명령줄 빌드:

```powershell
msbuild "PracticeD3D11.slnx" /p:Configuration=Debug /p:Platform=x64
```

---

## 구조

### 모든 프로젝트 공통 흐름

```
wWinMain
  ├─ InitWindow       윈도우 등록·생성
  ├─ InitDevice       D3D11 디바이스, 스왑체인, 렌더 타깃, 셰이더, 버퍼 생성
  │     └─ [ApplyStageSpecificSetup]   ← 17번 이후에만 존재
  ├─ message loop
  │     └─ Render()
  │           ├─ [UpdateStageSpecificDemo]
  │           ├─ ClearRenderTargetView / ClearDepthStencilView
  │           ├─ UpdateSubresource → Draw
  │           └─ [ApplyStageSpecificRender]
  │                 Present( 1, 0 )   ← vsync ON
  └─ CleanupDevice
        └─ [ApplyStageSpecificCleanup]
```

### LearningStage.h 패턴

06번과 17~27번 프로젝트는 `LearningStage.h`를 포함한다. 이 파일에 해당 회차의 고유 리소스·렌더 로직을 분리해 `main.cpp` 구조를 항상 동일하게 유지한다.

```
main.cpp          기본 큐브 씬 + 보일러플레이트 (변하지 않음)
LearningStage.h   해당 회차의 추가 리소스/렌더 로직
```

### 공통 설계 원칙

| 원칙 | 내용 |
|------|------|
| 독립 빌드 | 각 프로젝트가 `.vcxproj` 단독으로 빌드·실행 가능 |
| 인라인 셰이더 | HLSL을 `R"(...)"` 문자열로 main.cpp에 포함. `.hlsl` 파일 없음 |
| 인라인 에셋 | 텍스처(절차적), OBJ(임베디드), 큐브맵(절차적) — 외부 파일 의존 없음 |
| 런타임 컴파일 | 모든 셰이더를 `D3DCompile()`로 런타임 컴파일 |
| vsync | `Present( 1, 0 )` — 모든 프로젝트 60fps 제한 |
| 명시적 w=1 | VS_INPUT에서 `float3 Pos`, VS에서 `float4(input.Pos, 1.0f)` 명시 |

---

## 전체 커리큘럼

### Phase 1 — DirectX11 최소 실행 구조 (01~05)

| # | 프로젝트 | 핵심 개념 | 주요 API |
|---|----------|-----------|----------|
| 01 | Dx11Basic | Device · SwapChain · RenderTargetView · Present | `D3D11CreateDeviceAndSwapChain`, `ClearRenderTargetView` |
| 02 | HelloTriangle | Vertex Buffer · Input Layout · Shader · Draw | `IASetVertexBuffers`, `Draw(3,0)` |
| 03 | Shaders | 정점 색상 보간 · DYNAMIC 버퍼 · Map/Unmap | `D3DCompile`, `Map`, `Unmap` |
| 04 | Index Buffer | Index Buffer · DrawIndexed · 정점 재사용 | `IASetIndexBuffer`, `DrawIndexed` |
| 05 | Constant Buffer | cbuffer · UpdateSubresource · VS/PS 각각 | `VSSetConstantBuffers`, `PSSetConstantBuffers` |

> **03번 참고**: `D3D11_USAGE_DYNAMIC` + `Map`/`Unmap` 패턴은 05번 Constant Buffer에서 정식으로 다루는 GPU 업로드 기법의 미리보기다.

---

### Phase 2 — 파이프라인 이해 · 행렬 · 카메라 (06~10)

| # | 프로젝트 | 핵심 개념 | 비고 |
|---|----------|-----------|------|
| 06 | Render Pipeline | IA→VS→RS→PS→OM 단계 시각화 | LearningStage.h 사용. 행렬 없이 클립 공간 쿼드만 사용 |
| 07 | Coordinate Spaces | Local·World·View·Proj 4-뷰포트 비교 | `RSSetViewports`, 뷰포트별 다른 WVP 행렬 |
| 08 | Matrix | SRT 행렬 조합 순서 차이 | `XMMatrixScaling/RotationZ/Translation/RotationY` 조합 |
| 09 | Camera | 구면 좌표계 궤도 카메라 | `GetAsyncKeyState`, 화살표/W/S 키 조작 |
| 10 | 3D Cube | 8정점 큐브 + 엣지 와이어프레임 | `LINELIST` 토폴로지, 별도 edge index buffer |

> **07 / 08 순서**: 07에서 좌표 공간 개념(Local→World→View→Proj의 WHY)을 먼저 파악한 뒤 08에서 SRT 행렬 합성(HOW)을 학습한다.

---

### Phase 3 — 렌더 상태 · 표면 표현 (11~16)

| # | 프로젝트 | 핵심 개념 | 주요 API |
|---|----------|-----------|----------|
| 11 | Depth Buffer | Depth Stencil State · SPACE 키 토글 | `ID3D11DepthStencilState`, `OMSetDepthStencilState` |
| 12 | Texture | Texture2D · SRV · SamplerState · UV 좌표 | 3-cbuffer(NeverChanges/OnResize/EveryFrame), `PSSetShaderResources` |
| 13 | Lighting | per-face Normal · Blinn-Phong · 2 lights | VS diffuse + PS specular, g_CameraEye 전역 변수 |
| 14 | Material | Material 상수 + Texture + Lighting 통합 | `vCameraPosition` cbuffer, `vMaterialDiffuse/Specular` |
| 15 | Rasterizer State | FillMode · CullMode · 1/2/3 키 | `ID3D11RasterizerState`, `RSSetState`, 윈도우 타이틀 피드백 |
| 16 | Blend State | SRC_ALPHA/INV_SRC_ALPHA · depth write 해제 | `ID3D11BlendState`, `g_pDepthNoWrite`, SourceAlpha cbuffer 연결 |

> **12번 참고**: 3개 cbuffer로 나누는 이유는 업데이트 빈도 분리(View=한 번, Projection=창 크기 변경 시, World+Color=매 프레임)로 GPU 대역폭을 아끼기 위함이다.

> **16번 참고**: 완전한 투명도 정렬(back-to-front)은 생략됐다. 데모 목적은 blend state 설정 자체에 있다.

---

### Phase 4 — 장면 구조 · 반복 렌더링 (17~19)

| # | 프로젝트 | 핵심 개념 | 비고 |
|---|----------|-----------|------|
| 17 | Model Loading | 인라인 OBJ 파싱 → VB/IB | `v`/`vn`/`f(pos//normal)` 지원. `vt` 생략. 비중복 expand 방식 |
| 18 | Scene | Entity · Transform · MeshRenderer | `RenderSceneEntities` 템플릿, `Transform::World()` |
| 19 | Instancing | per-instance VB 슬롯 1 · DrawIndexedInstanced | `D3D11_INPUT_PER_INSTANCE_DATA`, `vInstancingEnabled` 플래그 |

> **19번 참고**: `SV_InstanceID`는 선언되지만 사용되지 않는다. 실제 인스턴스별 데이터는 버텍스 버퍼 슬롯 1(`D3D11_INPUT_PER_INSTANCE_DATA`)에서 직접 읽힌다.

---

### Phase 5 — 고급 렌더 패스 (20~24)

| # | 프로젝트 | 핵심 개념 | 비고 |
|---|----------|-----------|------|
| 20 | Render To Texture | 오프스크린 RTV → SRV 2패스 | `BIND_RENDER_TARGET\|BIND_SHADER_RESOURCE`, Write/Sample 분리 |
| 21 | Post Processing | 백버퍼 복사 → Grayscale fullscreen pass | `CopyResource`, `PSGrayscale`/`PSPassthrough` |
| 22 | Shadow Mapping | 광원 깊이 패스 → 섀도우 맵 샘플링 | 1024² `R24G8_TYPELESS`, bias=0.004, binary test (PCF 미적용) |
| 23 | Skybox | 절차적 큐브맵 · 역뷰행렬 레이 재구성 | `TextureCube`, z=0.9999 LESS_EQUAL DSS, 풀스크린 쿼드 |
| 24 | Deferred Rendering | G-Buffer MRT → 조명 패스 | 3 RTV (Albedo/Normal/Depth), `PSGBuffer`/`PSLighting`, 미리보기 스트립 |

> **22번 참고**: 그림자 경계가 선명한 이유는 PCF(Percentage Closer Filtering)를 사용하지 않기 때문이다. depth bias 0.004로 shadow acne만 방지한다.

> **24번 참고**: G-Buffer depth는 `R8G8B8A8_UNORM`(8비트)이다. 프로덕션 Deferred Renderer는 `R16F` 이상을 사용한다.

---

### Phase 6 — 도구화와 최종 구조 (25~27)

| # | 프로젝트 | 개요 |
|---|----------|------|
| 25 | ImGui Integration | 런타임 디버그 파라미터 UI overlay 연결 지점 |
| 26 | Mini Engine | ResourceHandle · ShaderManager · RenderQueueItem 미니 엔진 구조 |
| 27 | Final Project | 카메라 · 객체 · 재질 · 노출 파라미터를 통합한 최종 뷰어 |

---

## 키 입력 요약

| 프로젝트 | 키 | 동작 |
|----------|-----|------|
| 09. Camera | ←→ 화살표 | 좌우 궤도 회전 |
| 09. Camera | ↑↓ 화살표 | 상하 피치 조절 |
| 09. Camera | W / S | 줌 인 / 아웃 |
| 11. Depth Buffer | Space | 깊이 테스트 ON/OFF 토글 |
| 15. Rasterizer State | 1 | Solid + Back-cull |
| 15. Rasterizer State | 2 | Wireframe |
| 15. Rasterizer State | 3 | Solid + No cull |

---

## 학습 가이드 문서

각 Phase별 상세 학습 가이드가 `docs/` 폴더에 있다.

| 파일 | 범위 | 주요 내용 |
|------|------|-----------|
| [`01-10-d3d11-learning-guide.md`](docs/01-10-d3d11-learning-guide.md) | Phase 1+2 (01~10) | 장치 초기화, 셰이더, 행렬, 카메라 |
| [`11-16-render-state-texture-lighting-guide.md`](docs/11-16-render-state-texture-lighting-guide.md) | Phase 3 (11~16) | 깊이 버퍼, 텍스처, 조명, 재질, 래스터/블렌드 상태 |
| [`17-20-scene-structure-guide.md`](docs/17-20-scene-structure-guide.md) | Phase 4+5a (17~20) | OBJ 로딩, 씬 그래프, 인스턴싱, RTT |
| [`21-24-advanced-rendering-guide.md`](docs/21-24-advanced-rendering-guide.md) | Phase 5b (21~24) | 포스트 프로세싱, 섀도우 맵, 스카이박스, 디퍼드 렌더링 |
| [`25-27-tooling-final-guide.md`](docs/25-27-tooling-final-guide.md) | Phase 6 (25~27) | ImGui, 미니 엔진, 최종 프로젝트 |

---

## 프로젝트 파일 구성

```
practice-DirectX11/
├── PracticeD3D11.slnx          전체 솔루션
├── 01. Dx11Basic/
│   ├── 01. Dx11Basic.vcxproj   독립 빌드 단위
│   └── main.cpp
├── 02. HelloTriangle/
│   └── ...
│   ...
├── 17. Model Loading/
│   ├── main.cpp
│   └── LearningStage.h         ← 17번부터 존재
├── ...
└── 27. Final Project/
```

각 프로젝트는 `.vcxproj` 하나로 독립 빌드·실행된다. `PracticeD3D11.slnx`는 전체를 한 번에 빌드하기 위한 묶음이다.
