# 현황
### 확인된 기능
1. 아두이노 2개 동시 연결
2. 마우스 움직임, 클릭, 스크롤 딜레이 문제 없음

### 확인된 문제
1. (main.cpp) 마우스 움직임 쪽 노이즈 문제 -> 필터 씌어서 해결 예정.
2. (main_body.ino) 스크롤 장치 반영 안됨. 임시로 입력 포트 지정해서 작동중. -> 반영 에정.

# [Arduino nano sense] mouse with euler angle
![KakaoTalk_20240630_184556170](https://github.com/serafi1204/-Arduino-mouse-with-euler-angle/assets/122208990/f8af0d6c-b58a-474a-b001-4787ef760f81)

## Used python libraries
    pip install pyserial #Serial communication
    pip install pyautogui # Control mouse
    pip install pyautogui # math

## Referance
[Naver blog](https://m.blog.naver.com/PostView.naver?blogId=seongilseo77&logNo=222225637028&navType=by)
