from tkinter import *
from tkinter import messagebox
from tkinter import ttk
from PIL import Image, ImageTk
from javascript import On, require
import json
import math
import socket
import threading
mineflayer = require('mineflayer')
pathfinder = require('mineflayer-pathfinder')
goalfollow = pathfinder.goals.GoalFollow



def Hunting():
    bot = bot_data[0]
    mcData = require('minecraft-data')(bot.version)
    movements = pathfinder.Movements(bot, mcData)
    bot.pathfinder.setMovements(movements)
    current_target = None
    print("Start Hunting")
    current_target = bot.players[choices['Player']].entity
    goal = goalfollow(current_target, 1)
    bot.pathfinder.setGoal(goal, True)
    while True:
        playerPos = current_target.position
        botPos = bot.entity.position
        dist = math.sqrt((playerPos.x - botPos.x) ** 2 + (playerPos.y - botPos.y) ** 2 + (playerPos.z - botPos.z) ** 2)
        if dist < 3:
            bot.attack(current_target)

        if stop_duel[0]:
            stop_pvp.place_forget()
            start_pvp.place(x=325,y=148)
            stop_duel[0] = False
            current_target = bot.players[choices['Player']].entity
            goal = goalfollow(current_target, 0)
            bot.pathfinder.setGoal(goal, False)
            break


def Folow():
    bot = bot_data[0]
    mcData = require('minecraft-data')(bot.version)
    movements = pathfinder.Movements(bot, mcData)
    bot.pathfinder.setMovements(movements)
    current_target = None
    print("Start Hunting")
    current_target = bot.players[choices['Player']].entity
    goal = goalfollow(current_target, 1)
    bot.pathfinder.setGoal(goal, True)
    while True:
        if stop_duel[0]:
            stop_pvp.place_forget()
            start_pvp.place(x=325,y=148)
            stop_duel[0] = False
            current_target = bot.players[choices['Player']].entity
            goal = goalfollow(current_target, 0)
            bot.pathfinder.setGoal(goal, False)
            break


def Start_pvp():
    if choices.get('Player') != None and choices.get('Mod') != None:
        if choices["Mod"] == "Hunt":
            start_pvp.place_forget()
            stop_pvp.place(x=325,y=148)
            hunting_proces = threading.Thread(target=Hunting)
            hunting_proces.start()
        elif choices["Mod"] == "Folow":
            start_pvp.place_forget()
            stop_pvp.place(x=325,y=148)
            folow_proces = threading.Thread(target=Folow)
            folow_proces.start()


    else:
        messagebox.showerror("Please choose", "Please choose target player and mod")
        pvp_menu()


stop_duel = [False]

def Stop_pvp():
    stop_duel[0] = True


def data_cleaning(data):
    cleaned_data = data.replace('\n', ' ').replace('\u001b[94m', '').replace('\u001b[39m', '').replace('\u001b[32m', '').replace('\u001b[33m', '').replace('\u001b[90m', '').replace('\u001b[95m', '').replace('\u001b[34m', '')
    bar = ''
    tega = False
    data_set = set()
    for i in cleaned_data:
        bar += i
        if i == ' ':
            bar = ''
        if bar == 'username:':
            tega = True
        if i == "'" and len(bar) > 1 and tega == True:
            data_set.add(bar.replace("'", ''))
            tega = False

    return list(data_set)


def on_click_frame(event):
    frame.focus_set()
    ip_focus_out(0)
    port_focus_out(0)
    account_frame.place_forget()


def pvp_menu():
    mod_scrol.items = options[0]
    players_scrol.items = options[2]
    mod_scrol.update_menu_items()
    players_scrol.update_menu_items()
    mod_scrol.button.place(x=288, y = 225)
    players_scrol.button.place(x=288,y=100)
    start_pvp.place(x=325, y=148)
    menu("Forgot")


def mining_menu():
    pass


def menu(command):
    if command == "Place":
        mining_button.place(x=400, y=115)
        pvp_button.place(x=200, y=115)
        label_pvp.place(x=228, y=240)
        label_mining.place(x=432, y=240)

    elif command == "Forgot":
        mining_button.place_forget()
        pvp_button.place_forget()
        label_pvp.place_forget()
        label_mining.place_forget()
    else:
        raise ValueError(f"I don't know {command} command")


