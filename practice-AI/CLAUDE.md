# CLAUDE.md — practice-AI 에이전트 지침

## 워크스페이스 개요

- **주제**: AI / 머신러닝 / 딥러닝
- **언어**: Python 3.11+
- **주요 라이브러리**: NumPy, Pandas, Matplotlib, scikit-learn, TensorFlow 2.x / Keras
- **선행 학습**: Python 기본 문법 (변수, 함수, 클래스, 리스트/딕셔너리)
- **관련 저장소**: (없음)

## 디렉토리 구조

```
practice-AI/
├── Phase1_Fundamentals/
│   ├── 01_NumpyBasics/
│   ├── 02_PandasBasics/
│   ├── 03_MatplotlibVisualization/
│   └── 04_ScipyStats/
├── Phase2_MachineLearning/
│   ├── 01_LinearRegression/
│   ├── 02_LogisticRegression/
│   ├── 03_DecisionTree/
│   ├── 04_RandomForest/
│   ├── 05_SVM/
│   ├── 06_KMeansClustering/
│   └── 07_PCA/
├── Phase3_DeepLearning/
│   ├── 01_TensorFlowBasics/
│   ├── 02_KerasSequential/
│   ├── 03_CNNImageClassification/
│   ├── 04_RNNTextSequence/
│   └── 05_LSTMTimeSeries/
├── Phase4_AdvancedDL/
│   ├── 01_TransferLearning/
│   ├── 02_TransformersAttention/
│   ├── 03_GANBasics/
│   ├── 04_AutoEncoder/
│   └── 05_ReinforcementLearningBasics/
├── Phase5_Projects/
│   ├── 01_ImageClassifier/
│   ├── 02_TextSentimentAnalysis/
│   └── 03_TimeSeriesPrediction/
├── requirements.txt
├── README.md
├── CLAUDE.md               # 이 파일
└── LEARNING_GUIDE.md
```

## 새 모듈 추가 방법

1. 해당 Phase 폴더 아래 `NN_ModuleName/` 폴더를 만듭니다.
2. `main.py` (진입점), `utils.py` (공용 헬퍼, 필요시)를 생성합니다.
3. 모듈 폴더 내에 `requirements.txt` 또는 상단에 설치 안내를 추가합니다.
4. `LEARNING_GUIDE.md`에 해당 모듈의 학습 교안을 추가합니다.

## 주요 패키지 목록

```
numpy>=1.26
pandas>=2.2
matplotlib>=3.8
seaborn>=0.13
scipy>=1.13
scikit-learn>=1.4
tensorflow>=2.16
keras>=3.0
jupyter>=1.0
```

## main.py 초기화 템플릿

```python
"""
<모듈 이름> — <한 줄 설명>
"""
import numpy as np
# 필요한 라이브러리 import


def demo_<개념1>() -> None:
    print("=== <개념1> ===")
    # 구현


def demo_<개념2>() -> None:
    print("=== <개념2> ===")
    # 구현


if __name__ == "__main__":
    demo_<개념1>()
    demo_<개념2>()
```

## 코딩 컨벤션

- 각 `main.py`는 독립 실행 가능한 단일 파일로 구성합니다.
- 개념별로 `def demo_xxx()` 함수를 분리하고, `if __name__ == "__main__"` 에서 순서대로 호출합니다.
- 출력은 `print("=== 섹션명 ===")` 헤더로 구분합니다.
- 숫자 출력 시 `np.set_printoptions(precision=4, suppress=True)` 를 모듈 상단에 설정합니다.
- 모델 학습은 재현 가능하도록 `np.random.seed(42)`, `tf.random.set_seed(42)` 를 사용합니다.
- 플롯은 `plt.show()` 대신 `plt.savefig("output.png")` 로 저장 (헤드리스 환경 대비).
- 외부 데이터셋은 `scikit-learn` 내장 데이터셋 또는 Keras 내장 데이터셋 우선 사용.

## Phase별 학습 진행 현황

| Phase | 모듈 수 | 완료 | 상태 |
| --- | --- | --- | --- |
| Phase1_Fundamentals | 4 | 0 | 미시작 |
| Phase2_MachineLearning | 7 | 0 | 미시작 |
| Phase3_DeepLearning | 5 | 0 | 미시작 |
| Phase4_AdvancedDL | 5 | 0 | 미시작 |
| Phase5_Projects | 3 | 0 | 미시작 |

## 주의 사항

- TensorFlow는 GPU 드라이버 없이 CPU 모드로도 동작합니다. GPU 사용 시 CUDA Toolkit + cuDNN 버전 호환을 확인하세요.
- Phase4 Transformer 모듈은 Hugging Face `transformers` 라이브러리를 사용할 수 있습니다 (별도 설치).
- Phase5 프로젝트는 학습 시간이 길 수 있으므로 epoch 수를 낮게 시작한 뒤 늘립니다.
- `.venv` 가상 환경을 사용하고, `.gitignore` 에 포함되어 있어 커밋되지 않습니다.
