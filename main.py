import socket
import sys
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QPixmap
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QLabel, QLineEdit, QPushButton,
    QComboBox, QMessageBox, QSpinBox, QRadioButton, QButtonGroup, QHBoxLayout, QTableWidget, QTableWidgetItem,
    QStackedWidget, QSizePolicy, QHeaderView, QFileDialog
)


class LoginWindow(QWidget):
    def __init__(self):
        super().__init__()

        self.user_id = None
        self.setWindowTitle("Вхід до бібліотеки")
        self.setGeometry(100, 100, 400, 300)
        self.setStyleSheet("background-color: #606060;")
        self.layout = QVBoxLayout()

        self.label_username = QLabel("Ім'я користувача:")
        self.label_username.setStyleSheet("color: white; font-size: 16px;")
        self.entry_username = QLineEdit()
        self.entry_username.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.label_username)
        self.layout.addWidget(self.entry_username)

        self.label_email = QLabel("Електронна пошта:")
        self.label_email.setStyleSheet("color: white; font-size: 16px;")
        self.entry_email = QLineEdit()
        self.entry_email.setStyleSheet("background-color: white;")
        self.label_email.hide()
        self.entry_email.hide()
        self.layout.addWidget(self.label_email)
        self.layout.addWidget(self.entry_email)

        self.label_password = QLabel("Пароль:")
        self.label_password.setStyleSheet("color: white; font-size: 16px;")
        self.entry_password = QLineEdit()
        self.entry_password.setEchoMode(QLineEdit.Password)
        self.entry_password.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.label_password)
        self.layout.addWidget(self.entry_password)

        self.login_button = QPushButton("Увійти")
        self.login_button.setStyleSheet("""
            background-color: #556080;  
            color: white;  
            font-size: 14px;  
            padding: 10px;  
            border-radius: 10px;  
        """)
        self.login_button.clicked.connect(self.login)
        self.layout.addWidget(self.login_button)

        self.register_button = QPushButton("Зареєструватися")
        self.register_button.setStyleSheet("""
            background-color: #556080;  
            color: white;  
            font-size: 14px;  
            padding: 10px;  
            border-radius: 10px;
        """)
        self.register_button.clicked.connect(self.register)
        self.layout.addWidget(self.register_button)

        self.setLayout(self.layout)

        self.registration_mode = False

    def register(self):

        self.entry_username.clear()
        self.entry_password.clear()

        self.label_email.show()
        self.entry_email.show()

        self.register_button.setText("Готово")
        self.register_button.clicked.disconnect()
        self.register_button.clicked.connect(self.register_user)

    def register_user(self):

        username = self.entry_username.text().strip()
        password = self.entry_password.text().strip()
        email = self.entry_email.text().strip()

        if not username or not password or not email:
            self.show_message("Будь ласка, заповніть всі поля для реєстрації.")
            return

        request = f"REGISTER {username} {email} {password}"
        print(f"Sending request: {request}")

        response = self.send_user_request(request)
        print(f"Server response: {response}")

        self.entry_username.clear()
        self.entry_password.clear()
        self.entry_email.clear()

        if response == "Success":
            self.show_message("Реєстрація успішна!")
            self.switch_to_login()
        else:
            self.show_message("Сталася помилка під час реєстрації.")

    def switch_to_login(self):
        self.entry_username.clear()
        self.entry_password.clear()
        self.entry_email.clear()

        self.registration_mode = False
        self.register_button.setText("Зареєструватися")
        self.register_button.clicked.disconnect()
        self.register_button.clicked.connect(self.register)
        self.label_email.hide()
        self.entry_email.hide()

    def show_message(self, message):
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Information)
        msg.setText(message)
        msg.setWindowTitle("Увага")
        msg.exec_()

    def send_user_request(self, request):
        try:
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect(('127.0.0.1', 8080))

            client_socket.sendall(request.encode('utf-8'))
            response = client_socket.recv(1024).decode('utf-8')
            client_socket.close()
            return response
        except Exception as e:
            self.show_message(f"Не вдалося зв'язатися із сервером: {e}")
            return None

    def login(self):
        username = self.entry_username.text().strip()
        password = self.entry_password.text().strip()

        if not username or not password:
            self.show_message("Будь ласка, заповніть всі поля для входу.")
            return

        request = f"LOGIN {username} {password}"
        print(f"Sending request: {request}")

        response = self.send_user_request(request)
        print(f"Response from server: {response}")

        self.entry_username.clear()
        self.entry_password.clear()

        if response.startswith("Success"):
            try:
                parts = response.split()
                if len(parts) > 1:
                    user_id = parts[1]
                    self.username = username
                    self.id = user_id
                    self.open_main_window()
                else:
                    self.show_message("Помилка отримання ID користувача: неповна відповідь.")
            except Exception as e:
                self.show_message(f"Помилка: {str(e)}")
                print(f"Error: {str(e)}")
        else:
            self.show_message("Неправильне ім'я користувача або пароль.")

    def open_main_window(self):
        try:
            self.main_window = MainWindow(self.username, self.id, self)
            self.main_window.show()
            self.close()

            self.add_book_page = AddBookPage(self.id)
            self.edit_book_page = EditBookPage(self.user_id)
            self.view_all_books_page = ViewAllBooksPage(self.id)
            self.delete_book_page = DeleteBookPage(self.user_id)
            self.stats_page = StatsPage(self.user_id)
            self.search_book_page = SearchBookPage(self.user_id)

        except Exception as e:
            QMessageBox.critical(self, "Помилка", f"Не вдалося відкрити головне вікно: {e}")


