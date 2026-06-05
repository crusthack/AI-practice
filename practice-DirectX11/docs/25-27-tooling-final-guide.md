# DirectX 11 학습 가이드 — 25~27번

Phase 6 (도구화와 최종 구조)를 다룬다.

---

## 25. ImGui Integration

### 학습 목표
런타임 디버그 UI 오버레이 연결 지점을 분리하고, Dear ImGui + D3D11 백엔드 통합 패턴을 이해한다.

### ImGui D3D11 통합 개요

```cpp
// 초기화 (InitDevice 이후)
IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGui_ImplWin32_Init(g_hWnd);
ImGui_ImplDX11_Init(g_pd3dDevice, g_pImmediateContext);

// 프레임 시작 (Render 함수 내)
ImGui_ImplDX11_NewFrame();
ImGui_ImplWin32_NewFrame();
ImGui::NewFrame();

// UI 빌드
ImGui::Begin("Debug");
ImGui::SliderFloat("Param", &value, 0.0f, 1.0f);
ImGui::End();

// 렌더링 (Present 직전)
ImGui::Render();
ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

// 정리
ImGui_ImplDX11_Shutdown();
ImGui_ImplWin32_Shutdown();
ImGui::DestroyContext();
```

### WndProc에 ImGui 메시지 처리 추가

```cpp
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;  // ImGui가 처리한 경우 앱에 전달하지 않음
    // 기존 처리...
}
```

### 학습 포인트

- ImGui 렌더 패스는 메인 씬 렌더 완료 후, `Present` 호출 직전에 위치한다.
- ImGui는 D3D11 파이프라인 상태를 변경하므로 ImGui 렌더 후 상태를 재설정하거나, ImGui가 저장/복원하는 것에 의존한다.
- `LearningStage.h`에서 UI 빌드 코드를 `UpdateStageSpecificDemo(t)` 안에 넣으면 main.cpp 수정 없이 UI를 추가할 수 있다.

---

## 26. Mini Engine

### 학습 목표
ResourceHandle, ShaderManager, RenderQueueItem 같은 미니 엔진 레이어를 구성해 렌더링 인프라를 추상화한다.

### 엔진 레이어 구조

```
Application (main.cpp)
  ↓
MiniEngineContext       초기화·업데이트·렌더·종료 진입점
  ├─ ShaderManager      셰이더 컴파일·캐시·핸들 발급
  ├─ ResourceManager    버퍼/텍스처 생성·해제·핸들 발급
  └─ RenderQueue        DrawCall 항목 축적 → 정렬 → 제출
```

### ResourceHandle 패턴

```cpp
// 불투명 핸들: 내부 구현을 외부에 노출하지 않음
using MeshHandle   = uint32_t;
using ShaderHandle = uint32_t;

MeshHandle   CreateMesh(const SimpleVertex* verts, UINT count, ...);
ShaderHandle CompileShader(const char* source, const char* entry);
void         DestroyMesh(MeshHandle h);
```

### RenderQueueItem

```cpp
struct RenderQueueItem {
    MeshHandle   Mesh;
    ShaderHandle Shader;
    XMMATRIX     World;
    XMFLOAT4     MaterialDiffuse;
    float        DepthKey;  // 정렬 키 (앞→뒤 또는 셰이더별)
};

// 프레임마다: 항목 축적 → 정렬 → 배치 제출
void Submit(const RenderQueueItem& item);
void Flush(ID3D11DeviceContext* context);
```

### 18번 Scene과의 차이

| | 18. Scene | 26. Mini Engine |
|-|-----------|-----------------|
| 데이터 구조 | Entity/Transform/MeshRenderer | ResourceHandle + RenderQueue |
| 셰이더 관리 | 직접 포인터 | ShaderManager 통한 핸들 |
| 드로우 제출 | 즉시 DrawIndexed | 큐에 추가 → Flush 시 제출 |
| 정렬 | 없음 | DepthKey 기준 정렬 가능 |

---

## 27. Final Project

### 학습 목표
카메라 조작, 씬 오브젝트 목록, 재질 편집, 렌더 파라미터를 통합한 인터랙티브 뷰어를 완성한다.

### 뷰어 구성 요소

```cpp
struct FinalViewerState {
    // 카메라
    float CameraYaw   = 0.0f;
    float CameraPitch = 0.2f;
    float CameraDistance = 8.0f;

    // 씬 오브젝트
    std::vector<Entity> Objects;
    int SelectedIndex = 0;

    // 재질 파라미터 (ImGui 슬라이더로 조절)
    XMFLOAT4 MaterialDiffuse  = { 0.7f, 0.7f, 0.9f, 1.0f };
    XMFLOAT4 MaterialSpecular = { 0.3f, 0.3f, 0.3f, 0.0f };
    float     SpecularPower   = 32.0f;

    // 렌더 파라미터
    bool  ShowWireframe = false;
    float AmbientIntensity = 0.1f;
};
```

### 통합 렌더 루프

```
매 프레임:
  1. UpdateCamera(state)              키/마우스 → yaw/pitch/distance 갱신
  2. UpdateSceneObjects(state, t)     Entity 회전 업데이트
  3. ImGui UI 빌드                    슬라이더, 체크박스, 오브젝트 선택
  4. ClearRenderTargetView / ClearDepthStencilView
  5. RSSetState(wireframe ? rasterWire : rasterSolid)
  6. for each entity: UpdateSubresource + DrawIndexed
  7. ImGui::Render() + RenderDrawData
  8. Present(1, 0)
```

### 커리큘럼 통합 포인트

| 개념 | 출처 |
|------|------|
| D3D11 장치/스왑체인 | 01번 |
| WVP 행렬 | 07~08번 |
| 궤도 카메라 | 09번 |
| 텍스처 + 조명 | 13~14번 |
| Rasterizer 전환 | 15번 |
| Entity/Transform | 18번 |
| ImGui 통합 | 25번 |
| ResourceHandle | 26번 |
