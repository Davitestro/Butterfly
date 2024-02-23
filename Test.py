from tkinter import *
from tkinter import messagebox
from PIL import Image, ImageTk
import json

def login():
    email = email_entry.get().strip()
    password = password_entry.get().strip()
    if email == "":
        messagebox.showerror("Incorect username", "You can't authorize account without username")
    else:
        data = {
            "email": email,
            "password": password
        }

        with open("Datas.json", "w") as file:
            json.dump(data, file)
        
        print("Email:", email)
        print("Password:", password)
        check(0)


def toggle_frame():
    if frame.winfo_ismapped():
        frame.place_forget()
    else:
        frame.place(in_=root, x=0, y=45, relwidth=width, relheight=height)

def set_frame_size(width, height):
    frame.config(width=width, height=height)
    root.update_idletasks()

def check(event):
    with open("Datas.json", "r") as file:
        file = json.load(file)
        if file["email"] == email_entry.get().strip() and file["password"] == password_entry.get().strip():
            login_button.configure(state="disabled")
        else:
            login_button.configure(state='normal')


root = Tk()
root['bg'] = "#FFFFFF"
root.title("Minecraft Bot")
root.wm_attributes('-alpha', 0.95)
root.geometry('700x350')
root.resizable(width=False, height=False)

top_line_frame = Frame(root, bg="#8B4513", height=3)
top_line_frame.pack(fill=X)

original_image = Image.open("Minecraft_Bot_img/Account.png")
resized_image = original_image.resize((40, 40))
account_png = ImageTk.PhotoImage(resized_image)

toggle_button = Button(top_line_frame, image=account_png, command=toggle_frame, bg="#8B4513", fg="#FFFFFF")
toggle_button.pack(side=LEFT, padx=0)

frame_width = 200
frame_height = 150
frame = Frame(root,width=frame_width, height=frame_height, bg='#8B4513', bd=1, relief=SOLID)

email_label = Label(frame, text="Username:", bg='#8B4513', fg='#FFFFFF')
email_label.grid(row=0, column=0, padx=5, pady=5)
email_entry = Entry(frame)
email_entry.grid(row=0, column=1, padx=5, pady=5)

password_label = Label(frame, text="Password:", bg='#8B4513', fg='#FFFFFF')
password_label.grid(row=1, column=0, padx=5, pady=5)
password_entry = Entry(frame, show="*")
password_entry.grid(row=1, column=1, padx=5, pady=5)

email_entry.bind("<KeyRelease>", check)
password_entry.bind("<KeyRelease>", check)

with open("Datas.json", 'r') as file:
    file = json.load(file)
    email_entry.insert(0, file["email"])
    password_entry.insert(0, file["password"])

login_button = Button(frame, text="Authorize", command=login, bg="#8B4513", fg="#FFFFFF")
login_button.grid(row=2, columnspan=2, pady=10)

frame.place_forget()

check(0)

width = 0.3
height = 0.3

root.mainloop()