class MainWindow(QWidget):
    def __init__(self, username, user_id, login_window, photo_path=None):
        super().__init__()

        self.username = username
        self.user_id = user_id
        self.login_window = login_window  # Вікно входу
        self.setWindowTitle("Домашня бібліотека")
        self.setGeometry(100, 100, 800, 600)
        self.setStyleSheet("background-color: #606060;")
        self.photo_path = photo_path or "/path/to/default/photo.jpg"

        main_layout = QHBoxLayout()

        left_panel = QVBoxLayout()

        self.user_photo = QLabel()
        self.load_user_photo()
        self.user_photo.setAlignment(Qt.AlignLeft)
        self.user_photo.setStyleSheet("margin: 20px;")
        left_panel.addWidget(self.user_photo)

        self.user_label = QLabel(f"{self.username}")
        self.user_label.setAlignment(Qt.AlignLeft)
        self.user_label.setStyleSheet("color: white; font-size: 20px; font-weight: bold; margin-left: 45px;")
        left_panel.addWidget(self.user_label)

        self.button_panel = QVBoxLayout()
        button_style = """
            background-color: #556080; 
            color: white; 
            font-size: 16px; 
            padding: 18px; 
            border-radius: 10px;
        """

        self.add_book_button = QPushButton("Додати книгу")
        self.add_book_button.setStyleSheet(button_style)
        self.add_book_button.clicked.connect(self.show_add_book)
        self.button_panel.addWidget(self.add_book_button)

        self.edit_book_button = QPushButton("Редагувати книгу")
        self.edit_book_button.setStyleSheet(button_style)
        self.edit_book_button.clicked.connect(self.show_edit_book)
        self.button_panel.addWidget(self.edit_book_button)

        self.delete_book_button = QPushButton("Видалити книгу")
        self.delete_book_button.setStyleSheet(button_style)
        self.delete_book_button.clicked.connect(self.show_delete_book)
        self.button_panel.addWidget(self.delete_book_button)

        self.search_button = QPushButton("Пошук книги")
        self.search_button.setStyleSheet(button_style)
        self.search_button.clicked.connect(self.show_search_book)
        self.button_panel.addWidget(self.search_button)

        self.stats_button = QPushButton("Статистика")
        self.stats_button.setStyleSheet(button_style)
        self.stats_button.clicked.connect(self.show_stats)
        self.button_panel.addWidget(self.stats_button)

        self.view_button = QPushButton("Перегляд усіх книг")
        self.view_button.setStyleSheet(button_style)
        self.view_button.clicked.connect(self.show_view_all_books)
        self.button_panel.addWidget(self.view_button)

        self.button_panel.addStretch()

        self.logout_button = QPushButton("Вийти з акаунта")
        self.logout_button.setStyleSheet(button_style)
        self.logout_button.clicked.connect(self.logout)
        self.button_panel.addWidget(self.logout_button)

        left_panel.addLayout(self.button_panel)

        self.content_stack = QStackedWidget()

        image_label = QLabel()
        image_path = "/Users/test/PycharmProjects/client/books.png"

        pixmap = QPixmap(image_path)
        if pixmap.isNull():
            print("Помилка: Зображення не завантажено. Перевірте шлях:", image_path)
        else:
            scaled_pixmap = pixmap.scaled(500, 500, Qt.KeepAspectRatio, Qt.SmoothTransformation)
            image_label.setPixmap(scaled_pixmap)

        image_label.setAlignment(Qt.AlignRight| Qt.AlignBottom)
        image_label.setStyleSheet("margin: 20px;")

        self.image_page = QWidget()
        image_layout = QVBoxLayout()
        image_layout.addWidget(image_label)
        self.image_page.setLayout(image_layout)

        self.content_stack.addWidget(self.image_page)

        self.content_stack.setCurrentWidget(self.image_page)

        self.add_book_page = AddBookPage(self.user_id)
        self.content_stack.addWidget(self.add_book_page)

        self.edit_book_page = EditBookPage(self.user_id)
        self.content_stack.addWidget(self.edit_book_page)

        self.delete_book_page = DeleteBookPage(self.user_id)
        self.content_stack.addWidget(self.delete_book_page)

        self.search_book_page = SearchBookPage(self.user_id)
        self.content_stack.addWidget(self.search_book_page)

        self.stats_page = StatsPage(self.user_id)
        self.content_stack.addWidget(self.stats_page)

        self.view_all_books_page = ViewAllBooksPage(self.user_id)
        self.content_stack.addWidget(self.view_all_books_page)

        main_layout.addLayout(left_panel)
        main_layout.addWidget(self.content_stack)

        self.setLayout(main_layout)

    def load_user_photo(self):
        photo_path = "/Users/test/PycharmProjects/client/user.png"
        pixmap = QPixmap(photo_path)

        if pixmap.isNull():
            print("Помилка: зображення не завантажено")
            pixmap = QPixmap("path_to_default_image.jpg")

        self.user_photo.setPixmap(pixmap.scaled(120, 120, Qt.KeepAspectRatio, Qt.SmoothTransformation))

    def change_photo(self):
        file_dialog = QFileDialog()
        file_dialog.setNameFilter("Image files (*.png *.jpg *.bmp *.jpeg)")
        file_dialog.setFileMode(QFileDialog.ExistingFile)

        if file_dialog.exec_():
            file_path = file_dialog.selectedFiles()[0]
            pixmap = QPixmap(file_path)

            if not pixmap.isNull():
                self.user_photo.setPixmap(pixmap.scaled(100, 100))
            else:
                print("Помилка: не вдалося завантажити зображення.")

    def show_add_book(self):
        self.content_stack.setCurrentWidget(self.add_book_page)

    def show_edit_book(self):
        self.content_stack.setCurrentWidget(self.edit_book_page)

    def show_delete_book(self):
        self.content_stack.setCurrentWidget(self.delete_book_page)

    def show_search_book(self):
        self.content_stack.setCurrentWidget(self.search_book_page)

    def show_stats(self):
        self.content_stack.setCurrentWidget(self.stats_page)

    def show_view_all_books(self):
        self.content_stack.setCurrentWidget(self.view_all_books_page)

    def logout(self):
        self.close()
        self.login_window.show()


