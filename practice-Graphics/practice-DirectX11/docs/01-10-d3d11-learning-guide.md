# DirectX 11 학습 가이드 — 01~10번

Phase 1 (최소 실행 구조)과 Phase 2 (파이프라인·행렬·카메라)를 다룬다.

---

## 01. Dx11Basic

### 학습 목표
D3D11 장치를 만들고 화면을 지우는 가장 작은 프로그램을 작성한다.

### 핵심 개념

```
Device           GPU와 통신하는 논리적 인터페이스
DeviceContext    GPU에 명령을 기록·제출하는 채널
SwapChain        백 버퍼 ↔ 프론트 버퍼 교환 체인
RenderTargetView 렌더 결과를 쓸 텍스처의 바인딩 뷰
```

### 초기화 순서

```cpp
D3D11CreateDeviceAndSwapChain(...)   // Device + SwapChain 동시 생성
g_pSwapChain->GetBuffer(...)         // 백 버퍼 텍스처 획득
g_pd3dDevice->CreateRenderTargetView(...)  // RTV 생성
g_pImmediateContext->OMSetRenderTargets(1, &RTV, nullptr)
g_pImmediateContext->RSSetViewports(1, &vp)
```

### 렌더 루프

```cpp
g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
g_pSwapChain->Present( 1, 0 );  // 1 = vsync ON
```

> `Present(1, 0)` — 첫 번째 인자는 수직 동기화 간격. 1이면 디스플레이 주사율에 맞춰 60fps로 제한된다.

---

## 02. HelloTriangle

### 학습 목표
버텍스 버퍼, 셰이더, InputLayout을 연결해 첫 번째 삼각형을 그린다.

### 렌더링 파이프라인 최소 구성

```
CPU 정점 배열
  → Vertex Buffer (GPU 메모리)
  → Input Assembler (IASetVertexBuffers, IASetInputLayout, IASetPrimitiveTopology)
  → Vertex Shader (float3 → float4)
  → Rasterizer
  → Pixel Shader (고정 색상 반환)
  → Output Merger (RTV에 기록)
```

### 셰이더와 InputLayout 연결

```cpp
// HLSL
struct VS_INPUT { float3 Pos : POSITION; };
PS_INPUT VS(VS_INPUT input) {
    output.Pos = float4(input.Pos, 1.0f);  // w=1 명시
}

// C++ InputLayout — HLSL 시맨틱과 반드시 일치해야 함
D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
```

> `float3 Pos`를 버텍스 버퍼로 보내고 셰이더에서 `float4(input.Pos, 1.0f)`로 확장하는 것이 표준 패턴이다. 하드웨어의 암묵적 w=1 채움에 의존하지 않는다.

---

## 03. Shaders

### 학습 목표
정점 간 색상 보간(Gouraud shading 원리)을 이해하고, DYNAMIC 버퍼로 CPU에서 GPU 데이터를 매 프레임 갱신한다.

### 색상 보간 원리

```hlsl
// 래스터라이저가 두 정점 사이의 픽셀에 대해
// 각 속성(Color)을 무게 중심으로 보간한다
PS_INPUT VS(VS_INPUT input) {
    output.Color = input.Color;  // 그대로 전달 → 래스터라이저가 보간
}
float4 PS(PS_INPUT input) : SV_Target {
    return input.Color;  // 보간된 값 사용
}
```

### DYNAMIC 버퍼 갱신

```cpp
// 생성 시
bd.Usage = D3D11_USAGE_DYNAMIC;
bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

// 매 프레임 갱신
D3D11_MAPPED_SUBRESOURCE mapped;
context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
memcpy(mapped.pData, newVertices, size);
context->Unmap(vertexBuffer, 0);
```

> `D3D11_MAP_WRITE_DISCARD` — 버퍼 전체를 버리고 새 메모리를 받아 GPU 파이프라인과의 충돌을 피한다. 이 패턴은 05번 Constant Buffer에서 `UpdateSubresource`로 더 명확하게 정리된다.

---

## 04. Index Buffer

### 학습 목표
인덱스 버퍼로 정점을 재사용해 쿼드(사각형)를 그린다.

### 정점 재사용

```
정점 4개 (TL, TR, BR, BL)
인덱스 6개: 0,1,2  0,2,3  → 삼각형 2개
```

```cpp
// Index buffer 생성 (WORD = uint16)
bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
// ...
context->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
context->DrawIndexed(6, 0, 0);  // 6개 인덱스
```

> `DXGI_FORMAT_R16_UINT` (WORD) — 인덱스 65535 이하면 충분. 대형 메시는 `R32_UINT`를 사용한다.

---

## 05. Constant Buffer

### 학습 목표
CPU 데이터를 상수 버퍼로 GPU에 업로드해 셰이더에서 읽는다.

### cbuffer 설계 원칙

```hlsl
// b0: VS용, b1: PS용 (VS와 PS의 슬롯 공간은 독립적)
cbuffer VSConstantBuffer : register(b0) { float4 Offset; }
cbuffer PSConstantBuffer : register(b1) { float4 TintColor; }
```

```cpp
// C++ 업로드
context->UpdateSubresource(g_pVSConstantBuffer, 0, NULL, &vsConstants, 0, 0);
context->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer);
context->UpdateSubresource(g_pPSConstantBuffer, 0, NULL, &psConstants, 0, 0);
context->PSSetConstantBuffers(1, 1, &g_pPSConstantBuffer);
```

