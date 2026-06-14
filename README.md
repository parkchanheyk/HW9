Markdown
# HW9: Unreal C++ Multiplayer Game

본 프로젝트는 언리얼 엔진 C++을 기반으로 구현된 간단한 멀티플레이어 게임 과제(HW9)입니다. 
네트워크 동기화(Replication), RPC(Remote Procedure Call), 그리고 기본적인 멀티플레이어 게임플레이 루프를 포함하고 있습니다.

## 📂 파일 구조 (Project Structure)

```text
HW9/
├── Config/                  # 프로젝트 설정 파일 (.ini)
├── Content/                 # 에셋 (블루프린트, 애니메이션, 맵 등)
├── Source/                  # C++ 소스 코드 디렉토리
│   ├── HW9/
│   │   ├── HW9.Build.cs     # 모듈 빌드 설정
│   │   ├── GameMode/        # 게임모드 및 상태 관리 (GameMode, GameState)
│   │   ├── Character/       # 플레이어 캐릭터 및 복제(Replication) 로직
│   │   ├── Controller/      # 플레이어 컨트롤러 및 UI 연동        
├── HW9.uproject             # 언리얼 프로젝트 파일
└── README.md                # 프로젝트 설명 문서
```

🛠️ 개발 환경 및 요구 사항 (Prerequisites)
Engine Version: Unreal Engine [예: 5.3 / 5.4]

IDE: [예: Visual Studio 2022 / Rider]

Language: C++17 / C++20

Platform: Windows 10 / 11 (PC Multiplayer)

🚀 실행 및 테스트 방법 (Setup & Run)
1. 프로젝트 빌드 및 열기
HW9.uproject 파일을 우클릭한 후 "Generate Visual Studio project files"를 클릭합니다.

생성된 .sln 파일을 열고, Development Editor / Win64 설정으로 빌드(Build)를 수행합니다.

빌드가 완료되면 언리얼 에디터를 실행합니다.

2. 에디터 내 멀티플레이어 테스트 (PIE)
언리얼 에디터 상단의 Play(재생) 버튼 옆 점 3개(세부 설정)를 클릭합니다.

Net Mode를 Play As Listen Server 또는 Play As Client로 변경합니다.

Number of Players를 2 이상으로 설정한 뒤 실행하여 서버와 클라이언트 간의 동기화를 확인합니다.

💡 주요 구현 사항 (Key Features)
Network Replication (속성 복제):

캐릭터의 [예: 시도 횟수 , 점수(Score)] 변수에 Replicated 조건을 적용하여 서버-클라이언트 간 상태를 동기화했습니다.

OnRep_ 함수를 통해 상태 변경 시 클라이언트 측 UI나 이펙트가 즉각 반응하도록 구현했습니다.

RPC (Remote Procedure Calls):

Server RPC: 클라이언트가 공격하거나 상호작용할 때, 서버에 검증 및 로직 실행을 요청합니다 (WithValidation).

Multicast RPC: 서버에서 발생한 UI 변경 등의 시각적 요소를 모든 클라이언트에게 재생합니다.

Multiplayer Gameplay Loop:

GameMode를 통해 신규 플레이어 진입(PostLogin) 및 리스폰 로직을 관리합니다.

서버에서만 승부 판정 및 중요 로직이 처리되도록 안전하게 설계했습니다.