class AddBookPage(QWidget):
    def __init__(self, user_id=None):
        super().__init__()
        self.user_id = user_id

        self.layout = QVBoxLayout()

        self.author_label = QLabel("Автор:")
        self.author_label.setStyleSheet("color: white; font-size: 16px;")
        self.author_input = QLineEdit()
        self.author_input.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.author_label)
        self.layout.addWidget(self.author_input)

        self.title_label = QLabel("Назва:")
        self.title_label.setStyleSheet("color: white; font-size: 16px;")
        self.title_input = QLineEdit()
        self.title_input.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.title_label)
        self.layout.addWidget(self.title_input)

        self.year_label = QLabel("Рік видання:")
        self.year_label.setStyleSheet("color: white; font-size: 16px;")
        self.year_input = QSpinBox()
        self.year_input.setRange(1000, 2024)
        self.year_input.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.year_label)
        self.layout.addWidget(self.year_input)

        self.genre_label = QLabel("Жанр:")
        self.genre_label.setStyleSheet("color: white; font-size: 16px;")
        self.genre_input = QComboBox()
        self.genre_input.addItems(["Romance", "Fantasy", "Fiction", "Thriller", "Other"])
        self.genre_input.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.genre_label)
        self.layout.addWidget(self.genre_input)

        self.read_status_label = QLabel("Статус читання:")
        self.read_status_label.setStyleSheet("color: white; font-size: 16px;")
        self.layout.addWidget(self.read_status_label)

        self.read_button_group = QButtonGroup()
        self.read_radio = QRadioButton("Прочитана")
        self.read_radio.setStyleSheet("color: white; font-size: 16px;")
        self.unread_radio = QRadioButton("Непрочитана")
        self.unread_radio.setStyleSheet("color: white; font-size: 16px;")
        self.read_radio.setChecked(True)

        self.read_status_layout = QHBoxLayout()
        self.read_status_layout.addWidget(self.read_radio)
        self.read_status_layout.addWidget(self.unread_radio)
        self.layout.addLayout(self.read_status_layout)

        self.read_button_group.addButton(self.read_radio)
        self.read_button_group.addButton(self.unread_radio)

        self.rating_label = QLabel("Оцінка:")
        self.rating_label.setStyleSheet("color: white; font-size: 16px;")
        self.rating_input = QSpinBox()
        self.rating_input.setRange(0, 5)
        self.rating_input.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.rating_label)
        self.layout.addWidget(self.rating_input)

        self.button_layout = QHBoxLayout()

        self.save_button = QPushButton("Зберегти книгу")
        self.save_button.setStyleSheet(
            "background-color: #556080; color: white; font-size: 14px; padding: 14px; border-radius: 13px;")
        self.save_button.clicked.connect(self.save_book)
        self.button_layout.addWidget(self.save_button)

        self.layout.addLayout(self.button_layout)
        self.setLayout(self.layout)

    def save_book(self):
        title = self.title_input.text()
        author = self.author_input.text()
        year = self.year_input.value()
        genre = self.genre_input.currentText()
        read_status = self.read_radio.text() if self.read_radio.isChecked() else self.unread_radio.text()
        rating = self.rating_input.value()

        if author and title:
            try:
                title = f'"{title}"' if ' ' in title else title
                author = f'"{author}"' if ' ' in author else author

                print(
                    f"Title: {title}, Author: {author}, Year: {year}, Genre: {genre}, Read Status: {read_status}, Rating: {rating}, User ID: {self.user_id}")

                client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client_socket.connect(('127.0.0.1', 8080))

                request = f"INSERT_BOOK {title} {author}  {year} {genre} {read_status} {rating} {self.user_id}"
                client_socket.sendall(request.encode())

                response = client_socket.recv(1024).decode()
                QMessageBox.information(self, "Інформація", response)

                client_socket.close()

            except Exception as e:
                QMessageBox.warning(self, "Помилка", f"Не вдалося з'єднатися з сервером: {str(e)}")
        else:
            QMessageBox.warning(self, "Помилка", "Будь ласка, заповніть всі обов'язкові поля.")