> **16바이트 정렬**: 상수 버퍼 구조체 크기는 16의 배수여야 한다. `XMFLOAT4` 하나 = 16바이트. `XMMATRIX` 하나 = 64바이트. 크기가 맞지 않으면 패딩을 추가한다.

---

## 06. Render Pipeline

### 학습 목표
IA → VS → RS → PS → OM 각 단계가 파이프라인에서 어디에 해당하는지 시각적으로 확인한다.

### 파이프라인 단계 요약

| 단계 | 역할 | 설정 API |
|------|------|----------|
| IA (Input Assembler) | 정점·인덱스 버퍼 읽기, 토폴로지 결정 | `IASetVertexBuffers`, `IASetIndexBuffer`, `IASetPrimitiveTopology` |
| VS (Vertex Shader) | 정점 변환 (Local → Clip space) | `VSSetShader`, `VSSetConstantBuffers` |
| RS (Rasterizer) | 삼각형 → 픽셀, CullMode/FillMode | `RSSetViewports`, `RSSetState` |
| PS (Pixel Shader) | 픽셀 색상 계산 | `PSSetShader`, `PSSetShaderResources` |
| OM (Output Merger) | 깊이 테스트, 블렌딩, RTV에 기록 | `OMSetRenderTargets`, `OMSetDepthStencilState` |

> 이 프로젝트는 행렬 없이 클립 공간 직접 좌표를 사용해 파이프라인 흐름 자체에 집중한다. 행렬 변환은 07번부터 도입한다.

---

## 07. Coordinate Spaces

### 학습 목표
Local → World → View → Projection → NDC → Screen 변환을 4개 뷰포트에서 직접 비교한다.

### 좌표 공간 체인

```
Local Space   : 메시 자체 좌표 (모델 중심 = 원점)
World Space   : World 행렬 적용 후 (씬 안의 위치·방향·스케일)
View Space    : View 행렬 적용 후 (카메라 기준 좌표)
Clip Space    : Projection 행렬 적용 후 (원근 나누기 전)
NDC           : Clip ÷ w ([-1,1] 범위)
Screen Space  : 뷰포트 변환 후 픽셀 좌표
```

### WVP 행렬 합성

```hlsl
float4 worldPos = mul(float4(input.Pos, 1.0f), World);
float4 viewPos  = mul(worldPos, View);
float4 clipPos  = mul(viewPos,  Projection);
output.Pos = clipPos;
```

> **DirectXMath는 행 우선(Row-major)**: `mul(v, M)` = 행 벡터 × 행렬. HLSL 기본은 열 우선이므로 `UpdateSubresource` 전에 반드시 `XMMatrixTranspose`를 적용해야 한다.

---

## 08. Matrix

### 학습 목표
SRT(Scale·Rotate·Translate) 행렬 조합 순서가 결과에 미치는 영향을 두 큐브로 비교한다.

### 행렬 조합 순서

```cpp
// DirectXMath는 행 우선: 왼쪽이 먼저 적용됨
g_World2 = mScale * mSpin * mTranslate * mOrbit;
// 순서: Scale → 자전(Spin) → 평행이동(Translate) → 공전(Orbit)
```

> 조합 순서를 바꾸면 완전히 다른 결과가 나온다. 예) `mTranslate * mSpin`은 원점에서 자전 후 이동하지만, `mSpin * mTranslate`는 이미 이동한 위치에서 자전한다.

---

## 09. Camera

### 학습 목표
LookAt으로 View 행렬을 재구성하는 궤도 카메라를 구현한다.

### 구면 좌표계 카메라

```cpp
// yaw(좌우), pitch(상하), distance(거리)로 카메라 위치 계산
XMVECTOR Eye = XMVectorSet(
    sinf(yaw) * cosf(pitch) * distance,
    sinf(pitch) * distance + 0.5f,
   -cosf(yaw) * cosf(pitch) * distance,
    0.0f);
g_View = XMMatrixLookAtLH(Eye, target, up);
```

```
조작키:
  ←→ 화살표   yaw (좌우 공전)
  ↑↓ 화살표   pitch (상하 앙각)
  W / S        distance (줌 인/아웃)
```

> `pitch` 범위를 `±(π/2 - 0.05f)`로 클램프해 짐벌 락(극지방 뒤집힘)을 방지한다.

---

## 10. 3D Cube

### 학습 목표
24정점 큐브(면별 독립 정점)와 엣지 와이어프레임을 함께 렌더링해 메시 구조를 시각화한다.

### 8정점 vs 24정점 큐브

| 방식 | 정점 수 | 장단점 |
|------|---------|--------|
| 8정점 공유 | 8 | 법선·UV가 면마다 다를 수 없음 (조명·텍스처 불가) |
| 24정점 면별 독립 | 24 | 면마다 법선·UV 독립 지정 가능 (12번 Texture, 13번 Lighting 필수) |

### 엣지 와이어프레임 오버레이

```cpp
// 큐브를 1.015배 확대해 z-fighting 방지
cb.mWorld = XMMatrixTranspose(XMMatrixScaling(1.015f, 1.015f, 1.015f) * g_World);
context->IASetIndexBuffer(g_pEdgeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
context->DrawIndexed(24, 0, 0);  // 12 엣지 × 2 정점
```
