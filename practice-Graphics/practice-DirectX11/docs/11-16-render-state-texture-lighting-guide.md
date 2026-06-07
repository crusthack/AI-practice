# DirectX 11 학습 가이드 — 11~16번

Phase 3 (렌더 상태 · 표면 표현)을 다룬다.

---

## 11. Depth Buffer

### 학습 목표
깊이 스텐실 버퍼를 생성하고, 깊이 테스트를 켜고 끄며 3D 오클루전 처리를 이해한다.

### 깊이 버퍼 생성 절차

```cpp
// 1. Depth Stencil Texture 생성
D3D11_TEXTURE2D_DESC descDepth = {};
descDepth.Format  = DXGI_FORMAT_D24_UNORM_S8_UINT;
descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);

// 2. Depth Stencil View 생성
g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);

// 3. RTV + DSV 함께 바인딩
g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
```

### DepthStencilState 토글

```cpp
// ON 상태: 더 가까운 픽셀만 통과
dsDesc.DepthEnable    = TRUE;
dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
dsDesc.DepthFunc      = D3D11_COMPARISON_LESS;

// OFF 상태: 모든 픽셀 통과 (뒤에 있어도 그려짐)
dsDesc.DepthEnable = FALSE;

// 매 프레임 시작 시 깊이 버퍼 초기화 필수
context->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
```

> 깊이 버퍼를 클리어하지 않으면 이전 프레임의 깊이값이 남아 오클루전이 잘못된다.

---

## 12. Texture

### 학습 목표
절차적 Texture2D를 GPU에 업로드하고, SamplerState로 픽셀 셰이더에서 샘플링한다.

### 3-cbuffer 패턴

12번부터 업데이트 빈도에 따라 cbuffer를 분리한다.

| cbuffer | 슬롯 | 업데이트 시점 | 내용 |
|---------|------|--------------|------|
| `cbNeverChanges` | b0 | 초기화 1회 | View 행렬 |
| `cbChangeOnResize` | b1 | 창 크기 변경 시 | Projection 행렬 |
| `cbChangesEveryFrame` | b2 | 매 프레임 | World 행렬 + 색상 |

```cpp
// 렌더 시 View/Proj는 재전송 없이 이미 바인딩된 버퍼를 사용
context->VSSetConstantBuffers(0, 1, &g_pCBNeverChanges);
context->VSSetConstantBuffers(1, 1, &g_pCBChangeOnResize);
context->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame);
context->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);
```

### 텍스처 생성 및 바인딩

```cpp
// CPU 픽셀 데이터 → Texture2D → SRV
D3D11_TEXTURE2D_DESC textureDesc = {};
textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
device->CreateTexture2D(&textureDesc, &initialData, &texture);
device->CreateShaderResourceView(texture, &viewDesc, &shaderResourceView);

// 픽셀 셰이더에 바인딩
context->PSSetShaderResources(0, 1, &g_pTextureRV);
context->PSSetSamplers(0, 1, &g_pSamplerLinear);
```

```hlsl
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

float4 PS(PS_INPUT input) : SV_Target {
    return txDiffuse.Sample(samLinear, input.Tex) * vMeshColor;
}
```

### SamplerState 주요 옵션

| 옵션 | 설명 |
|------|------|
| `FILTER_MIN_MAG_MIP_LINEAR` | 3선형 필터링 (부드러운 확대·축소) |
| `TEXTURE_ADDRESS_WRAP` | UV가 [0,1]을 벗어나면 반복 |
| `TEXTURE_ADDRESS_CLAMP` | UV를 [0,1]로 고정 |

---

## 13. Lighting

### 학습 목표
면별 법선(per-face normal)과 Blinn-Phong 모델로 ambient·diffuse·specular 조명을 구현한다.

### Blinn-Phong 조명 모델

```
최종 색상 = ambient + diffuse + specular
```

```hlsl
// VS: diffuse (Gouraud — 정점에서 계산, 래스터라이저가 보간)
float3 normal = normalize(mul(input.Normal, (float3x3)World));
output.Color  = saturate(dot(normal, lightDir)) * lightColor;
output.Normal   = normal;
output.WorldPos = worldPos.xyz;

// PS: specular (Phong — 픽셀에서 계산, 하이라이트 정확도 높음)
float3 viewDir    = normalize(vCameraPosition.xyz - input.WorldPos);
float3 reflectDir = reflect(-lightDir, normal);
float  specPower  = pow(saturate(dot(viewDir, reflectDir)), 32.0f);
float4 specular   = specPower * vSpecularColor;
return saturate(ambient + diffuse + specular);
```

> **vCameraPosition**: 카메라 Eye 위치를 cbuffer로 전달해야 specular가 시점에 따라 변한다. `g_CameraEye` 전역 변수를 InitDevice에서 저장하고 Render에서 cbuffer에 업로드한다.

### 면별 법선(per-face normal) 구조

```
24정점 큐브: 각 면마다 4개 정점이 독립적으로 존재
각 정점의 Normal = 해당 면의 방향
예) 윗면 4정점 모두 Normal = (0, 1, 0)
```

---

## 14. Material

### 학습 목표
Material 상수(diffuse·specular 배율)를 텍스처 및 조명과 통합해 표면 표현을 확장한다.

### Material 구조

