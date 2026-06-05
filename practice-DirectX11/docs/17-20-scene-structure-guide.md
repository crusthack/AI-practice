# DirectX 11 학습 가이드 — 17~20번

Phase 4 (장면 구조·반복 렌더링)와 Phase 5 일부(RTT)를 다룬다.

---

## 17. Model Loading

### 학습 목표
외부 메시 데이터(OBJ 형식)를 파싱해 GPU 버텍스/인덱스 버퍼로 만들고 렌더링한다.

### OBJ 파일 구조

```
v  x y z          위치 정점
vn nx ny nz       법선 벡터
f  p1//n1 p2//n2 p3//n3   삼각형 면 (위치//법선 쌍)
```

이 프로젝트는 `vt`(텍스처 UV)를 지원하지 않는다. 슬래시 사이 텍스처 인덱스는 파싱 시 무시된다.

### Expand 방식 파싱

```cpp
// 각 면 정점마다 새로운 고유 정점 항목 생성 (중복 제거 없음)
mesh.Vertices.push_back({ positions[p-1], normals[n-1] });
mesh.Indices.push_back( static_cast<unsigned short>(mesh.Indices.size()) );
// → 인덱스가 0,1,2,3,... 순서 = 사실상 순차 인덱스
```

> **Expand vs Dedup**: Expand는 구현이 단순하지만 정점을 재사용하지 않는다. 프로덕션 로더는 `unordered_map<VertexKey, uint16>`로 중복 제거 후 실제 재사용 인덱스를 생성한다.

### LearningStage.h 패턴 첫 등장

```
main.cpp               기본 큐브 씬 유지 (변하지 않음)
LearningStage.h        OBJ 파싱 + 별도 메시 GPU 버퍼 + 렌더
```

```cpp
// InitDevice 끝에서 호출
ApplyStageSpecificSetup(g_pd3dDevice, g_pImmediateContext);

// Render() 끝에서 호출 (IA 상태 저장/복원 포함)
ApplyStageSpecificRender(g_pImmediateContext, g_pRenderTargetView, g_pDepthStencilView);
```

파이프라인 상태(VB, IB, 셰이더 등)를 저장하고 복원하기 때문에 main.cpp의 큐브 씬과 충돌하지 않는다.

---

## 18. Scene

### 학습 목표
Entity-Component 개념(Entity, Transform, MeshRenderer)으로 씬 데이터를 구조화하고, 렌더 루프를 엔티티 순회로 추상화한다.

### Entity 구조

```cpp
struct Transform {
    XMFLOAT3 Position, Rotation, Scale;
    XMMATRIX World() const {
        return XMMatrixScaling(Scale.x, Scale.y, Scale.z)
             * XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, Rotation.z)
             * XMMatrixTranslation(Position.x, Position.y, Position.z);
    }
};
struct MeshRenderer {
    unsigned int IndexCount = 36;
    XMFLOAT4 MaterialDiffuse;
};
struct Entity { Transform LocalTransform; MeshRenderer Renderer; };
```

### 엔티티 순회 렌더링

```cpp
template<typename TCB>
void RenderSceneEntities(context, constantBuffer, const TCB& baseConstants) {
    for (const Entity& entity : g_sceneEntities) {
        TCB constants = baseConstants;
        constants.mWorld = XMMatrixTranspose(entity.LocalTransform.World());
        constants.vMaterialDiffuse = entity.Renderer.MaterialDiffuse;
        context->UpdateSubresource(constantBuffer, 0, nullptr, &constants, 0, 0);
        context->DrawIndexed(entity.Renderer.IndexCount, 0, 0);
    }
}
```

> `UpdateSubresource`를 엔티티마다 호출하므로 엔티티 수가 많아지면 병목이 된다. 이것이 19번 Instancing이 해결하는 문제다.

### 라이트 인디케이터 큐브

조명 방향 시각화 큐브는 Entity로 관리하지 않고 Render()에서 직접 그린다. 이는 씬 오브젝트가 아닌 렌더러 인프라이기 때문이다.

---

## 19. Instancing

### 학습 목표
`DrawIndexedInstanced`로 하나의 메시를 여러 번 그리되, GPU가 인스턴스별 데이터(위치·색상)를 버텍스 버퍼에서 직접 읽게 한다.

### 인스턴싱 메커니즘

