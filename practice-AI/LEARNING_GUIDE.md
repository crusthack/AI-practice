# LEARNING_GUIDE — practice-AI

각 모듈의 목표, 핵심 개념, 관찰 포인트, 확장 과제를 정리한 학습 교안입니다.

---

## Phase 1 — Fundamentals

### 01 NumpyBasics

- **목표**: Python에서 수치 계산의 기반인 다차원 배열을 자유롭게 다룬다.
- **핵심 개념**
  - `ndarray` 생성 (`zeros`, `ones`, `arange`, `linspace`, `random`)
  - 인덱싱·슬라이싱·팬시 인덱싱·불린 마스킹
  - 브로드캐스팅 규칙 (shape 정렬)
  - 벡터·행렬 연산 (`dot`, `matmul`, `@` 연산자)
  - 축(axis) 기반 집계 (`sum`, `mean`, `max`, `argmax`)
- **관찰 포인트**: 루프 없이 벡터 연산으로 처리할 때 속도 차이를 `time.time()` 으로 측정해본다.
- **확장 과제**: 이미지를 `numpy` 배열로 불러와 채널 분리, 회전, 크기 조절을 순수 numpy로 구현.

---

### 02 PandasBasics

- **목표**: 표 형식 데이터를 불러오고 정제·변환·집계하는 흐름을 익힌다.
- **핵심 개념**
  - `Series`와 `DataFrame` 생성 및 기본 속성 (`shape`, `dtypes`, `info`, `describe`)
  - CSV 로드 (`read_csv`), 결측치 처리 (`dropna`, `fillna`)
  - 필터링 (`loc`, `iloc`), 정렬 (`sort_values`), 중복 제거
  - 그룹 집계 (`groupby`, `agg`), 피벗 테이블
  - DataFrame 병합 (`merge`, `concat`, `join`)
- **관찰 포인트**: `groupby` + `agg` 조합으로 통계를 한 줄에 뽑는 패턴을 체득한다.
- **확장 과제**: 공개 CSV 데이터(Titanic, Iris)를 불러와 EDA(탐색적 데이터 분석) 리포트 작성.

---

### 03 MatplotlibVisualization

- **목표**: 데이터를 시각적으로 표현하여 분포·추세·관계를 직관적으로 파악한다.
- **핵심 개념**
  - `Figure` / `Axes` 객체 모델, `subplots`
  - 라인 차트, 막대 차트, 히스토그램, 박스플롯, 산점도
  - Seaborn 통합 (`heatmap`, `pairplot`, `violinplot`)
  - 축 레이블·제목·범례·스타일 설정
  - `savefig` 로 PNG 저장
- **관찰 포인트**: 동일 데이터를 히스토그램과 KDE로 비교하여 분포 해석 차이를 확인한다.
- **확장 과제**: 학습 곡선(training loss vs validation loss)을 실시간으로 그리는 Keras 콜백 구현.

---

### 04 ScipyStats

- **목표**: 통계 검정과 최적화의 기초를 이해하고 ML 배경 지식으로 활용한다.
- **핵심 개념**
  - 확률분포 (`norm`, `t`, `chi2`, `f`) — PDF, CDF, PPF
  - 가설 검정: t-test, chi-square test, ANOVA
  - 상관관계 (`pearsonr`, `spearmanr`)
  - 최적화 (`minimize`, `curve_fit`)
  - 보간 (`interp1d`)
- **관찰 포인트**: p-value 해석 연습 — 0.05 기준으로 귀무가설 기각 여부를 직접 판정해본다.
- **확장 과제**: A/B 테스트 시뮬레이션 — 두 그룹의 전환율 차이가 유의한지 검정.

---

## Phase 2 — MachineLearning

### 01 LinearRegression

- **목표**: 회귀 문제의 기본 원리(최소제곱법, 경사 하강법)와 정규화를 이해한다.
- **핵심 개념**
  - OLS (Ordinary Least Squares), 비용 함수 (MSE, RMSE, MAE)
  - 다중 회귀, 다항 회귀 (`PolynomialFeatures`)
  - 정규화: Ridge (L2), Lasso (L1), ElasticNet
  - `train_test_split`, `cross_val_score`, R² 점수
- **관찰 포인트**: 규제 강도(α)를 높일수록 계수가 0에 수렴하는 과정을 관찰한다.
- **확장 과제**: 경사 하강법을 numpy로 직접 구현하고 scikit-learn 결과와 비교.

---

### 02 LogisticRegression