class EditBookPage(QWidget):
    def __init__(self, user_id=None):
        super().__init__()
        self.user_id = user_id

        self.layout = QVBoxLayout()

        self.title_label = QLabel("Назва книги для редагування:")
        self.title_label.setStyleSheet("color: white; font-size: 16px;")
        self.title_input = QLineEdit()
        self.title_input.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.title_label)
        self.layout.addWidget(self.title_input)

        self.read_status_label = QLabel("Статус прочитання:")
        self.read_status_label.setStyleSheet("color: white; font-size: 16px;")
        self.read_status_combo = QComboBox()
        self.read_status_combo.addItems(["Прочитано", "Не прочитано"])
        self.read_status_combo.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.read_status_label)
        self.layout.addWidget(self.read_status_combo)

        self.rating_label = QLabel("Оцінка:")
        self.rating_label.setStyleSheet("color: white; font-size: 16px;")
        self.rating_input = QSpinBox()
        self.rating_input.setRange(1, 5)
        self.rating_input.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.rating_label)
        self.layout.addWidget(self.rating_input)

        self.button_layout = QHBoxLayout()

        self.save_button = QPushButton("Зберегти зміни")
        self.save_button.setStyleSheet("background-color: #556080; color: white; font-size: 14px; padding: 14px; border-radius: 13px;")
        self.save_button.clicked.connect(self.edit_book)
        self.button_layout.addWidget(self.save_button)

        self.cancel_button = QPushButton("Скасувати")
        self.cancel_button.setStyleSheet("background-color: #556080; color: white; font-size: 14px; padding: 9px; border-radius: 13px;")
        self.cancel_button.clicked.connect(self.close)
        self.button_layout.addWidget(self.cancel_button)

        self.layout.addLayout(self.button_layout)
        self.setLayout(self.layout)

    def edit_book(self):
        title = self.title_input.text()
        read_status = self.read_status_combo.currentText()
        rating = self.rating_input.text()

        if title and rating.isdigit():
            try:
                client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client_socket.connect(('127.0.0.1', 8080))

                request_data = f"EDIT \"{title}\" \"{read_status}\" {rating} {self.user_id}"
                client_socket.sendall(request_data.encode())

                response = client_socket.recv(4096).decode()

                if response.strip() == "SUCCESS":
                    QLabel("Зміни успішно збережено").show()
                else:
                    QLabel("Не вдалося редагувати книгу: " + response).show()

                client_socket.close()
                self.close()

            except Exception as e:
                QLabel(f"Помилка під час редагування: {e}").show()
        else:
            QLabel("Будь ласка, заповніть усі поля та введіть правильну оцінку.").show()