```cpp
struct Material {
    XMFLOAT4 Diffuse  = XMFLOAT4(0.75f, 0.72f, 0.68f, 1.0f);
    XMFLOAT4 Specular = XMFLOAT4(0.03f, 0.03f, 0.03f, 0.0f);
    bool UsesTexture  = true;
};
```

### PS 최종 색상 계산

```hlsl
float4 PS(PS_INPUT input) : SV_Target {
    float4 texColor = txDiffuse.Sample(samLinear, input.Tex);
    // viewDir, reflectDir 계산
    float3 cameraPos = vCameraPosition.xyz;  // cbuffer에서 읽음
    float3 viewDir   = normalize(cameraPos - input.WorldPos);
    float3 lightDir  = normalize(vLightDir[1].xyz);
    float3 reflectDir = reflect(-lightDir, normalize(input.Normal));
    float specPower  = pow(saturate(dot(viewDir, reflectDir)), 32.0f);
    float4 specular  = specPower * vMaterialSpecular;
    return saturate(input.Color * texColor + specular);
}
```

> `input.Color`에는 이미 VS에서 `(ambient + diffuse) × vMaterialDiffuse`가 계산되어 있다. PS에서 텍스처 색상과 곱하고 specular를 더하는 구조다.

### 12번 Texture와의 차이

| | 12. Texture | 14. Material |
|-|-------------|--------------|
| 정점 구조 | Pos + Tex | Pos + Normal + Tex |
| 조명 | 없음 | Blinn-Phong |
| cbuffer | 3개 분리 | 1개 통합 |
| specular | 없음 | vMaterialSpecular로 조절 |

---

## 15. Rasterizer State

### 학습 목표
`ID3D11RasterizerState`로 FillMode(솔리드/와이어프레임)와 CullMode(배면 제거 여부)를 런타임에 전환한다.

### RasterizerState 생성

```cpp
D3D11_RASTERIZER_DESC rasterDesc = {};
rasterDesc.DepthClipEnable = TRUE;  // 항상 TRUE 권장

// 모드 1: 솔리드 + 배면 제거 (기본)
rasterDesc.FillMode = D3D11_FILL_SOLID;
rasterDesc.CullMode = D3D11_CULL_BACK;
g_pd3dDevice->CreateRasterizerState(&rasterDesc, &g_pRasterSolid);

// 모드 2: 와이어프레임
rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
g_pd3dDevice->CreateRasterizerState(&rasterDesc, &g_pRasterWire);

// 모드 3: 솔리드 + 양면 렌더링
rasterDesc.FillMode = D3D11_FILL_SOLID;
rasterDesc.CullMode = D3D11_CULL_NONE;
g_pd3dDevice->CreateRasterizerState(&rasterDesc, &g_pRasterNoCull);
```

```cpp
// 적용
context->RSSetState(activeRasterizer);
```

### 키 입력과 타이틀 피드백

| 키 | 모드 | 윈도우 타이틀 |
|----|------|--------------|
| `1` | Solid / Back-Cull | `[1: Solid / Back-Cull]` |
| `2` | Wireframe | `[2: Wireframe]` |
| `3` | Solid / No Cull | `[3: No Cull]` |

> D3D11 기본 전면 방향은 **시계 방향(CW)**이다. 모드 3(No Cull)에서는 반시계 방향 면도 렌더링되므로 큐브 안쪽 면이 보인다.

---

## 16. Blend State

### 학습 목표
Output Merger 단계에서 알파 블렌딩을 설정하고, 반투명 오브젝트 렌더링의 올바른 처리 방법을 이해한다.

### BlendState 생성

```cpp
D3D11_BLEND_DESC blendDesc = {};
blendDesc.RenderTarget[0].BlendEnable    = TRUE;
blendDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
blendDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
blendDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
// 결과 = src.rgb × src.a + dst.rgb × (1 - src.a)
```

### 반투명 렌더링 올바른 순서

```
1. 불투명 오브젝트 먼저 렌더링 (depth write ON)
2. DepthStencilState: DepthWriteMask = ZERO (depth read-only)
3. 반투명 오브젝트를 카메라에서 먼 것부터 가까운 순으로 렌더링
4. BlendState 복원, DepthStencilState 복원
```

```cpp
// 반투명 오브젝트 그리기 전
context->OMSetDepthStencilState(g_pDepthNoWrite, 0);  // depth write 해제
context->OMSetBlendState(g_pAlphaBlendState, blendFactor, 0xffffffff);

// 그린 후
context->OMSetDepthStencilState(NULL, 0);  // 기본값 복원
context->OMSetBlendState(NULL, blendFactor, 0xffffffff);
```

> 이 데모에서는 두 큐브의 back-to-front 정렬을 생략한다. 완전한 투명도 구현을 위해서는 카메라와의 거리 기준 정렬이 필요하다.

### alpha 값을 cbuffer로 전달

```cpp
// C++ 구조체
struct ConstantBuffer {
    XMMATRIX mWorld, mView, mProjection;
    XMFLOAT4 vBlendParams;  // x = source alpha
};

// Render()
cb.vBlendParams = XMFLOAT4(g_BlendLesson.SourceAlpha, 0, 0, 0);
```

```hlsl
// PS
return float4(input.Color.rgb, vBlendParams.x);
```