- **목표**: 분류 문제의 기초와 모델 성능 평가 지표를 익힌다.
- **핵심 개념**
  - 시그모이드 함수, 로그 손실(Cross-Entropy)
  - 이진 분류 vs 다중 분류 (OvR, Softmax)
  - 혼동 행렬 (TP, FP, FN, TN), 정밀도·재현율·F1
  - ROC 곡선과 AUC
  - 불균형 데이터 처리 (`class_weight`, SMOTE)
- **관찰 포인트**: 임계값(threshold)을 0.3~0.7 사이로 바꾸며 정밀도-재현율 트레이드오프 확인.
- **확장 과제**: Iris 데이터셋으로 3-class 분류, 각 클래스의 결정 경계 시각화.

---

### 03 DecisionTree

- **목표**: 트리 기반 모델이 분기 기준을 어떻게 결정하는지 이해한다.
- **핵심 개념**
  - 정보 이득(Information Gain), 지니 불순도(Gini Impurity)
  - 최대 깊이(`max_depth`), 최소 샘플(`min_samples_leaf`)로 과적합 제어
  - `export_graphviz` 또는 `plot_tree`로 트리 시각화
  - 회귀 트리 (DecisionTreeRegressor)
- **관찰 포인트**: 깊이 제한 없이 학습 시 train 100% / test 저조한 과적합 현상 재현.
- **확장 과제**: 직접 1-레벨 결정 트루(결정 스텀프)를 구현하고 Gini 계산 검증.

---

### 04 RandomForest

- **목표**: 앙상블이 개별 트리보다 강인한 이유와 Feature Importance 활용법을 익힌다.
- **핵심 개념**
  - 배깅(Bagging), 부트스트랩 샘플링
  - OOB(Out-Of-Bag) 오류율
  - Feature Importance (MDI, Permutation)
  - 하이퍼파라미터 탐색 (`GridSearchCV`, `RandomizedSearchCV`)
- **관찰 포인트**: 트리 수(`n_estimators`)를 10 → 100 → 500으로 늘릴 때 성능·속도 변화 측정.
- **확장 과제**: Gradient Boosting (XGBoost/LightGBM)과 RandomForest 성능 비교.

---

### 05 SVM

- **목표**: 마진 최대화 원리와 커널 트릭으로 비선형 분류를 다룬다.
- **핵심 개념**
  - 서포트 벡터, 하드 마진 vs 소프트 마진 (C 파라미터)
  - 커널 함수: 선형, RBF (가우시안), 다항식
  - SVC (분류), SVR (회귀)
  - `GridSearchCV`로 C, gamma 최적화
- **관찰 포인트**: C가 클수록 결정 경계가 복잡해지고 과적합 위험이 증가함을 시각화로 확인.
- **확장 과제**: 불균형 클래스에서 `class_weight='balanced'` 효과 비교.

---

### 06 KMeansClustering

- **목표**: 레이블 없는 데이터를 군집화하는 비지도 학습의 기초를 익힌다.
- **핵심 개념**
  - K-Means 알고리즘 (초기화 → 할당 → 업데이트 반복)
  - K 선택: 엘보우 법칙(Inertia), 실루엣 점수
  - 초기화 전략: `k-means++`
  - DBSCAN, 계층적 군집화와 비교
- **관찰 포인트**: K를 2~10으로 바꾸며 실루엣 점수 그래프를 그려 최적 K를 찾는다.
- **확장 과제**: 고차원 데이터를 PCA로 2D로 줄인 뒤 군집 결과를 시각화.

---

### 07 PCA

- **목표**: 고차원 데이터를 저차원으로 압축하면서 정보 손실을 최소화하는 방법을 이해한다.
- **핵심 개념**
  - 고유값 분해, 공분산 행렬
  - 주성분(PC) 선택: 누적 설명 분산 비율 (≥ 95%)
  - Scree Plot
  - Whitening, 표준화 전처리 필요성
- **관찰 포인트**: MNIST 784차원 → 50차원 PCA 후 재구성 이미지 품질 비교.
- **확장 과제**: t-SNE와 UMAP을 PCA와 비교하여 비선형 구조 표현 차이 확인.

---

## Phase 3 — DeepLearning

### 01 TensorFlowBasics

- **목표**: TensorFlow의 핵심 추상화(Tensor, Variable, GradientTape)를 이해한다.
- **핵심 개념**
  - `tf.Tensor` vs `tf.Variable` — 불변/가변 차이
  - `tf.GradientTape` 자동 미분, 고차 미분
  - `@tf.function` 그래프 모드 컴파일
  - 데이터 파이프라인: `tf.data.Dataset`
