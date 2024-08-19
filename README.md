### ★[프로젝트, 아두이노 소스 다운로드](https://drive.google.com/file/d/1qMjp9VInAZmbb86hGWtLLq77AieaAka-/view?usp=sharing)★

## 업데이트
2024.08.19
- 마우스 움직임 노이즈 해결 (단, 아두이노 단의 카넬 필터 제거됨)
- 움직임 딜레이 양호함 확인


## 확인된 문제
1. (main_body.ino) 스크롤 장치 반영 안됨. 임시로 입력 포트 지정해서 작동중. -> 반영 에정.
2. C++에서 처음에 연결되고 일정 시간동안 딜레이 있음.
3. 디버그 출력 비활성화 시 움직임이 비정상적으로 빠름.

# [Arduino nano sense] mouse with euler angle
![KakaoTalk_20240630_184556170](https://github.com/serafi1204/-Arduino-mouse-with-euler-angle/assets/122208990/f8af0d6c-b58a-474a-b001-4787ef760f81)

## Used python libraries
    pip install pyserial #Serial communication
    pip install pyautogui # Control mouse
    pip install pyautogui # math

## Referance
[Naver blog](https://m.blog.naver.com/PostView.naver?blogId=seongilseo77&logNo=222225637028&navType=by)
