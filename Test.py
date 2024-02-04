import tkinter as tk
from tkinter import ttk

def on_loading_button_click():
    loading_button.pack_forget()  # Скрыть кнопку
    progressbar.pack(pady=50)  # Показать полосу загрузки
    simulate_loading(0)

def simulate_loading(i):
    if i <= 30:
        root.after(150, lambda: update_progress(i + 1))
    elif 30 < i <= 70:
        root.after(130, lambda: update_progress(i + 1))
    elif 70 < i <= 87:
        root.after(240, lambda: update_progress(i + 1))
    elif 87 < i <= 95:
        root.after(25, lambda: update_progress(i + 1))
    elif 95 < i <= 99:
        root.after(575, lambda: update_progress(i + 1))
    else:
        root.after(2250, lambda: update_progress(i + 1))

def update_progress(i):
    progressbar["value"] = i
    root.update_idletasks()
    
    if i < 100:
        simulate_loading(i)
    else:
        print("Loading completed!")
        show_loading_button()

def show_loading_button():
    progressbar.pack_forget()
    loading_button.pack(pady=50)

root = tk.Tk()
root.title("Главное окно")

# Создаем кнопку
loading_button = tk.Button(root, text="Начать загрузку", command=on_loading_button_click)
loading_button.pack(pady=50)

# Создаем полосу загрузки
progressbar = ttk.Progressbar(root, orient="horizontal", length=200, mode="determinate")

root.mainloop()
