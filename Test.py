from tkinter import *
from tkinter import messagebox
from PIL import Image, ImageTk
import json

root = Tk()

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
    if account_frame.winfo_ismapped():
        account_frame.place_forget()
    else:
        account_frame.place(in_=root, x=0, y=45, relwidth=width, relheight=height)

def set_frame_size(width, height):
    account_frame.config(width=width, height=height)
    root.update_idletasks()

def check(event):
    with open("Datas.json", "r") as file:
        file = json.load(file)
        if file["email"] == email_entry.get().strip() and file["password"] == password_entry.get().strip():
            login_button.configure(state="disabled")
        else:
            login_button.configure(state='normal')


def On_Logged():
    global height
    email_entry.grid_forget()
    password_entry.grid_forget()
    password_label.grid_forget()
    login_button.grid_forget()
    Head_Button.place(relx=0.5,rely=0.302, anchor='center')
    Body_Button.place(relx=0.502,rely=0.49, anchor='center')
    LeftArm_Button.place(relx=0.37,rely=0.49, anchor='center')
    RightArm_Button.place(relx=0.64,rely=0.49, anchor='center')
    LeftLeg_Button.place(relx=0.411,rely=0.593)
    RightLeg_Button.place(relx=0.5,rely=0.593)
    
    height = 0.7

    with open("Datas.json", 'r') as file:
        file = json.load(file)
        email_label.config(text=file['email'], font=("Helvetica", 12), anchor="center", justify="center")
        email_label.place(relx=0.5, rely=0.1, anchor="center")

        email_label.update_idletasks()


h_clicked = False
def head():
    global h_clicked
    h_clicked = not h_clicked
    if h_clicked:
        Head_Button.config(image=head_photo)
    else:
        Head_Button.config(image=Head_photo)

b_clicked = False
def body():
    global b_clicked
    b_clicked = not b_clicked
    if b_clicked:
        Body_Button.config(image=body_photo)
    else:
        Body_Button.config(image=Body_photo)


ra_clicked = False
def rightArm():
    global ra_clicked
    ra_clicked = not ra_clicked
    if ra_clicked:
        RightArm_Button.config(image=right_arm_photo)
    else:
        RightArm_Button.config(image=Right_Arm_photo)


la_clicked = False
def leftArm():
    global la_clicked
    la_clicked = not la_clicked
    if la_clicked:
        LeftArm_Button.config(image=left_arm_photo)
    else:
        LeftArm_Button.config(image=LeftArm_photo)

rl_clicked = False
def rightLeg():
    global rl_clicked
    rl_clicked = not rl_clicked
    if rl_clicked:
        RightLeg_Button.config(image=right_leg_photo)
    else:
        RightLeg_Button.config(image=Right_Leg_photo)


ll_clicked = False
def leftLeg():
    global ll_clicked
    ll_clicked = not ll_clicked
    if ll_clicked:
        LeftLeg_Button.config(image=left_leg_photo)
    else:
        LeftLeg_Button.config(image=Left_Leg_photo)

root['bg'] = "#FFFFFF"
root.title("Minecraft Bot")
root.wm_attributes('-alpha', 0.95)
root.geometry('700x350')
root.resizable(width=False, height=False)


frame = Frame(root, bg="green")
frame.place(relheight=1,relwidth=1)

top_line_frame = Frame(root, bg="#8B4513", height=3)
top_line_frame.pack(fill=X)

original_image = Image.open("Minecraft_Bot_img/Account.png")
resized_image = original_image.resize((40, 40))
account_png = ImageTk.PhotoImage(resized_image)

toggle_button = Button(top_line_frame, image=account_png, command=toggle_frame, bg="#8B4513", fg="#FFFFFF")
toggle_button.pack(side=LEFT, padx=0)

frame_width = 200
frame_height = 150
account_frame = Frame(root,width=frame_width, height=frame_height, bg='#8B4513', bd=1, relief=SOLID)

email_label = Label(account_frame, text="Username:", bg='#8B4513', fg='#FFFFFF')
email_label.grid(row=0, column=0, padx=5, pady=5)
email_entry = Entry(account_frame)
email_entry.grid(row=0, column=1, padx=5, pady=5)

password_label = Label(account_frame, text="Password:", bg='#8B4513', fg='#FFFFFF')
password_label.grid(row=1, column=0, padx=5, pady=5)
password_entry = Entry(account_frame, show="*")
password_entry.grid(row=1, column=1, padx=5, pady=5)

