from tkinter import *

class CustomMenuButton:
    def __init__(self, root, has_search_box, items, key, text, all_buttons):
        self.root = root
        self.has_search_box = has_search_box
        self.items = items
        self.key = key
        self.text = text
        self.all_buttons = all_buttons

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
            self.enable_buttons()
        else:
            button_pos_x, button_pos_y = self.button.winfo_rootx(), self.button.winfo_rooty() + self.button.winfo_height()
            menu_width = 130
            distance_from_button = 0

            root_x, root_y = self.root.winfo_rootx(), self.root.winfo_rooty()
            self.menu.place(x=(button_pos_x - root_x), y=(button_pos_y - root_y + distance_from_button), width=menu_width)
            self.disable_other_buttons()

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
            self.toggle_menu()

    def disable_other_buttons(self):
        for other_button in self.all_buttons:
            if other_button != self.button:
                other_button.config(state='disabled')

    def enable_buttons(self):
        for other_button in self.all_buttons:
            other_button.config(state='normal')

# Пример использования
root = Tk()
root.geometry('700x350')
root.resizable(width=False, height=False)

frame = Frame(root, bg="green")
frame.place(relheight=1,relwidth=1)

options = [["Option 1", "Option 2", "Option 3"]]
buttons_list = []
mod_scrol = CustomMenuButton(root, has_search_box=False, items=options[0], key='Mod', text='Choose mod', all_buttons=buttons_list)
eli_button = CustomMenuButton(root, has_search_box=True, items=["Mek","Erku"], key='mi ban', text='Choose miaban', all_buttons=buttons_list)
mod_scrol.items = ['0','1','2']
mod_scrol.update_menu_items()
eli_button.button.place(x=350, y=100)
mod_scrol.button.place(x=350, y=200)


root.mainloop()