class DeleteBookPage(QWidget):
    def __init__(self, user_id=None):
        super().__init__()
        self.user_id = user_id

        self.layout = QVBoxLayout()
        self.layout.setSpacing(20)

        self.title_label = QLabel("Назва книги для видалення:")
        self.title_label.setStyleSheet("color: white; font-size: 16px;")
        self.title_input = QLineEdit()
        self.title_input.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.title_label)
        self.layout.addWidget(self.title_input)

        self.author_label = QLabel("Автор книги для видалення:")
        self.author_label.setStyleSheet("color: white; font-size: 16px;")
        self.author_input = QLineEdit()
        self.author_input.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.author_label)
        self.layout.addWidget(self.author_input)

        self.delete_button = QPushButton("Видалити книгу")
        self.delete_button.setStyleSheet("background-color: #556080; color: white; font-size: 14px; padding: 10px; border-radius: 10px;")
        self.delete_button.clicked.connect(self.delete_book)
        self.layout.addWidget(self.delete_button)

        self.cancel_button = QPushButton("Скасувати")
        self.cancel_button.setStyleSheet("background-color: #556080; color: white; font-size: 14px; padding: 10px; border-radius: 10px;")
        self.cancel_button.clicked.connect(self.close)
        self.layout.addWidget(self.cancel_button)

        self.setLayout(self.layout)

    def delete_book(self):
        title = self.title_input.text()
        author = self.author_input.text()

        if title and author:
            try:
                client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client_socket.connect(('127.0.0.1', 8080))

                request = f"DELETE_BOOK \"{title}\" \"{author}\" {self.user_id}"
                client_socket.sendall(request.encode())

                response = client_socket.recv(4096).decode()
                client_socket.close()

                if response == "Book deleted successfully":
                    QMessageBox.information(self, "Інформація", f"Книга '{title}' успішно видалена!")
                else:
                    QMessageBox.warning(self, "Помилка", f"Не вдалося видалити книгу '{title}'.")

                self.close()

            except Exception as e:
                QMessageBox.warning(self, "Помилка", f"Не вдалося підключитися до сервера: {str(e)}")
        else:
            QMessageBox.warning(self, "Помилка", "Будь ласка, введіть назву та автора книги для видалення.")


