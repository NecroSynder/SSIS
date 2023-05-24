
from kivy.clock import Clock
from kivy.lang.builder import Builder
from kivy.metrics import dp
from kivy.properties import BooleanProperty
from kivy.uix.anchorlayout import AnchorLayout
from kivy.uix.screenmanager import Screen
from kivymd.app import MDApp
from kivymd.uix.datatables import MDDataTable
from kivymd.uix.label import MDLabel
from kivymd.uix.menu import MDDropdownMenu
from kivymd.uix.snackbar import MDSnackbar, MDSnackbarCloseButton
import psycopg2
from kivy.core.window import Window


def connect_to_db():
    connection = psycopg2.connect(
        host="localhost",
        database="postgres",
        user="postgres",
        password="Necronomicon",
        port="5432",
    )
    return connection


class StartPage(Screen):
    pass


class ClientsTable(Screen):
    edit_button_disabled = BooleanProperty(True)
    delete_button_disabled = BooleanProperty(True)

    def __init__(self, **kw):
        super().__init__()
        self.data_tables = None
        self.selected_rows = set()
        connection = connect_to_db()
        cursor = connection.cursor()
        cursor.execute("""CREATE TABLE IF NOT EXISTS students (
                            id SERIAL PRIMARY KEY,
                            name VARCHAR(255),
                            year_level VARCHAR(255),
                            course VARCHAR(255),
                            gender VARCHAR(255)
                          )""")

        connection.commit()
        cursor.close()
        connection.close()
        self.load_table()

    def load_table(self):
        connection = connect_to_db()
        cursor = connection.cursor()
        cursor.execute("SELECT * FROM students")
        results = cursor.fetchall()
        cursor.close()
        connection.close()
        layout = AnchorLayout()

        # Sort the results based on the ID column
        sorted_results = sorted(results, key=lambda x: x[0])

        if self.data_tables:
            self.remove_widget(self.data_tables)

        self.data_tables = MDDataTable(
            pos_hint={'center_y': 0.5, 'center_x': 0.5},
            size_hint=(0.9, 0.7),
            use_pagination=True,
            check=True,
            rows_num=10,
            column_data=[
                ("ID.", dp(68)),
                ("Name", dp(68)),
                ("Year Level", dp(68)),
                ("Courses", dp(68)),
                ("Gender", dp(68)), ],
            # Use sorted_results instead of results
            row_data=[(str(row[0]), row[1], str(row[2]), row[3], row[4]) for row in sorted_results], )
        self.data_tables.bind(on_check_press=self.on_check_press)
        self.add_widget(self.data_tables)
        return layout

    def on_enter(self):
        self.load_table()
        self.reset_button_states()

    def on_check_press(self, instance_table, current_row):
        row_id = int(current_row[0])

        if row_id in self.selected_rows:
            self.selected_rows.remove(row_id)
        else:
            self.selected_rows.add(row_id)  # Remove the clear() line here

        self.update_button_states()

    def update_button_states(self):
        self.edit_button_disabled = len(self.selected_rows) != 1
        self.delete_button_disabled = len(self.selected_rows) == 0

    def reset_button_states(self):
        self.edit_button_disabled = True
        self.delete_button_disabled = True

    def reset_id_sequence(self):
        connection = connect_to_db()
        cursor = connection.cursor()
        cursor.execute("SELECT MAX(id) FROM students")
        max_id = cursor.fetchone()[0] or 0
        cursor.execute("ALTER SEQUENCE students_id_seq RESTART WITH %s", (max_id + 1,))
        connection.commit()
        cursor.close()
        connection.close()

    def delete_selected_rows(self):
        connection = connect_to_db()
        cursor = connection.cursor()

        selected_rows_list = list(self.selected_rows)
        cursor.execute("DELETE FROM students WHERE id IN %s", (tuple(selected_rows_list),))
        connection.commit()
        cursor.close()
        connection.close()

        self.reset_id_sequence()
        self.load_table()

        MDSnackbar(
            MDLabel(
                text="Deleted",
                theme_text_color="Custom",
                text_color="#060000",
            ),
            pos_hint={"top": .98, "right": .98},
            size_hint_x=0.18,
            md_bg_color="#E8FFE8",
        ).open()

        self.reset_button_states()  # Add this line to reset the button states after deleting rows
        self.selected_rows.clear()

        return len(selected_rows_list) > 0

    def edit_selected_rows(self):
        if len(self.selected_rows) != 1:
            # Show an error message if more than one row is selected or none is selected
            return

        row_id = list(self.selected_rows)[0]
        connection = connect_to_db()
        cursor = connection.cursor()
        cursor.execute("SELECT * FROM students WHERE id = %s", (row_id,))
        row_data = cursor.fetchone()
        cursor.close()
        connection.close()

        if row_data is None:
            # Show an error message if the row_id does not exist in the database
            return

        edit_screen = self.manager.get_screen('Edit')
        edit_screen.row_data = row_data
        self.manager.current = 'Edit'

        self.selected_rows.clear()

    def search_data(self, search_text):
        # print(f"Searching for: {search_text}")  # Add this print statement
        connection = connect_to_db()
        cursor = connection.cursor()

        if search_text:
            cursor.execute("""SELECT * FROM students WHERE
                              id::TEXT LIKE %s OR
                              name LIKE %s OR
                              year_level::TEXT LIKE %s OR
                              course LIKE %s OR
                              (gender = 'M' AND %s LIKE '%%M%%') OR
                              (gender = 'F' AND %s LIKE '%%F%%') OR
                              gender LIKE %s""",
                           (f"%{search_text}%", f"%{search_text}%", f"%{search_text}%", f"%{search_text}%",
                            search_text, search_text, f"%{search_text}%"))
        else:
            cursor.execute("SELECT * FROM students")

        results = cursor.fetchall()
        # print(f"Results: {results}")  # Add this print statement
        cursor.close()
        connection.close()

        # Sort the results based on the ID column
        sorted_results = sorted(results, key=lambda x: x[0])

        # Update the DataTable with the filtered data
        self.data_tables.row_data = [(str(row[0]), row[1], row[2], row[3], row[4]) for row in sorted_results]