def check_server_status(server_address, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.settimeout(2)
        sock.connect((server_address, port))
        print(f"Сервер {server_address}:{port} доступен")
        return True
    except socket.error:
        print(f"Сервер {server_address}:{port} недоступен")
        return False

    finally:
        sock.close()


def on_login():
    print("Bot connected successfully!")


def ip_entry_click(event):
    if ip.get() == "Server IP":
        ip.delete(0, "end")
        ip.config(fg='black')


def ip_focus_out(event):
    if ip.get() == "":
        ip.insert(0, "Server IP")
        ip.config(fg='grey')


def port_entry_click(event):
    if port.get() == "Server Port":
        port.delete(0, "end")
        port.config(fg='black')


def port_focus_out(event):
    if port.get() == "":
        port.insert(0, "Server Port")
        port.config(fg='grey')


list_1 = []

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
        if len(list_1) == 0:
            bot_data[0].end()
            messagebox.showerror("Connection", "Connection error, pls try again")
            ip.place(x=300, y=125)
            port.place(x=300, y=175)
            create_button.place(x=329, y=225)
            bot_data.clear()
        else:
            players = data_cleaning(str(bot_data[0].players))
            players.remove("DAI_BOT")
            options.append(players)
            menu("Place")


            @On(bot_data[0], "playerJoined")
            def tab_update(first, seconde):
                players = data_cleaning(str(bot_data[0].players))
                players.remove("DAI_BOT")
                options[2] = players
                players_scrol.items = options[2]
                players_scrol.update_menu_items()


            @On(bot_data[0], "playerLeft")
            def tab_update(first, seconde):
                players = data_cleaning(str(bot_data[0].players))
                players.remove("DAI_BOT")
                options[2] = players
                players_scrol.items = options[2]
                players_scrol.update_menu_items()

        progressbar.place_forget()
        progressbar['value'] = 0
        list_1.clear()

def exit():
    bot_data[0].end()

def create_bot():
    try:
        server_port = int(port.get())
    except ValueError:
        messagebox.showerror("Port error", "Please enter a valid port number.")
        return None
    server_ip = ip.get().strip().lower()
    ip.place_forget()
    port.place_forget()
    create_button.place_forget()

    if server_ip != "Server IP" and server_port != 0:
        with open("Datas.json", 'r') as file:
            file = json.load(file)
            if check_server_status(server_ip, server_port):
                bot = mineflayer.createBot({
                    'host': server_ip,
                    'port': server_port,
                    'username': file["email"],
                    'password': file["password"]
                })

                progressbar.place(x=265,y=175)
                bot.loadPlugin(pathfinder.pathfinder)
                
                @On(bot, 'spawn')
                def spawned(*args):
                    list_1.insert(0,True)
                    mcData = require('minecraft-data')(bot.version)
                    options.append(bot.players.username)
                
                
                bot_data.append(bot)
                simulate_loading(0)
            else:
                messagebox.showerror("Error", f"Server {server_ip}:{server_port} isn't acsess")
                progressbar.place_forget()
                ip.place(x=300, y=125)
                port.place(x=300, y=175)
                create_button.place(x=329, y=225)
    else:
        messagebox.showerror("Invalid input", "Please provide valid server IP and port.")



class CustomMenuButton:
    def __init__(self, root, has_search_box, items, key, text):
        self.root = root
        self.has_search_box = has_search_box
        self.items = items
        self.key = key
        self.text = text

        self.button = Button(root, text=text, width=17)
        self.button.bind('<Button-1>', self.toggle_menu)

        self.menu = Frame(root, width=130, height=200, bg='white', bd=1, relief=SOLID)

        self.array_list = Listbox(self.menu, bg='white', selectbackground='lightgray')
        self.array_list.pack(fill='both', expand=True)


        if self.has_search_box:
            self.search_entry = Entry(self.menu)
            self.search_entry.pack(fill='x')
            self.search_entry.bind('<KeyRelease>', lambda event: self.update_menu_items())
        self.update_menu_items()

        self.array_list.bind('<<ListboxSelect>>', self.on_select)

        self.menu.place_forget()

    def toggle_menu(self, event=None):
        if self.menu.winfo_ismapped():
            self.menu.place_forget()
        else:
            button_pos_x, button_pos_y = self.button.winfo_rootx(), self.button.winfo_rooty() + self.button.winfo_height()
            menu_width = 130
            distance_from_button = 0

            root_x, root_y = self.root.winfo_rootx(), self.root.winfo_rooty()
            self.menu.place(x=(button_pos_x - root_x), y=(button_pos_y - root_y + distance_from_button), width=menu_width)

    def update_menu_items(self):
        if self.has_search_box:
            search_text = self.search_entry.get().lower()
            filtered_items = [item for item in self.items if search_text in item.lower()]
            self.array_list.delete(0, END)
            for item in filtered_items:
                self.array_list.insert(END, item)
        else:
            self.array_list.delete(0, END)
            for item in self.items:
                self.array_list.insert(END, item)

    def on_select(self, event):
        selected_index = self.array_list.curselection()
        if selected_index:
            selected_item = self.array_list.get(selected_index)
            self.button.config(text=selected_item)
            print("Выбрано:", selected_item)
            choices[self.key] = selected_item
            self.toggle_menu()

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

previous_selection = "Choose Target"
is_dropdown_open = False
dropdown_window = None
selected_item = previous_selection
options = [["Hunt", "Deafend", "Folow"]]



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

bot_data = []
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

start_pvp = Button(root, image = start_photo, command=Start_pvp, bg='green', fg='green')
start_pvp.image = start_photo

original_image = Image.open("Minecraft_Bot_img/Stop.jpg")
resized_image = original_image.resize((50, 50))

bg_image = Image.new('RGBA', (50, 50), color='green')
bg_photo = ImageTk.PhotoImage(bg_image)

combined_image = Image.alpha_composite(bg_image, resized_image)
combined_photo = ImageTk.PhotoImage(combined_image)

stop_photo = ImageTk.PhotoImage(combined_image)

stop_pvp = Button(root, image=stop_photo, command=Stop_pvp, bg='green', fg='green')
stop_pvp.image = stop_photo

mod_scrol = CustomMenuButton(root, has_search_box=False, items=[], key='Mod', text='Choose mod')

players_scrol = CustomMenuButton(root, has_search_box=True, items=[], key='Player', text="Choose player")

progressbar = ttk.Progressbar(root, orient="horizontal", length=200, mode="determinate")

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

account_frame.place_forget()

check(0)

width = 0.3
height = 0.3

root.mainloop()