```
버텍스 버퍼 슬롯 0  : 정점 데이터 (POSITION, NORMAL) — PER_VERTEX_DATA
버텍스 버퍼 슬롯 1  : 인스턴스 데이터 (OFFSET_SCALE, COLOR) — PER_INSTANCE_DATA
                       DrawIndexedInstanced 호출 시 인스턴스마다 1 step 진행
```

### InputLayout 설정

```cpp
D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION",             0, R32G32B32_FLOAT,  0,  0, PER_VERTEX_DATA,   0 },
    { "NORMAL",               0, R32G32B32_FLOAT,  0, 12, PER_VERTEX_DATA,   0 },
    { "INSTANCE_OFFSET_SCALE",0, R32G32B32A32_FLOAT,1,  0, PER_INSTANCE_DATA, 1 },
    { "INSTANCE_COLOR",       0, R32G32B32A32_FLOAT,1, 16, PER_INSTANCE_DATA, 1 },
};
// 마지막 인자 = InstanceDataStepRate (1 = 인스턴스당 1 step)
```

### 드로우 호출 비교

```cpp
// 기존: 엔티티마다 UpdateSubresource + DrawIndexed
for (Entity& e : entities) {
    UpdateSubresource(cb, ...);
    DrawIndexed(36, 0, 0);
}

// 인스턴싱: 한 번의 드로우 호출
DrawIndexedInstanced(
    36,  // 메시 인덱스 수
    3,   // 인스턴스 수
    0, 0, 0
);
```

> `SV_InstanceID`는 VS_INPUT에 선언되어 있지만 이 구현에서는 사용하지 않는다. 실제 인스턴스별 데이터는 버텍스 버퍼 슬롯 1의 `INSTANCE_OFFSET_SCALE`/`INSTANCE_COLOR` 시맨틱에서 직접 읽힌다.

### 인스턴싱 활성화 플래그

```cpp
// 인스턴싱 ON/OFF를 셰이더에서 분기
cbuffer InstanceBuffer : register(b1) { float4 vInstancingEnabled; }

// VS에서
if (vInstancingEnabled.x > 0.5f) {
    localPos.xyz = localPos.xyz * instanceScale + instanceOffset;
    instanceColor = input.InstanceColor;
}
```

---

## 20. Render To Texture

### 학습 목표
오프스크린 렌더 타깃에 씬을 렌더링한 뒤, 그 결과를 텍스처(SRV)로 다음 패스에서 샘플링하는 2패스 렌더링을 구현한다.

### 오프스크린 텍스처 생성

```cpp
D3D11_TEXTURE2D_DESC tex = {};
tex.Width  = 512; tex.Height = 512;
// 렌더 타깃과 셰이더 리소스 둘 다 사용
tex.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
device->CreateTexture2D(&tex, nullptr, &offscreenTexture);

device->CreateRenderTargetView(offscreenTexture,  nullptr, &offscreenRTV);
device->CreateShaderResourceView(offscreenTexture, nullptr, &offscreenSRV);
```

> 같은 텍스처가 RTV(쓰기)와 SRV(읽기) 두 가지 뷰를 가진다. 단, **동시에 둘 다 바인딩할 수 없다**. 패스 전환 시 이전 RTV를 언바인딩해야 한다.

### 2패스 렌더링 흐름

```
패스 1 (Write Pass):
  OMSetRenderTargets → offscreenRTV     오프스크린으로 렌더 타깃 전환
  Draw(fullscreenQuad)                  UV 그라디언트 패턴 기록
  OMSetRenderTargets → backBufferRTV    백 버퍼로 복원

패스 2 (Main Scene):
  PSSetShaderResources(0, offscreenSRV) 오프스크린 텍스처를 샘플러로 바인딩
  Draw(cube)                            큐브 각 면에 오프스크린 텍스처 적용
  PSSetShaderResources(0, nullSRV)      언바인딩

패스 3 (Preview):
  Draw(previewQuad)                     좌측 하단 미리보기 쿼드
```

### 파이프라인 상태 저장/복원

LearningStage.h의 각 함수는 호출 전 파이프라인 상태를 저장하고 복원한다.

```cpp
// 저장
context->IAGetVertexBuffers(0, 1, &prevVB, &prevStride, &prevOffset);
context->VSGetShader(&prevVS, ...);
// ... 렌더링 ...
// 복원
context->IASetVertexBuffers(0, 1, &prevVB, &prevStride, &prevOffset);
context->VSSetShader(prevVS, ...);
if (prevVB) prevVB->Release();  // IAGet이 레퍼런스 카운트 증가시킴
```