- **관찰 포인트**: `@tf.function` 유무에 따른 첫 실행 지연(trace) vs 반복 실행 속도 비교.
- **확장 과제**: GradientTape로 선형 회귀를 수동 구현 (W, b를 직접 업데이트).

---

### 02 KerasSequential

- **목표**: Keras의 레이어 구성 방식과 학습 루프(compile → fit → evaluate)를 익힌다.
- **핵심 개념**
  - `Dense`, `Dropout`, `BatchNormalization` 레이어
  - 활성화 함수: ReLU, Sigmoid, Softmax, Tanh
  - 손실 함수: MSE, BinaryCrossentropy, CategoricalCrossentropy
  - 옵티마이저: SGD, Adam, RMSprop
  - 콜백: `EarlyStopping`, `ModelCheckpoint`, `ReduceLROnPlateau`
- **관찰 포인트**: 학습률(lr)을 0.1 → 0.001 → 0.0001로 바꾸며 수렴 속도·안정성 비교.
- **확장 과제**: Functional API로 다중 입력 모델 구성 (숫자 + 텍스트 피처 합산).

---

### 03 CNNImageClassification

- **목표**: 합성곱 신경망의 공간 특징 추출 원리와 이미지 분류 파이프라인을 구현한다.
- **핵심 개념**
  - `Conv2D` (filters, kernel_size, padding, strides)
  - `MaxPooling2D`, `GlobalAveragePooling2D`
  - `BatchNormalization`, `Dropout` 정규화
  - Data Augmentation (`ImageDataGenerator`, `tf.keras.layers.RandomFlip`)
  - CIFAR-10 데이터셋으로 10-class 분류
- **관찰 포인트**: Augmentation 유무에 따른 validation accuracy 차이를 에폭별로 기록.
- **확장 과제**: VGG-style 3×3 conv 블록을 직접 쌓아 파라미터 수를 계산.

---

### 04 RNNTextSequence

- **목표**: 순환 신경망이 시퀀스 의존성을 어떻게 포착하는지 이해한다.
- **핵심 개념**
  - `Embedding` 레이어, 워드 인덱싱, 패딩
  - `SimpleRNN`, `Bidirectional(RNN)`
  - 기울기 소실 문제와 해결 방향
  - 텍스트 전처리 (`Tokenizer`, `pad_sequences`)
- **관찰 포인트**: 단방향 vs 양방향 RNN의 정확도 차이를 IMDB 데이터셋으로 비교.
- **확장 과제**: Attention 메커니즘을 수동으로 추가하여 분류 성능 변화 관찰.

---

### 05 LSTMTimeSeries

- **목표**: LSTM/GRU로 시계열 패턴을 학습하고 미래 값을 예측한다.
- **핵심 개념**
  - LSTM 게이트 구조 (Forget, Input, Output)
  - GRU와 LSTM 비교
  - 슬라이딩 윈도우로 입력 시퀀스 구성
  - MinMaxScaler 정규화, 역변환
  - 다단계 예측 (Multi-step Forecast)
- **관찰 포인트**: 시퀀스 길이(window size)를 7 / 30 / 90일로 바꾸며 예측 성능 비교.
- **확장 과제**: Stacked LSTM(3 레이어)과 단층 LSTM 성능·학습 시간 비교.

---

## Phase 4 — AdvancedDL

### 01 TransferLearning

- **목표**: 대형 사전 학습 모델의 지식을 소규모 데이터셋에 전이하는 방법을 익힌다.
- **핵심 개념**
  - Feature Extraction (베이스 모델 동결, 새 헤드만 학습)
  - Fine-tuning (일부 레이어 해동, 낮은 학습률)
  - `VGG16`, `ResNet50`, `MobileNetV2` (ImageNet 사전 학습)
  - `include_top=False`, GlobalAveragePooling2D 연결
- **관찰 포인트**: 동결 레이어 수를 바꾸며 학습 속도와 최종 성능의 트레이드오프 확인.
- **확장 과제**: 자체 이미지 데이터셋(10~20장/클래스)에 MobileNetV2 파인튜닝 적용.

---

### 02 TransformersAttention