class SearchBookPage(QWidget):
    def __init__(self, user_id=None):
        super().__init__()
        self.user_id = user_id

        self.layout = QVBoxLayout()

        self.results_table = QTableWidget()
        self.results_table.setColumnCount(6)
        self.results_table.setHorizontalHeaderLabels(["Назва", "Автор", "Рік", "Жанр", "Прочитана", "Оцінка"])
        self.results_table.setStyleSheet("background-color: white; border: 1px solid black;")
        self.layout.addWidget(self.results_table)

        self.setLayout(self.layout)
        self.title_label = QLabel("Назва книги:")
        self.title_label.setStyleSheet("color: white; font-size: 16px;")
        self.title_input = QLineEdit()
        self.title_input.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.title_label)
        self.layout.addWidget(self.title_input)

        self.author_label = QLabel("Автор:")
        self.author_label.setStyleSheet("color: white; font-size: 16px;")
        self.author_input = QLineEdit()
        self.author_input.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.author_label)
        self.layout.addWidget(self.author_input)

        self.button_layout = QHBoxLayout()

        self.search_button = QPushButton("Шукати")
        self.search_button.setStyleSheet("background-color: #556080; color: white; font-size: 14px; padding: 14px; border-radius: 13px;")
        self.search_button.clicked.connect(self.search_books)
        self.button_layout.addWidget(self.search_button)

        self.cancel_button = QPushButton("Скасувати")
        self.cancel_button.setStyleSheet("background-color: #556080; color: white; font-size: 14px; padding: 14px; border-radius: 13px;")
        self.cancel_button.clicked.connect(self.close)
        self.button_layout.addWidget(self.cancel_button)

        self.layout.addLayout(self.button_layout)

    def search_books(self):
        title = self.title_input.text()
        author = self.author_input.text()

        if title and author and self.user_id:
            try:
                client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client_socket.connect(('127.0.0.1', 8080))

                request_data = f"SEARCH \"{title}\" \"{author}\" {self.user_id}"
                client_socket.sendall(request_data.encode())

                response = client_socket.recv(4096).decode()

                if not response.strip():
                    self.results_table.setRowCount(0)
                    self.results_table.setColumnCount(6)
                    self.results_table.setHorizontalHeaderLabels(["Назва", "Автор", "Рік", "Жанр", "Прочитана", "Оцінка"])
                else:
                    self.results_table.setRowCount(0)

                    rows = response.strip().split("\n")
                    for row in rows:
                        columns = row.split("|")
                        if len(columns) == 6:
                            row_idx = self.results_table.rowCount()
                            self.results_table.insertRow(row_idx)
                            for col_idx, col_data in enumerate(columns):
                                self.results_table.setItem(row_idx, col_idx, QTableWidgetItem(col_data.strip()))

                client_socket.close()

            except Exception as e:
                error_item = QTableWidgetItem(f"Помилка під час пошуку: {e}")
                self.results_table.setRowCount(1)
                self.results_table.setItem(0, 0, error_item)
        else:
            error_item = QTableWidgetItem("Будь ласка, введіть автора та назву книги.")
            self.results_table.setRowCount(1)
            self.results_table.setItem(0, 0, error_item)

