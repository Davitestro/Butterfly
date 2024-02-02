from tkinter import *
from tkinter import messagebox
from PIL import Image, ImageTk
from javascript import On, require
import time
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
    current_target = bot.players[coices['Player']].entity
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
            start_pvp.place(x=288,y=150)
            stop_duel[0] = False
            current_target = bot.players[coices['Player']].entity
            goal = goalfollow(current_target, 0)
            bot.pathfinder.setGoal(goal, False)
            break

def Start_pvp():
    if coices.get('Player') != None and coices.get('Mod') != None:
        if coices["Mod"] == "Hunt":
            start_pvp.place_forget()
            stop_pvp.place(x=288,y=150)
            hunting_proces = threading.Thread(target=Hunting)
            hunting_proces.start()


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

def pvp_menu():
    players_scrol = generate_custom_menu_button(root, has_search_box=True, items=options[2], key='Player', text="Choose player")
    mod_scrol = generate_custom_menu_button(root, has_search_box=False, items=options[0], key='Mod', text='Choose mod')
    players_scrol.place(x=288,y=100)
    mod_scrol.place(x=288, y = 200)
    start_pvp.place(x=288, y = 150)
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
        if check_server_status(server_ip, server_port):
            bot = mineflayer.createBot({
                'host': server_ip,
                'port': server_port,
                'username': 'DAI_BOT',
                'password': 'DAI1234',
            })

            bot.loadPlugin(pathfinder.pathfinder)
            list_1 = []
            
            @On(bot, 'spawn')
            def spawned(*args):
                list_1.append(True)
                mcData = require('minecraft-data')(bot.version)
                options.append(bot.players.username)            
            
            for i in range(9):
                time.sleep(1)
                if len(list_1) != 0:
                    break

            if len(list_1) == 0:
                bot.end()
                messagebox.showerror("Connection", "Connection error, pls try again")
                ip.place(x=300, y=125)
                port.place(x=300, y=175)
                create_button.place(x=329, y=225)
            else:
                players = data_cleaning(str(bot.players))
                players.remove("DAI_BOT")
                options.append(players)
                bot_data.append(bot)
                menu("Place")
            list_1.clear()

        else:
            messagebox.showerror("Error", "Socket error")
            ip.place(x=300, y=125)
            port.place(x=300, y=175)
            create_button.place(x=329, y=225)
    else:
        messagebox.showerror("Invalid input", "Please provide valid server IP and port.")


def generate_custom_menu_button(root, has_search_box, items, key, text):
    def toggle_menu(event=None):
        if menu.winfo_ismapped():
            menu.place_forget()
        else:
            button_pos_x, button_pos_y = button.winfo_rootx(), button.winfo_rooty() + button.winfo_height()
            menu_width = 130
            distance_from_button = 0

            root_x, root_y = root.winfo_rootx(), root.winfo_rooty()
            menu.place(x=(button_pos_x - root_x), y=(button_pos_y - root_y + distance_from_button), width=menu_width)

    def update_menu_items():
        if has_search_box:
            search_text = search_entry.get().lower()
            filtered_items = [item for item in items if search_text in item.lower()]
            array_list.delete(0, END)
            for item in filtered_items:
                array_list.insert(END, item)
        else:
            for item in items:
                array_list.insert(END, item)

    def on_select(event):
        selected_index = array_list.curselection()
        if selected_index:
            selected_item = array_list.get(selected_index)
            button.config(text=selected_item)
            print("Выбрано:", selected_item)
            coices[key] = selected_item
            toggle_menu()

    button = Button(root, text=text, width=17)
    button.bind('<Button-1>', toggle_menu)

    menu = Frame(root, width=130, height=200, bg='white', bd=1, relief=SOLID)

    array_list = Listbox(menu, bg='white', selectbackground='lightgray')
    array_list.pack(fill='both', expand=True)

    if has_search_box:
        search_entry = Entry(menu)
        search_entry.pack(fill='x') 
        search_entry.bind('<KeyRelease>', lambda event: update_menu_items())
    update_menu_items()

    array_list.bind('<<ListboxSelect>>', on_select)

    menu.place_forget()

    return button

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

ip_text = "Server IP"
port_text = "Server Port"

ip = Entry(root, fg="gray")
ip.place(x=300, y=125)
ip.insert(0, ip_text)

bot_data = []

coices = {}

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


start_pvp = Button(root, text='Start', command=Start_pvp)
stop_pvp = Button(root, text='Stop', command=Stop_pvp)



root.mainloop()