- **목표**: Attention 메커니즘과 Transformer 아키텍처의 작동 원리를 이해한다.
- **핵심 개념**
  - Scaled Dot-Product Attention, Multi-Head Attention
  - Positional Encoding
  - Encoder-Decoder 구조, Layer Normalization
  - BERT 사전 학습 개념 (MLM, NSP)
  - `transformers` 라이브러리로 BERT Fine-tuning 맛보기
- **관찰 포인트**: Attention Weight를 Heatmap으로 시각화하여 어떤 토큰에 집중하는지 확인.
- **확장 과제**: Transformer Encoder 블록을 Keras Subclassing으로 직접 구현.

---

### 03 GANBasics

- **목표**: 적대적 생성 신경망의 훈련 역학과 주요 안정화 기법을 이해한다.
- **핵심 개념**
  - Generator / Discriminator 역할과 손실 함수
  - 훈련 루프: G step → D step 교번
  - DCGAN: Conv 기반 아키텍처, BatchNorm 위치
  - Mode Collapse, Vanishing Gradient 문제와 대응책 (Wasserstein Loss, Label Smoothing)
- **관찰 포인트**: MNIST 숫자 생성 과정을 epoch별로 저장하여 품질 향상 시각화.
- **확장 과제**: Conditional GAN (cGAN) — 클래스 레이블을 조건으로 특정 숫자 생성.

---

### 04 AutoEncoder

- **목표**: 인코더-디코더 구조로 데이터 압축·복원·이상 탐지를 수행한다.
- **핵심 개념**
  - Undercomplete AutoEncoder — 병목 잠재 공간(latent space)
  - Sparse AutoEncoder — 희소 정규화
  - Variational AutoEncoder (VAE) — 확률적 잠재 공간, KL Divergence
  - 이상 탐지: 재구성 오류 임계값 설정
- **관찰 포인트**: VAE의 잠재 공간을 2D로 줄여 클래스 군집 분포를 시각화.
- **확장 과제**: 이미지 노이즈 제거(Denoising AutoEncoder) 구현 — 노이즈 있는 MNIST 복원.

---

### 05 ReinforcementLearningBasics

- **목표**: 에이전트-환경 상호작용의 기본 프레임워크와 Q-Learning을 구현한다.
- **핵심 개념**
  - MDP (상태, 행동, 보상, 전이 확률, 할인율 γ)
  - Q-Table 업데이트 (Bellman Equation)
  - Epsilon-Greedy 탐색-활용 균형
  - DQN: 신경망으로 Q-함수 근사, Experience Replay, Target Network
  - `gymnasium` (OpenAI Gym) 환경: FrozenLake, CartPole
- **관찰 포인트**: ε 감소 속도를 빠르게/느리게 바꾸며 에이전트 학습 곡선 비교.
- **확장 과제**: CartPole-v1을 DQN으로 500점 이상 유지하도록 하이퍼파라미터 튜닝.

---

## Phase 5 — Projects

### 01 ImageClassifier

- **목표**: 데이터 수집 → 전처리 → 모델 학습 → 평가 → 추론의 엔드투엔드 파이프라인 완성.
- **구성**
  - 데이터셋: Keras Flowers 또는 자체 수집 이미지
  - 전처리: Resize 224×224, Normalization, Augmentation
  - 모델: Transfer Learning (MobileNetV2) Fine-tuning
  - 평가: Confusion Matrix, Classification Report, Grad-CAM 시각화
- **확장 과제**: REST API (`FastAPI`) 로 모델 서빙 — 이미지 업로드 → 예측 결과 반환.

---

### 02 TextSentimentAnalysis

- **목표**: 텍스트 전처리부터 감성 분류 모델 학습·평가까지 완성한다.
- **구성**
  - 데이터셋: IMDB 50K 영화 리뷰 (Keras 내장)
  - 모델 비교: TF-IDF + LogisticRegression vs Embedding + LSTM vs BERT Fine-tuning
  - 평가: Accuracy, F1-score, 틀린 예측 샘플 분석
- **확장 과제**: 한국어 감성 분류 — Naver 영화 리뷰 데이터셋 적용.

---

### 03 TimeSeriesPrediction

- **목표**: 실세계 시계열 데이터를 전처리하고 여러 모델을 비교해 예측 파이프라인을 완성한다.
- **구성**
  - 데이터셋: Yahoo Finance 주가 또는 기상청 기온 CSV
  - 모델 비교: ARIMA vs LSTM vs Transformer
  - 평가: MAE, RMSE, MAPE, 예측 구간 시각화
- **확장 과제**: 다변량 시계열 (주가 + 거래량 + 뉴스 감성 점수) 입력으로 예측 개선.
