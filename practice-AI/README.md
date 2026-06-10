# AI / 머신러닝 / 딥러닝 학습 저장소

NumPy·Pandas부터 scikit-learn, TensorFlow/Keras, 트랜스포머까지 단계적으로 쌓아 올리는 Python 기반 AI 실습 모음입니다.

> **선행 학습**: Python 기본 문법 (변수, 함수, 클래스, 리스트/딕셔너리)

## 환경 준비

```powershell
# 가상 환경 생성 및 활성화
python -m venv .venv
.\.venv\Scripts\Activate.ps1

# 필수 패키지 설치
pip install -r requirements.txt
```

## 빠른 시작

```powershell
# 예시 — Phase1 NumPy 기초
python Phase1_Fundamentals\01_NumpyBasics\main.py
```

## 문서 구조

| 문서 | 용도 |
| --- | --- |
| `README.md` | 저장소 개요, 환경 설정, 전체 목차 |
| `CLAUDE.md` | 에이전트 작업 지침 및 컨벤션 |
| `LEARNING_GUIDE.md` | 모듈별 학습 교안, 관찰 포인트, 확장 과제 |

## 전체 로드맵

| Phase | 주제 | 모듈 수 |
| --- | --- | --- |
| Phase1_Fundamentals | Python 과학 라이브러리 기초 (NumPy, Pandas, Matplotlib) | 4 |
| Phase2_MachineLearning | scikit-learn 지도·비지도 학습 | 7 |
| Phase3_DeepLearning | TensorFlow / Keras 기초 + CNN / RNN / LSTM | 5 |
| Phase4_AdvancedDL | Transfer Learning, Transformer, GAN, AutoEncoder, RL | 5 |
| Phase5_Projects | 엔드투엔드 실전 프로젝트 | 3 |

## 모듈 목차

### Phase 1 — Fundamentals (Python 과학 라이브러리)

| # | 모듈 | 핵심 개념 |
| --- | --- | --- |
| 01 | NumpyBasics | 배열 생성·인덱싱·브로드캐스팅·벡터·행렬 연산 |
| 02 | PandasBasics | DataFrame·Series·데이터 정제·집계·Merge |
| 03 | MatplotlibVisualization | 라인·바·히스토그램·산점도·Seaborn |
| 04 | ScipyStats | 확률분포·통계 검정·최적화 |

### Phase 2 — MachineLearning (scikit-learn)

| # | 모듈 | 핵심 개념 |
| --- | --- | --- |
| 01 | LinearRegression | 단순·다중 회귀·Ridge·Lasso·정규화 |
| 02 | LogisticRegression | 이진·다중 분류·혼동 행렬·ROC·AUC |
| 03 | DecisionTree | CART·과적합·가지치기·시각화 |
| 04 | RandomForest | 앙상블·배깅·Feature Importance |
| 05 | SVM | 선형·RBF 커널·SVC·SVR·마진 |
| 06 | KMeansClustering | K-Means·엘보우 법칙·실루엣 점수 |
| 07 | PCA | 차원 축소·설명 분산 비율·시각화 |

### Phase 3 — DeepLearning (TensorFlow/Keras)

| # | 모듈 | 핵심 개념 |
| --- | --- | --- |
| 01 | TensorFlowBasics | Tensor·GradientTape·자동 미분·그래프 모드 |
| 02 | KerasSequential | 레이어·활성화 함수·손실 함수·옵티마이저·콜백 |
| 03 | CNNImageClassification | Conv2D·MaxPooling·BatchNorm·CIFAR-10 |
| 04 | RNNTextSequence | Embedding·SimpleRNN·양방향·텍스트 분류 |
| 05 | LSTMTimeSeries | LSTM·GRU·시계열 예측·슬라이딩 윈도우 |

### Phase 4 — AdvancedDL (고급 딥러닝)

| # | 모듈 | 핵심 개념 |
| --- | --- | --- |
| 01 | TransferLearning | VGG16·ResNet50·Fine-tuning·Feature Extraction |
| 02 | TransformersAttention | Self-Attention·Multi-Head·Positional Encoding·BERT |
| 03 | GANBasics | Generator·Discriminator·DCGAN·Mode Collapse |
| 04 | AutoEncoder | 인코더-디코더·잠재 공간·VAE·이상 탐지 |
| 05 | ReinforcementLearningBasics | Q-Learning·Epsilon-Greedy·DQN·Experience Replay |

### Phase 5 — Projects (실전 프로젝트)

| # | 모듈 | 설명 |
| --- | --- | --- |
| 01 | ImageClassifier | 꽃·동물 이미지 분류 엔드투엔드 파이프라인 |
| 02 | TextSentimentAnalysis | 영화 리뷰 감성 분석 (IMDB 데이터셋) |
| 03 | TimeSeriesPrediction | 시계열 데이터 예측 (주가 또는 기온) |

## 권장 학습 방식

1. `LEARNING_GUIDE.md`에서 모듈의 목표와 핵심 수식을 읽습니다.
2. `main.py`의 함수 호출 순서를 따라가며 출력 결과를 분석합니다.
3. 하이퍼파라미터를 바꿔 보고 결과가 어떻게 달라지는지 관찰합니다.
4. 확장 과제를 직접 구현하고 다른 데이터셋에 적용합니다.