class StatsPage(QWidget):
    def __init__(self, user_id=None):
        super().__init__()
        self.user_id = user_id

        self.layout = QVBoxLayout()

        self.table = QTableWidget()
        self.table.setStyleSheet("background-color: white;")
        self.layout.addWidget(self.table)

        self.table.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        header = self.table.horizontalHeader()
        header.setSectionResizeMode(QHeaderView.Stretch)

        self.setLayout(self.layout)

        self.read_unread_button = QPushButton("Показати Прочитані/Непрочитані книги")
        self.read_unread_button.setStyleSheet(
            "background-color: #556080; color: white; font-size: 14px; padding: 14px; border-radius: 13px;"
        )
        self.read_unread_button.clicked.connect(self.show_read_unread_books)

        self.rating_button = QPushButton("Показати Рейтинг книг")
        self.rating_button.setStyleSheet(
            "background-color: #556080; color: white; font-size: 14px; padding: 14px; border-radius: 13px;"
        )
        self.rating_button.clicked.connect(self.show_book_ratings)

        self.layout.addWidget(self.read_unread_button)
        self.layout.addWidget(self.rating_button)

    def show_read_unread_books(self):
        try:
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect(('127.0.0.1', 8080))
            request = f"GET_READ_UNREAD {self.user_id}"
            client_socket.sendall(request.encode())

            response = client_socket.recv(4096).decode()
            client_socket.close()

            read_books, unread_books = response.split("|")
            read_books = read_books.split(";") if read_books else []
            unread_books = unread_books.split(";") if unread_books else []

            self.table.clear()
            self.table.setColumnCount(2)
            self.table.setHorizontalHeaderLabels(["Прочитані книги", "Непрочитані книги"])

            max_rows = max(len(read_books), len(unread_books))
            self.table.setRowCount(max_rows)

            for row in range(max_rows):
                if row < len(read_books):
                    self.table.setItem(row, 0, QTableWidgetItem(read_books[row]))
                if row < len(unread_books):
                    self.table.setItem(row, 1, QTableWidgetItem(unread_books[row]))

        except Exception as e:
            QMessageBox.warning(self, "Помилка", f"Не вдалося отримати дані з сервера: {str(e)}")

    def show_book_ratings(self):
        try:
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect(('127.0.0.1', 8080))
            request = f"GET_BOOK_RATINGS {self.user_id}"
            client_socket.sendall(request.encode())

            response = client_socket.recv(4096).decode()
            client_socket.close()

            books = [line.split(";") for line in response.split("\n") if line.strip()]

            self.table.clear()
            self.table.setColumnCount(2)
            self.table.setHorizontalHeaderLabels(["Назва книги", "Рейтинг"])

            self.table.setRowCount(len(books))
            for row, book in enumerate(books):
                self.table.setItem(row, 0, QTableWidgetItem(book[0]))
                self.table.setItem(row, 1, QTableWidgetItem(book[1]))

        except Exception as e:
            QMessageBox.warning(self, "Помилка", f"Не вдалося отримати дані з сервера: {str(e)}")


class ViewAllBooksPage(QWidget):
    def __init__(self, user_id=None):
        super().__init__()
        self.user_id = user_id

        self.layout = QVBoxLayout()

        self.books_table = QTableWidget()
        self.books_table.setColumnCount(6)
        self.books_table.setHorizontalHeaderLabels(["Назва", "Автор", "Рік", "Жанр", "Статус читання", "Оцінка"])
        self.books_table.setStyleSheet("background-color: white;")

        self.books_table.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        header = self.books_table.horizontalHeader()
        header.setSectionResizeMode(QHeaderView.Stretch)

        self.layout.addWidget(self.books_table)

        self.refresh_button = QPushButton("Оновити список")
        self.refresh_button.setStyleSheet(
            "background-color: #556080; color: white; font-size: 14px; padding: 14px; border-radius: 13px;")
        self.refresh_button.clicked.connect(self.load_books_from_server)
        self.layout.addWidget(self.refresh_button)

        self.setLayout(self.layout)


    def load_books_from_server(self, author="", title="", year="", genre="", read_status="", rating=""):
        try:
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect(('127.0.0.1', 8080))

            request = f"GET_ALL_BOOKS {author} {title} {year} {genre} {read_status} {rating} {self.user_id}"
            print(f"Sending request: {author} {title} {year} {genre} {read_status} {rating} {self.user_id}")
            client_socket.sendall(request.encode())

            response = client_socket.recv(4096).decode()
            client_socket.close()

            self.books_table.setRowCount(0)

            books = response.split("\n")
            for book in books:
                if book.strip():
                    data = book.split(";")
                    row_position = self.books_table.rowCount()
                    self.books_table.insertRow(row_position)
                    for col, item in enumerate(data):
                        self.books_table.setItem(row_position, col, QTableWidgetItem(item))

        except Exception as e:
            QMessageBox.warning(self, "Помилка", f"Не вдалося отримати дані з сервера: {str(e)}")

def main():
    app = QApplication(sys.argv)
    login_window = LoginWindow()
    login_window.show()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
