from tkinter import *
from tkinter import messagebox
from tkinter import ttk
from PIL import Image, ImageTk
from javascript import On, require
import json
import math
import socket
import subprocess
import threading


def toggle_frame():
    if account_frame.winfo_ismapped():
        account_frame.place_forget()
    else:
        account_frame.place(in_=root, x=0, y=42, relwidth=width, relheight=height)
        account_frame.focus_set()
        ip_focus_out(0)
        port_focus_out(0)

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

previous_selection = "Choose Target"
is_dropdown_open = False
dropdown_window = None
selected_item = previous_selection
options = [["Hunt", "Deafend", "Folow"]]


if __name__ == '__main__':
    root = Tk()

    root['bg'] = "#FFFFFF"
    root.title("Minecraft Bot")
    root.wm_attributes('-alpha', 0.95)
    root.geometry('700x350')
    root.resizable(width=False, height=False)

    frame = Frame(root, bg="green")
    frame.place(relheight=1,relwidth=1)
    frame.bind("<Button-1>", on_click_frame)

    ip_text = "Server IP"
    port_text = "Server Port"

    ip = Entry(root, fg="gray")
    ip.place(x=300, y=125)
    ip.insert(0, ip_text)

    bot = None
    buttons_list = []
    choices = {}

    port = Entry(root, fg="gray")
    port.place(x=300, y=175)
    port.insert(0, port_text)

    ip.bind("<FocusIn>", ip_entry_click)
    ip.bind("<FocusOut>", ip_focus_out)
    port.bind("<FocusIn>", port_entry_click)
    port.bind("<FocusOut>", port_focus_out)

    original_image = Image.open("Minecraft_Bot_img/PVP.jpg")
    resized_image = original_image.resize((100, 100))
    pvp_photo = ImageTk.PhotoImage(resized_image)


    original_image = Image.open("Minecraft_Bot_img/Mining.jpg")
    resized_image = original_image.resize((100, 100))
    mining_photo = ImageTk.PhotoImage(resized_image)

    pvp_button = Button(root, image=pvp_photo, command=pvp_menu)
    pvp_button.image = pvp_photo

    mining_button = Button(root, image=mining_photo, command=mining_menu)
    mining_button.image = mining_photo  

    label_pvp = Label(root, text="Figthing")
    label_mining = Label(root, text="Mining")

    create_button = Button(root, text="Create Bot", command=create_bot)
    create_button.place(x=329, y=225)

    original_image = Image.open("Minecraft_Bot_img/Play.jpg")
    resized_image = original_image.resize((50, 50))


    bg_image = Image.new('RGBA', (50, 50), color='green')
    bg_photo = ImageTk.PhotoImage(bg_image)

    combined_image = Image.alpha_composite(bg_image, resized_image)
    combined_photo = ImageTk.PhotoImage(combined_image)

    start_photo = ImageTk.PhotoImage(combined_image)

    start_pvp = Button(root, image = start_photo, bd=0, command=Start_pvp, bg='green', fg='green', activebackground='green')
    start_pvp.image = start_photo

    original_image = Image.open("Minecraft_Bot_img/Stop.jpg")
    resized_image = original_image.resize((50, 50))

    bg_image = Image.new('RGBA', (50, 50), color='green')
    bg_photo = ImageTk.PhotoImage(bg_image)

    combined_image = Image.alpha_composite(bg_image, resized_image)
    combined_photo = ImageTk.PhotoImage(combined_image)

    stop_photo = ImageTk.PhotoImage(combined_image)

    stop_pvp = Button(root, image=stop_photo, bd=0,command=Stop_pvp, bg='green', fg='green', activebackground='green')
    stop_pvp.image = stop_photo

    original_image = Image.open("Minecraft_Bot_img/Back.jpg")
    resized_image = original_image.resize((50, 50))

    bg_image = Image.new('RGBA', (50, 50), color='green')
    bg_photo = ImageTk.PhotoImage(bg_image)

    combined_image = Image.alpha_composite(bg_image, resized_image)
    combined_photo = ImageTk.PhotoImage(combined_image)

    back_photo = ImageTk.PhotoImage(combined_image)
    
    back_button = Button(root, image = back_photo, bd=0, command=pvp_menu_forget, bg='green', fg='green', activebackground='green')

    back_button.image = back_photo

    mod_scrol = CustomMenuButton(root, has_search_box=False, items=[], key='Mod', text='Choose mod')

    players_scrol = CustomMenuButton(root, has_search_box=True, items=[], key='Player', text="Choose player")

    progressbar = ttk.Progressbar(root, orient="horizontal", length=200, mode="determinate")

    top_line_frame = Frame(root, bg="#8B4513", height=3)
    top_line_frame.pack(fill=X)

    original_image = Image.open("Minecraft_Bot_img/Account.png")
    resized_image = original_image.resize((40, 40))
    account_png = ImageTk.PhotoImage(resized_image)

    toggle_button = Button(top_line_frame, image=account_png, command=toggle_frame, bg="#8B4513", fg="#FFFFFF", bd=0)
    toggle_button.pack(side=LEFT, padx=0)

    frame_width = 200
    frame_height = 150
    account_frame = Frame(root,width=frame_width, height=frame_height, bg='#8B4513', bd=0, relief=SOLID)

    email_label = Label(account_frame, text="Username:", bg='#8B4513', fg='#FFFFFF', font=('Helvetica', 8))
    email_label.grid(row=0, column=0, padx=5, pady=5)
    email_entry = Entry(account_frame)
    email_entry.grid(row=0, column=1, padx=5, pady=5)

    password_label = Label(account_frame, text="Password:", bg='#8B4513', fg='#FFFFFF', font=('Helvetica', 8))
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

    exit_button = Button(account_frame, text="Exit", command=ex, bg="#8B4513", fg="#FFFFFF")

    inventory_button = Button(account_frame, text="Inventory", command=call_inv, bg="#8B4513", fg="#FFFFFF")

    radar_button = Button(account_frame, text="Minimap", command=call_radar, bg="#8B4513", fg="#FFFFFF")


    account_frame.place_forget()

    check(0)

    width = 0.3
    height = 0.3

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