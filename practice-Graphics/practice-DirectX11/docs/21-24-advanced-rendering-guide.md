# DirectX 11 학습 가이드 — 21~24번

Phase 5 고급 렌더 패스를 다룬다.

---

## 21. Post Processing

### 학습 목표
렌더링된 백버퍼를 중간 텍스처로 복사하고 풀스크린 쿼드에서 셰이더 효과(Grayscale)를 적용한다.

### 백버퍼 캡처

```cpp
// 1. 백버퍼에서 Texture2D 레퍼런스 획득
backRTV->GetResource(&backResource);
backResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&backTexture);

// 2. 같은 크기의 BIND_SHADER_RESOURCE 텍스처 생성
postDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
device->CreateTexture2D(&postDesc, nullptr, &g_stagePostTexture);
device->CreateShaderResourceView(g_stagePostTexture, nullptr, &g_stagePostSRV);

// 3. 복사
context->CopyResource(g_stagePostTexture, backTexture);
```

> `CopyResource`는 같은 포맷·크기여야 하며 GPU 내부 복사라 CPU로 데이터를 내리지 않아 빠르다.

### Grayscale 셰이더

```hlsl
float4 PSGrayscale(PSI i) : SV_Target {
    float4 c = SceneTexture.Sample(LinearSampler, i.Uv);
    // NTSC 밝기 가중치 (사람 눈의 R/G/B 민감도)
    float g = dot(c.rgb, float3(0.299f, 0.587f, 0.114f));
    return float4(g, g, g, c.a);
}
```

### 렌더 흐름

```
1. main.cpp: 3D 씬을 백버퍼에 정상 렌더링
2. LearningStage: CaptureBackBufferForPost() → CopyResource
3. LearningStage: OMSetDepthStencilState(NoDepth) — 깊이 테스트 비활성화
4. LearningStage: Draw(fullscreenQuad, PSGrayscale) — 그레이스케일 덮어쓰기
5. LearningStage: Draw(insetQuad, PSPassthrough) — 원본 인셋 미리보기
6. 상태 복원
```

---

## 22. Shadow Mapping

### 학습 목표
광원 시점에서 깊이 맵을 생성하고 메인 패스에서 그 깊이 맵을 샘플링해 그림자를 적용한다.

### Shadow Map 생성

```cpp
// R24G8_TYPELESS: DSV(깊이 쓰기)와 SRV(읽기) 양쪽에 바인딩 가능
tex.Format    = DXGI_FORMAT_R24G8_TYPELESS;
tex.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

// DSV 뷰: D24_UNORM_S8_UINT
dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

// SRV 뷰: R24_UNORM_X8_TYPELESS (24비트 깊이를 float[0,1]로 읽음)
srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
```

### Shadow Pass (광원 시점에서 깊이 기록)

```cpp
// 광원 View/Proj 행렬로 cbuffer 교체
ctx->VSSetConstantBuffers(0, 1, &g_stageLightCB);

// 1024×1024 뷰포트로 전환, 색상 RT 없이 깊이만 기록
ctx->RSSetViewports(1, &shadowVP);
ctx->OMSetRenderTargets(0, nullptr, g_stageShadowDSV);
ctx->ClearDepthStencilView(g_stageShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
ctx->PSSetShader(nullptr, nullptr, 0);  // 픽셀 셰이더 없음 (깊이만 필요)
ctx->DrawIndexed(36, 0, 0);

// 복원
ctx->OMSetRenderTargets(1, &backRTV, depthDSV);
```

### Shadow Test (메인 패스 PS)

```hlsl
// VS에서 그림자 좌표 계산
output.ShadowPos = mul(worldPos, LightViewProj);

// PS에서 비교
float3 projected = input.ShadowPos.xyz / input.ShadowPos.w;
float2 shadowUv  = float2( projected.x *  0.5f + 0.5f,
                           -projected.y *  0.5f + 0.5f);  // y축 반전
if (shadowUv.x in [0,1] && shadowUv.y in [0,1]) {
    float shadowDepth = ShadowMap.Sample(ShadowSampler, shadowUv).r;
    float currentDepth = projected.z;
    // depth bias 0.004: shadow acne 방지
    shadow = (currentDepth - 0.004f > shadowDepth) ? 0.45f : 1.0f;
}
```

> **Shadow Acne**: 깊이 정밀도 한계로 자기 자신에게 그림자가 생기는 현상. depth bias(0.004f) 로 완화한다.
>
> **PCF(Percentage Closer Filtering)**: 이 데모는 단일 샘플 이진 판별만 구현한다. 부드러운 그림자 경계를 위해서는 주변 여러 픽셀 깊이를 샘플링하는 PCF가 필요하다.

---

## 23. Skybox

### 학습 목표
역뷰행렬로 픽셀별 월드 방향 벡터를 재구성하고 TextureCube를 샘플링해 배경 스카이박스를 렌더링한다.

### TextureCube 생성

```cpp
// ArraySize=6, D3D11_RESOURCE_MISC_TEXTURECUBE 플래그
D3D11_TEXTURE2D_DESC tex = {};
tex.ArraySize  = 6;
tex.MiscFlags  = D3D11_RESOURCE_MISC_TEXTURECUBE;

// SRV
srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
```

