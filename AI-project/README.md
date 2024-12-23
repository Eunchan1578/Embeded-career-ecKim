# This_AI_project is about tracking person for smart cart & item carrier
## 프로젝트 관련 자세한 설명은 Color-Checker에서~

# 
# 
# 
# challenges :
# 1. Intel RealSense와 Raspberry Pi 호환성 문제
Problem : Intel RealSense 카메라의 프레임을 가져오지 못함<br>
Solution : Raspberry pi에 호환될 수 있도록 직접 build하여 사용<br> 
# 2. Pyrealsense 와 ROS2의 python 버전 호환성 문제
Problem : Jetson Nano의 Ubuntu 18.04 환경에서는 ROS2 Dashing이 Python 3.6까지만 지원. 하지만 Intel RealSense의 pyrealsense2 라이브러리는 Python 3.7 이상에서만 동작<br>
Solution : Intel RealSense의 구버전 소스 코드를 사용하여 pyrealsense2 모듈을 Python 3.6 환경에서 빌드
Python 3.6을 지원하는 버전의 pyrealsense2를 빌드해 ROS2 Dashing과의 호환성 문제 해결
docker container를 사용해보기로 함<br>
# 3. Docker 환경에서 ROS2 모터 제어 문제
Problem : Ubuntu 20.04 기반 Docker 컨테이너에서 ROS2 명령을 사용해 OpenCR에 모터 제어 명령을 보내도 모터가 동작하지 않음<br>
Cause : 명확하진 않으나 docker 자체는 Host OS가 아니기에 발생하는 문제라고 판단<br>
Solution : 해결하려면 시간이 상당히 필요하다고 판단하고, pyrealsense를 python 3.6에서 동작시키기로 함<br>
# 4. 시나리오에 대한 알고리즘 디버깅
Problem : 여러 예상치 못한 동작을 수행함<br>
Solution : 동작을 분석하여 디버깅하여 해결<br>
# 5. 코드를 병합하는 과정에서의 에러
Problem : library 충돌이나 상대 코드를 이해하는 데에 있어서 어려움을 겪음<br>
Solution : 대화와 refactoring을 통하여 해결<br>


#
#
# 프로젝트에 대한 자세한 정보는 본 디렉토리의 ppt에서 확인 가능



