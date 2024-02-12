import tkinter as tk
from PIL import Image, ImageTk

window = tk.Tk()

window.configure(bg='green')

jpg_image = Image.open("Minecraft_Bot_img\Play.jpg")
jpg_image = jpg_image.resize((50, 50))

bg_image = Image.new('RGBA', (50, 50), color='green')
bg_photo = ImageTk.PhotoImage(bg_image)

combined_image = Image.alpha_composite(bg_image, jpg_image)
combined_photo = ImageTk.PhotoImage(combined_image)

button = tk.Button(window, image=combined_photo, command=lambda: print("Button clicked"))

button.config(borderwidth=0, highlightthickness=0)

button.pack()

window.mainloop()