class Add(Screen):
    def __init__(self, **kwargs):
        super(Add, self).__init__(**kwargs)
        self.menu = None
        Clock.schedule_once(self.create_menu)

    def create_menu(self, *args):
        menu_items = [
            {
                "text": gender,
                "on_release": lambda x=gender: self.set_item(x),
            } for gender in ["M", "F", "Other"]
        ]
        self.menu = MDDropdownMenu(
            caller=self.ids['gender'],
            items=menu_items,
            position="center",
        )

    def open_menu(self):
        self.menu.open()

    def set_item(self, text_item):
        self.ids['gender'].text = text_item
        self.menu.dismiss()

    def go_back(self):
        screen_manager = self.manager
        screen_manager.current = 'Clients-table'

    def are_all_fields_filled(self):
        text_fields = [self.ids.name, self.ids.year_level, self.ids.course, self.ids.gender]

        for text_field in text_fields:
            if not text_field.text.strip():
                return False
        return True

    def open_snackbar(self):
        if not self.are_all_fields_filled():
            self.snackbar = MDSnackbar(
                MDLabel(
                    text="No Text Filled",
                    theme_text_color="Custom",
                    text_color="#393231",
                ),
                MDSnackbarCloseButton(
                    icon="close",
                    theme_text_color="Custom",
                    text_color="#8E353C",
                    _no_ripple_effect=True,
                    on_release=self.snackbar_close,
                ),
                pos=(dp(24), dp(20)),
                size_hint_x=0.2,
                md_bg_color="#E8D8D7",
            )
            self.snackbar.open()
        else:
            connection = connect_to_db()
            cursor = connection.cursor()
            cursor.execute("INSERT INTO students (name, year_level, course, gender) VALUES (%s, %s, %s, %s)",
                           (self.ids.name.text, self.ids.year_level.text, self.ids.course.text, self.ids.gender.text))

            connection.commit()
            cursor.close()
            connection.close()

            # Update the KivyMD datatable
            clients_table = self.manager.get_screen('Clients-table')
            clients_table.load_table()

            self.snackbar = MDSnackbar(
                MDLabel(
                    text="Loaded",
                    theme_text_color="Custom",
                    text_color="#393231",
                ),
                MDSnackbarCloseButton(
                    icon="close",
                    theme_text_color="Custom",
                    text_color="#8E353C",
                    _no_ripple_effect=True,
                    on_release=self.snackbar_close,
                ),
                pos=(dp(24), dp(20)),
                size_hint_x=0.2,
                md_bg_color="#E8D8D7",
            )
            self.snackbar.open()

            # Clear text fields
            self.ids.name.text = ""
            self.ids.year_level.text = ""
            self.ids.course.text = ""
            self.ids.gender.text = ""

    def snackbar_close(self, *args):
        self.snackbar.dismiss()


class Edit(Screen):
    def __init__(self, **kwargs):
        super(Edit, self).__init__(**kwargs)
        self.row_id = None
        self.menu = None
        self.row_data = None
        Clock.schedule_once(self.create_menu)

    def create_menu(self, *args):
        menu_items = [
            {
                "text": gender,
                "on_release": lambda x=gender: self.set_item(x),
            } for gender in ["M", "F", "Other"]
        ]
        self.menu = MDDropdownMenu(
            caller=self.ids['gender_input'],
            items=menu_items,
            position="center",
        )

    def open_menu(self):
        self.menu.open()

    def set_item(self, text_item):
        self.ids['gender_input'].text = text_item  # Update this line
        self.menu.dismiss()

    def on_pre_enter(self):
        if self.row_data is not None:
            self.set_edit_data(self.row_data)
            self.row_data = None

    def set_edit_data(self, row_data):
        self.row_id = row_data[0]
        self.ids['name_input'].text = row_data[1]
        self.ids['year_level_input'].text = str(row_data[2])
        self.ids['course_input'].text = row_data[3]
        self.ids['gender_input'].text = row_data[4]

    def update_data(self):
        connection = connect_to_db()
        cursor = connection.cursor()
        cursor.execute("UPDATE students SET name = %s, year_level = %s, course = %s, gender = %s WHERE id = %s",
                       (self.ids.name_input.text, self.ids.year_level_input.text, self.ids.course_input.text,
                        self.ids.gender_input.text,
                        str(self.row_id)))

        connection.commit()
        cursor.close()
        connection.close()

        clients_table = self.manager.get_screen('Clients-table')
        clients_table.load_table()
        self.manager.current = 'Clients-table'

    def go_back(self):
        screen_manager = self.manager
        screen_manager.current = 'Clients-table'


class MainWindow(MDApp):
    def build(self):
        Window.maximize()
        self.theme_cls.material_style = "M3"
        self.title = "SSIS v2"
        self.theme_cls.primary_palette = "Blue"
        self.theme_cls.theme_style = "Light"
        return Builder.load_file('SSIS.kv')


if __name__ == "__main__":
    MainWindow().run()
