# Python 3.10 / Author: SSoGari Studio
# Project: 숫자야구

import tkinter as tk
import random

# 게임 관련 변수 초기화
secret_number = ''.join(random.sample('0123456789', 4))
attempts = []
results = []
remaining_attempts = 10

# 게임 결과 계산 함수
def calculate_result(guess):
    if len(guess) != 4 or not guess.isdigit():
        return "올바른 4자리 숫자를 입력하세요."

    b_count = 0
    s_count = 0

    for i in range(4):
        if guess[i] == secret_number[i]:
            b_count += 1
        elif guess[i] in secret_number:
            s_count += 1

    results.append(f"{guess}: {b_count}B{s_count}S")
    return f"{b_count}B{s_count}S"

# 사용자 입력 처리 함수
def check_guess():
    global remaining_attempts
    guess = entry.get()
    result = calculate_result(guess)
    attempts.append(guess)
    listbox.insert(tk.END, f"{guess}: {result}")
    entry.delete(0, tk.END)
    remaining_attempts -= 1
    attempts_label.config(text=f"남은 기회: {remaining_attempts}")

    if result == "0B4S" or remaining_attempts == 0:
        end_game()

# 게임 종료 처리 함수
def end_game():
    listbox.insert(tk.END, f"정답: {secret_number}")
    if remaining_attempts == 0:
        listbox.insert(tk.END, "게임 종료. 기회가 모두 소진되었습니다.")
    else:
        listbox.insert(tk.END, "게임 종료. 정답을 맞췄습니다!")
    entry.config(state=tk.DISABLED)
    submit_button.config(state=tk.DISABLED)

# tkinter 창 생성
root = tk.Tk()
root.title("숫자야구 게임")

# 입력 텍스트 박스
entry_label = tk.Label(root, text="4자리 숫자를 입력하세요:")
entry_label.pack()
entry = tk.Entry(root)
entry.pack()

# 결과 출력을 위한 리스트 박스
listbox = tk.Listbox(root, height=10, width=40)
listbox.pack()

# 남은 기회 표시
attempts_label = tk.Label(root, text=f"남은 기회: {remaining_attempts}")
attempts_label.pack()

# 확인 버튼
submit_button = tk.Button(root, text="확인", command=check_guess)
submit_button.pack()

root.mainloop()