### 풀스크린 쿼드 + 역뷰행렬 방식

```hlsl
cbuffer SkyboxCB : register(b2) {
    matrix InvView;
    float4 ProjScale;  // .x = aspect*tan(FoV/2), .y = tan(FoV/2)
}

PSI VS(VSI i) {
    PSI o;
    o.Pos = float4(i.Pos, 1.0f);  // z=0.9999, NDC 공간 직접 배치
    // UV로부터 뷰 공간 레이 계산
    float3 viewRay = float3(
        (i.Uv.x * 2.0f - 1.0f) * ProjScale.x,
        -(i.Uv.y * 2.0f - 1.0f) * ProjScale.y,
        1.0f);
    o.Dir = mul(float4(viewRay, 0.0f), InvView).xyz;
    return o;
}

float4 PS(PSI i) : SV_Target {
    return SkyboxTexture.Sample(SkyboxSampler, normalize(i.Dir));
}
```

### 깊이 테스트 설정

```cpp
D3D11_DEPTH_STENCIL_DESC dsd = {};
dsd.DepthEnable    = TRUE;
dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  // 깊이 쓰기 없음
dsd.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL;   // 0.9999 ≤ 초기화값 1.0 → 통과
```

> 스카이박스 쿼드의 NDC z = 0.9999이고 깊이 버퍼는 1.0으로 초기화되어 있다. `LESS_EQUAL` 테스트를 통과하면서, 이미 그려진 오브젝트(z < 0.9999)에는 가려진다. depth write를 끄면 스카이박스 위에 이후 오브젝트를 정상적으로 그릴 수 있다.

---

## 24. Deferred Rendering

### 학습 목표
지오메트리 패스에서 G-Buffer(Multiple Render Targets)를 채우고, 별도 조명 패스에서 G-Buffer를 읽어 최종 조명을 계산한다.

### G-Buffer 구성

| 슬롯 | 포맷 | 내용 |
|------|------|------|
| SV_Target0 (Albedo) | `R8G8B8A8_UNORM` | 재질 색상(diffuse) |
| SV_Target1 (Normal) | `R16G16B16A16_FLOAT` | 월드 법선 (×0.5+0.5 로 [0,1] 인코딩) |
| SV_Target2 (Depth) | `R8G8B8A8_UNORM` | 뷰 공간 깊이 (saturate(z/100)) |

> Normal에 16비트 float를 사용하는 이유: 법선 방향 정밀도가 낮으면 조명 계산 시 아티팩트가 발생한다. Depth에 8비트를 사용하는 것은 데모 단순화 목적이며, 프로덕션은 R16F 이상을 권장한다.

### 지오메트리 패스 (PSGBuffer)

```hlsl
GBUFFER_OUTPUT PSGBuffer(PS_INPUT input) {
    GBUFFER_OUTPUT output;
    output.Albedo = vMaterialDiffuse;
    output.Normal = float4(normalize(input.Normal) * 0.5f + 0.5f, 1.0f);
    output.Depth  = float4(input.ViewDepth, input.ViewDepth, input.ViewDepth, 1.0f);
    return output;
}
```

### 조명 패스 (PSLighting)

```hlsl
float4 PSLighting(PS_INPUT input) : SV_Target {
    float3 albedo = GAlbedo.Sample(sampler, uv).rgb;
    float3 normal = GNormal.Sample(sampler, uv).rgb * 2.0f - 1.0f;
    float  depth  = GDepth.Sample(sampler, uv).r;

    if (depth <= 0.001f) return background_color;  // 빈 픽셀

    float diffuse0 = saturate(dot(normalize(normal), lightDir0));
    float diffuse1 = saturate(dot(normalize(normal), lightDir1));
    float3 lit = albedo * (ambient + diffuse0 * 0.72f + diffuse1 * ...);
    return float4(lit, 1.0f);
}
```

### Forward vs Deferred 비교

| | Forward Rendering | Deferred Rendering |
|-|-------------------|-------------------|
| 조명 계산 | 오브젝트마다 모든 빛 계산 | G-Buffer 채운 뒤 빛별 1회 |
| 빛이 많을 때 | O(오브젝트 × 빛) | O(해상도 × 빛) |
| 반투명 처리 | 쉬움 | 별도 Forward 패스 필요 |
| MSAA | 직접 사용 가능 | 복잡함 |
| 메모리 | 적음 | G-Buffer 추가 |

### 렌더 흐름

```
1. ApplyStageSpecificBeginGBuffer  → 3 RTV 바인딩, 클리어
2. PSGBuffer로 큐브 그리기         → G-Buffer 채움
3. ApplyStageSpecificRender
   a. PSLighting으로 풀스크린 쿼드  → 최종 조명 계산 → 백버퍼
   b. PSGBufferPreview 쿼드         → 하단 G-Buffer 미리보기 스트립
4. 백버퍼 + DSV 복원
5. 라이트 인디케이터 큐브 (PSSolid) 직접 렌더링
```
