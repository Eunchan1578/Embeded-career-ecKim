# Intel-AI-project
Gihub got openvino AI project(Human chasing mobility)


# Project Gantt Chart

```mermaid
gantt
    title Project Schedule
    dateFormat  MM/DD
    section Market Research
    Pre-research        :active,   pre-research, 09/25, 1d
    section AI Model
    Human Detection     :active,   human-detection, 09/25, 3d
    Hand Detection      :          hand-detection, 09/27, 3d
    Hand Gesture Classification:   hand-gesture-classification, 09/30, 3d
    Mono-eye Distance Measurement: mono-eye-distance, 09/25, 6d
    Color Detection     :          color-detection, 09/25, 6d
    Segmentation Model  :          segmentation, 09/25, 10d
    Model Synthesis (Ensemble) :    model-synthesis, 09/30, 9d
    Model Testing       :          model-testing, 09/27, 14d
    section Hardware
    Hardware Setup      :          hardware-setup, 10/08, 3d
    Vehicle Algorithm Development:  vehicle-algorithm, 10/10, 7d
    Embedded Software Development: embedded-software, 10/10, 7d
    section Total Inspection
    Debugging           :          debugging, 10/16, 6d
    Testing             :          testing, 10/16, 7d
    Rehearsal           :          rehearsal, 10/21, 2d
```

# Software Flow Chart

```mermaid
flowchart fc
    A[Start: Chasing Car Activated] --> B[Depth Camera Input]
    B --> C[Person Detection]
    C --> D[Pose Estimation: Shoulders and Hips]
    D --> E[Create Landmark and Drawing Rectangle on Body Features]
    E --> F[Is person stand alone?]
    F -- Yes --> G[Is Person Showing Victory Hand Gesture over three second?]
    F -- No --> C
    G -- Yes --> H[Detect Specific Person and Memorize RGB & HSV mean data]
    G -- No --> C
    H --> I[Follow Detected Person]
    I --> J[Hand Gesture Recognition]
    I --> Q[Has Person Disappeared from View?]
    Q -- Yes --> M[Stop]
    Q -- No --> I
    J --> K{Gesture Command}
    K -- Open Palm --> M[Stop]
    K -- Thumbs Up--> N[Move]
    K -- I Love You --> L[Reset]
    L --> C
```



# HLD1 (젯슨나노에서 모델을 돌리기 어려운 경우)
<img src="./HLD1.png" alt="이미지 설명" width="500" height="400"/>

# HLD2 (젯슨나노에서 모델을 돌릴 수 있을 경우)
<img src="./HLD2.png" alt="이미지 설명" width="300" height="300"/>





# 본 프로젝트에서는 HLD1을 진행
# 프로젝트 완성 영상 : https://www.youtube.com/watch?v=t410JO3gH-A