email_entry.bind("<KeyRelease>", check)
password_entry.bind("<KeyRelease>", check)

with open("Datas.json", 'r') as file:
    file = json.load(file)
    email_entry.insert(0, file["email"])
    password_entry.insert(0, file["password"])

login_button = Button(account_frame, text="Authorize", command=login, bg="#8B4513", fg="#FFFFFF")
login_button.grid(row=2, columnspan=2, pady=10)

exit_button = Button(account_frame, text="Exit", command=exit, bg="#8B4513", fg="#FFFFFF")

width = 0.3
height = 0.3

account_frame.place_forget()

check(0)

button = Button(root, text="Change", command=On_Logged)
button.place(x=300,y=150)

original_image = Image.open("Minecraft_Bot_img/Layer_buttons/Steve_Head_2layer.png")
resized_image = original_image.resize((39, 39))
Head_photo = ImageTk.PhotoImage(resized_image)
Head_Button = Button(account_frame, image=Head_photo, bd=0, background="#8B4513", activebackground="#8B4513", command=head)


original_image = Image.open("Minecraft_Bot_img/Layer_buttons/Steve_Left_Arm_2layer.png")
resized_image = original_image.resize((20, 50))
LeftArm_photo = ImageTk.PhotoImage(resized_image)
LeftArm_Button = Button(account_frame, bd=0, image=LeftArm_photo, background="#8B4513", activebackground="#8B4513", command=leftArm)


original_image = Image.open("Minecraft_Bot_img/Layer_buttons/Steve_Right_Arm_2layer.png")
resized_image = original_image.resize((20, 50))
Right_Arm_photo = ImageTk.PhotoImage(resized_image)
RightArm_Button = Button(account_frame, bd=0, image=Right_Arm_photo, background="#8B4513", activebackground="#8B4513", command=rightArm)


original_image = Image.open("Minecraft_Bot_img/Layer_buttons/Steve_Body_2layer.png")
resized_image = original_image.resize((35, 50))
Body_photo = ImageTk.PhotoImage(resized_image)
Body_Button = Button(account_frame, bd=0, image=Body_photo, background="#8B4513", activebackground="#8B4513", command=body)

original_image = Image.open("Minecraft_Bot_img/Layer_buttons/Steve_Left_Leg_2layer.png")
resized_image = original_image.resize((18, 50))
Left_Leg_photo = ImageTk.PhotoImage(resized_image)
LeftLeg_Button = Button(account_frame, bd=0, image=Left_Leg_photo, background="#8B4513", activebackground="#8B4513", command=leftLeg)

original_image = Image.open("Minecraft_Bot_img/Layer_buttons/Steve_Right_Leg_2layer.png")
resized_image = original_image.resize((18, 50))
Right_Leg_photo = ImageTk.PhotoImage(resized_image)
RightLeg_Button = Button(account_frame, bd=0, image=Right_Leg_photo, background="#8B4513", activebackground="#8B4513", command=rightLeg)

original_image = Image.open("Minecraft_Bot_img/Layer_buttons/Steve_Head.png")
resized_image = original_image.resize((39, 39))
head_photo = ImageTk.PhotoImage(resized_image)

original_image = Image.open("Minecraft_Bot_img/Layer_buttons/Steve_Body.png")
resized_image = original_image.resize((35, 50))
body_photo = ImageTk.PhotoImage(resized_image)

original_image = Image.open("Minecraft_Bot_img/Layer_buttons/Steve_Right_Arm.png")
resized_image = original_image.resize((20, 50))
right_arm_photo = ImageTk.PhotoImage(resized_image)

original_image = Image.open("Minecraft_Bot_img/Layer_buttons/Steve_Left_Arm.png")
resized_image = original_image.resize((20, 50))
left_arm_photo = ImageTk.PhotoImage(resized_image)

original_image = Image.open("Minecraft_Bot_img/Layer_buttons/Steve_Right_Leg.png")
resized_image = original_image.resize((18, 50))
right_leg_photo = ImageTk.PhotoImage(resized_image)

original_image = Image.open("Minecraft_Bot_img/Layer_buttons/Steve_Left_Leg.png")
resized_image = original_image.resize((18, 50))
left_leg_photo = ImageTk.PhotoImage(resized_image)

root.mainloop()