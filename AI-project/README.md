# This_AI_project is about tracking person for smart cart & item carrier




# challenges
# 1. Intel RealSense와 Raspberry Pi 호환성 문제
Problem : Intel RealSense 카메라의 프레임을 가져오지 못함<br>
Solution : Raspberry pi에 호환될 수 있도록 직접 build하여 사용<br> 
# 2. Pyrealsense 와 ROS2의 python 버전 호환성 문제
Problem : Jetson Nano의 Ubuntu 18.04 환경에서는 ROS2 Dashing이 Python 3.6까지만 지원. 하지만 Intel RealSense의 pyrealsense2 라이브러리는 Python 3.7 이상에서만 동작<br>
Solution : Intel RealSense의 구버전 소스 코드를 사용하여 pyrealsense2 모듈을 Python 3.6 환경에서 빌드
Python 3.6을 지원하는 버전의 pyrealsense2를 빌드해 ROS2 Dashing과의 호환성 문제 해결
docker container를 사용해보기로 함
