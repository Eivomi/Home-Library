#include <iostream>
#include <string>
#include <sql.h>
#include <sqlext.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <vector>

class User {
private:
    int id;
    std::string username;
    std::string mail;
    std::string password;

public:
    User(int id, const std::string& username, const std::string& mail, const std::string& password)
        : id(id), username(username), mail(mail), password(password) {}

    int getId() const { return id; }
    std::string getUsername() const { return username; }
    std::string getMail() const { return mail; }
    std::string getPassword() const { return password; }

    void displayUser() const {
        std::cout << "User ID: " << id << "\n"
                  << "Username: " << username << "\n"
                  << "Mail: " << mail << "\n";
    }
};

class Book {
private:
    int id;
    std::string title;
    std::string author;
    int year;
    std::string genre;
    bool readStatus;
    float rating;
    int userId;

public:
    Book(int id, const std::string& title, const std::string& author, int year,
         const std::string& genre, bool readStatus, float rating, int userId)
        : id(id), title(title), author(author), year(year), genre(genre),
          readStatus(readStatus), rating(rating), userId(userId) {}

    int getId() const { return id; }
    std::string getTitle() const { return title; }
    std::string getAuthor() const { return author; }
    int getYear() const { return year; }
    std::string getGenre() const { return genre; }
    bool isRead() const { return readStatus; }
    float getRating() const { return rating; }
    int getUserId() const { return userId; }

    void displayBook() const {
        std::cout << "ID: " << id << "\n"
                  << "Title: " << title << "\n"
                  << "Author: " << author << "\n"
                  << "Year: " << year << "\n"
                  << "Genre: " << genre << "\n"
                  << "Read Status: " << (readStatus ? "Yes" : "No") << "\n"
                  << "Rating: " << rating << "\n"
                  << "User ID: " << userId << "\n";
    }
};

void connectToDatabase() {
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLRETURN ret;

    // Allocate environment handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating environment handle." << std::endl;
        return;
    }

    // Set the ODBC version environment attribute
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    // Allocate connection handle
    SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

    // Connect to the database (replace placeholders with your credentials)
    ret = SQLConnect(hdbc, (SQLCHAR*)"SQLServerDSN", SQL_NTS,
                     (SQLCHAR*)"sa", SQL_NTS,
                     (SQLCHAR*)"040406.Zap", SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Failed to connect to the database!" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return;
    }
    // Allocate statement handle
    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    // Insert query
    const char* insertQuery =
        "INSERT INTO LibraryDB.dbo.books (title, author, [year], genre, read_status, rating, user_id) "
        "VALUES ('Test', 'J.D. Salinger', 1951, 'Fiction', 1, 5, 1)";
    
    ret = SQLExecDirect(hstmt, (SQLCHAR*)insertQuery, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error inserting data." << std::endl;
    } else {
        std::cout << "Book inserted successfully!" << std::endl;
    }

    // SELECT query
    const char* selectQuery =
        "SELECT id, title, author, [year], genre, read_status, rating, user_id "
        "FROM LibraryDB.dbo.books WHERE title = 'Test'";

    ret = SQLExecDirect(hstmt, (SQLCHAR*)selectQuery, SQL_NTS);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        int id, year, userId;
        char title[256], author[256], genre[256];
        bool readStatus;
        float rating;

        // Bind columns
        SQLBindCol(hstmt, 1, SQL_C_LONG, &id, 0, NULL);
        SQLBindCol(hstmt, 2, SQL_C_CHAR, title, sizeof(title), NULL);
        SQLBindCol(hstmt, 3, SQL_C_CHAR, author, sizeof(author), NULL);
        SQLBindCol(hstmt, 4, SQL_C_LONG, &year, 0, NULL);
        SQLBindCol(hstmt, 5, SQL_C_CHAR, genre, sizeof(genre), NULL);
        SQLBindCol(hstmt, 6, SQL_C_BIT, &readStatus, 0, NULL);
        SQLBindCol(hstmt, 7, SQL_C_FLOAT, &rating, 0, NULL);
        SQLBindCol(hstmt, 8, SQL_C_LONG, &userId, 0, NULL);

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            Book fetchedBook(id, title, author, year, genre, readStatus, rating, userId);
            fetchedBook.displayBook();
        }
    } else {
        std::cerr << "Error executing SELECT query." << std::endl;
    }

    // Clean up
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

std::string addBook(const std::string &request) {
    std::string author, title, genre;
    int year, rating, userId;
    std::string readStatusStr;
    int readStatusInt;

    std::string data = request.substr(11);

    std::stringstream ss(data);
    std::getline(ss, title, '"');
    std::getline(ss, title, '"');

    std::getline(ss, author, '"');
    std::getline(ss, author, '"');

    ss >> year >> genre >> readStatusStr >> rating >> userId;

    if (author.empty() || title.empty() || year <= 0 || rating > 5) {
        return "Invalid input data.";
    }

    if (readStatusStr == "Прочитана") {
        readStatusInt = 1;
    } else if (readStatusStr == "Непрочитана") {
        readStatusInt = 0;
    } else {
        return "Invalid read status.";
    }

    std::vector<std::string> params = {title, author, std::to_string(year), genre, std::to_string(readStatusInt), std::to_string(rating), std::to_string(userId)};

    std::string insertQuery =
        "INSERT INTO LibraryDB.dbo.books (title, author, [year], genre, read_status, rating, user_id) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)";
    
    // Connect to the database
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLRETURN ret;

    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating environment handle." << std::endl;
        return "Error inserting book.";
    }

    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error setting ODBC version." << std::endl;
        return "Error inserting book.";
    }

    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating connection handle." << std::endl;
        return "Error inserting book.";
    }

    ret = SQLConnect(hdbc, (SQLCHAR*)"SQLServerDSN", SQL_NTS, (SQLCHAR*)"sa", SQL_NTS, (SQLCHAR*)"040406.Zap", SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Failed to connect to the database!" << std::endl;
        return "Error inserting book.";
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating statement handle." << std::endl;
        return "Error inserting book.";
    }

    // Prepare the SQL statement
    ret = SQLPrepare(hstmt, (SQLCHAR*)insertQuery.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error preparing SQL statement." << std::endl;
        return "Error inserting book.";
    }

    // Bind parameters dynamically from the vector
    for (size_t i = 0; i < params.size(); ++i) {
        SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)params[i].c_str(), 0, NULL);
    }

    // Execute the SQL statement
    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLCHAR sqlState[6], messageText[256];
        SQLINTEGER nativeError;
        SQLSMALLINT textLength;
        SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlState, &nativeError, messageText, sizeof(messageText), &textLength);
        std::cerr << "SQL error state: " << sqlState << ", message: " << messageText << std::endl;
        return "Error executing SQL query.";
    } else {
        std::cout << "Book inserted successfully" << std::endl;
    }

    // Clean up
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return "Book inserted successfully";
}

std::string getAllBooks(const std::string &request) {
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLRETURN ret;
    std::ostringstream result;

    // Extract user_id from the request
    size_t userIdPos = request.rfind(' ');
    int userId = std::stoi(request.substr(userIdPos + 1));

    // Allocate environment handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating environment handle.";
    }

    // Set the ODBC version environment attribute
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error setting ODBC version.";
    }

    // Allocate connection handle
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating connection handle.";
    }

    // Connect to the database
    ret = SQLConnect(hdbc, (SQLCHAR*)"SQLServerDSN", SQL_NTS, (SQLCHAR*)"sa", SQL_NTS, (SQLCHAR*)"040406.Zap", SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Failed to connect to the database!";
    }

    // Allocate statement handle
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating statement handle.";
    }

    // Modify query to include user_id
    std::ostringstream queryStream;
    queryStream << "SELECT title, author, [year], genre, read_status, rating FROM LibraryDB.dbo.books WHERE user_id = " << userId;
    std::string query = queryStream.str();

    // Execute SELECT query
    ret = SQLExecDirect(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error executing SELECT query.";
    }

    // Bind columns
    int year, readStatus;
    float rating;
    char title[255], author[255], genre[255];

    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_CHAR, title, sizeof(title), NULL);
        SQLGetData(hstmt, 2, SQL_C_CHAR, author, sizeof(author), NULL);
        SQLGetData(hstmt, 3, SQL_C_LONG, &year, 0, NULL);
        SQLGetData(hstmt, 4, SQL_C_CHAR, genre, sizeof(genre), NULL);
        SQLGetData(hstmt, 5, SQL_C_LONG, &readStatus, 0, NULL);
        SQLGetData(hstmt, 6, SQL_C_FLOAT, &rating, 0, NULL);

        result << title << ";"
               << author << ";"
               << year << ";"
               << genre << ";"
               << (readStatus == 1 ? "Read" : "Unread") << ";"
               << rating << "\n";
    }

    // Clean up
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return result.str();
}

std::string editBook(const std::string &request) {
    SQLHENV henv = nullptr;
    SQLHDBC hdbc = nullptr;
    SQLHSTMT hstmt = nullptr;
    SQLRETURN ret;
    std::ostringstream result;

    // Витягуємо user_id з кінця запиту
    size_t userIdPos = request.rfind(' ');
    if (userIdPos == std::string::npos) {
        return "Invalid request format: user ID not found.";
    }

    std::string userIdStr = request.substr(userIdPos + 1);
    int userId = std::stoi(userIdStr);

    std::stringstream ss(request.substr(5));
    std::string title, readStatusStr;
    int rating;

    std::getline(ss, title, '"');
    std::getline(ss, title, '"');

    std::getline(ss, readStatusStr, '"');
    std::getline(ss, readStatusStr, '"');

    ss >> rating;

    int readStatusNumeric = (readStatusStr == "Прочитано") ? 1 : 0;

    std::string query = "UPDATE LibraryDB.dbo.books "
                        "SET read_status = ?, rating = ? "
                        "WHERE user_id = ? AND title = ?";

    try {
        ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Error allocating environment handle.");
        }

        ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Error setting ODBC version.");
        }

        ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Error allocating connection handle.");
        }

        ret = SQLConnect(hdbc, (SQLCHAR*)"SQLServerDSN", SQL_NTS, (SQLCHAR*)"sa", SQL_NTS, (SQLCHAR*)"040406.Zap", SQL_NTS);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Failed to connect to the database!");
        }

        ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Error allocating statement handle.");
        }

        // Підготовка SQL-запиту
        ret = SQLPrepare(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Error preparing SQL query.");
        }

        // Додавання параметрів до SQL-запиту
        ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &readStatusNumeric, 0, NULL);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Error binding read_status parameter.");
        }

        ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &rating, 0, NULL);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Error binding rating parameter.");
        }

        ret = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userId, 0, NULL);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Error binding user_id parameter.");
        }

        ret = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, title.size(), 0, (SQLCHAR*)title.c_str(), 0, NULL);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Error binding title parameter.");
        }
        ret = SQLExecute(hstmt);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Error executing SQL query.");
        }
 
        SQLLEN rowCount;
        ret = SQLRowCount(hstmt, &rowCount);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Error retrieving row count.");
        }

        if (rowCount > 0) {
            result << "Book \"" << title << "\" successfully updated.";
        } else {
            result << "No rows were updated. Make sure the title and user ID are correct.";
        }
    } catch (const std::exception &e) {
        result << "Error: " << e.what();
    }

    if (hstmt) SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    if (hdbc) SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    if (henv) SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return result.str();
}

std::string deleteBook(const std::string &request) {

    std::string title, author;
    int userId;

    std::stringstream ss(request.substr(12));
    std::getline(ss, title, '"');
    std::getline(ss, title, '"');
    std::getline(ss, author, '"');
    std::getline(ss, author, '"');

    size_t userIdPos = request.rfind(' ');
    if (userIdPos == std::string::npos) {
        return "Error: User ID is missing.";
    }
    userId = std::stoi(request.substr(userIdPos + 1));

    if (title.empty() || author.empty()) {
        return "Error: Title or author is empty.";
    }

    SQLHENV henv = SQL_NULL_HENV;
    SQLHDBC hdbc = SQL_NULL_HDBC;
    SQLHSTMT hstmt = SQL_NULL_HSTMT;
    SQLRETURN ret;

    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating environment handle.";
    }

    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error allocating connection handle.";
    }

    ret = SQLConnect(hdbc, (SQLCHAR*)"SQLServerDSN", SQL_NTS, (SQLCHAR*)"sa", SQL_NTS, (SQLCHAR*)"040406.Zap", SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error connecting to database.";
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error allocating statement handle.";
    }

    std::string checkOwnerQuery = "SELECT COUNT(*) FROM LibraryDB.dbo.books WHERE title = ? AND author = ? AND user_id = ?";
    ret = SQLPrepare(hstmt, (SQLCHAR*)checkOwnerQuery.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error preparing check owner query.";
    }
    SQLLEN userIdLen = sizeof(userId);
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)title.c_str(), 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)author.c_str(), 0, NULL);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userId, 0, &userIdLen);

    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error executing check owner query.";
    }

    SQLLEN count = 0;
    ret = SQLFetch(hstmt);
    if (ret == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_LONG, &count, sizeof(count), NULL);
    }

    if (count == 0) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "You are not the owner of this book.";
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    std::string deleteQuery = "DELETE FROM LibraryDB.dbo.books WHERE title = ? AND author = ? AND user_id = ?";
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error allocating statement handle for delete.";
    }

    ret = SQLPrepare(hstmt, (SQLCHAR*)deleteQuery.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error preparing delete query.";
    }

    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)title.c_str(), 0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)author.c_str(), 0, NULL);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userId, 0, &userIdLen);

    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error executing delete query.";
    }

    // Очистка ресурсів
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return "Book deleted successfully";
}

std::string searchBook(const std::string &request) {
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLRETURN ret;
    std::ostringstream result;

    size_t userIdPos = request.rfind(' ');
    std::string userIdStr = request.substr(userIdPos + 1);
    int userId = std::stoi(userIdStr);

    std::stringstream ss(request.substr(7));
    std::string title, author;
    std::getline(ss, title, '"');
    std::getline(ss, title, '"');
    std::getline(ss, author, '"');
    std::getline(ss, author, '"');

    std::string query = "SELECT title, author, [year], genre, read_status, rating "
                        "FROM LibraryDB.dbo.books "
                        "WHERE user_id = ? AND title LIKE ? AND author LIKE ?";

    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating environment handle.";
    }

    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error setting ODBC version.";
    }

    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating connection handle.";
    }

    ret = SQLConnect(hdbc, (SQLCHAR*)"SQLServerDSN", SQL_NTS, (SQLCHAR*)"sa", SQL_NTS, (SQLCHAR*)"040406.Zap", SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Failed to connect to the database!";
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating statement handle.";
    }

    // Підготовка SQL-запиту
    ret = SQLPrepare(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error preparing SQL query.";
    }

    // Додавання параметрів до SQL-запиту
    std::string titleWildcard = "%" + title + "%";
    std::string authorWildcard = "%" + author + "%";

    ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userId, 0, NULL);
    ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLCHAR*)titleWildcard.c_str(), 0, NULL);
    ret = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLCHAR*)authorWildcard.c_str(), 0, NULL);

    // Виконання SQL-запиту
    ret = SQLExecute(hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error executing SQL query.";
    }

    // Прив'язка колонок для отримання результатів
    char bookTitle[255], bookAuthor[255], genre[255];
    int year, readStatus;
    float rating;

    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_CHAR, bookTitle, sizeof(bookTitle), NULL);
        SQLGetData(hstmt, 2, SQL_C_CHAR, bookAuthor, sizeof(bookAuthor), NULL);
        SQLGetData(hstmt, 3, SQL_C_LONG, &year, 0, NULL);
        SQLGetData(hstmt, 4, SQL_C_CHAR, genre, sizeof(genre), NULL);
        SQLGetData(hstmt, 5, SQL_C_LONG, &readStatus, 0, NULL);
        SQLGetData(hstmt, 6, SQL_C_FLOAT, &rating, 0, NULL);

        result << bookTitle << " | "
               << bookAuthor << " | "
               << year << " | "
               << genre << " | "
               << (readStatus == 1 ? "Read" : "Unread") << " | "
               << rating << "\n";
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return result.str().empty() ? "No books found matching your search criteria." : result.str();
}

std::string getReadUnreadBooks(const std::string &request) {
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLRETURN ret;
    std::ostringstream result;

    // Extract user_id from the request
    size_t userIdPos = request.rfind(' ');
    int userId = std::stoi(request.substr(userIdPos + 1));

    // Allocate environment handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating environment handle.";
    }

    // Set the ODBC version environment attribute
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error setting ODBC version.";
    }

    // Allocate connection handle
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating connection handle.";
    }

    // Connect to the database
    ret = SQLConnect(hdbc, (SQLCHAR*)"SQLServerDSN", SQL_NTS, (SQLCHAR*)"sa", SQL_NTS, (SQLCHAR*)"040406.Zap", SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Failed to connect to the database!";
    }

    // Allocate statement handle
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating statement handle.";
    }

    // Query for read/unread books
    std::ostringstream queryStream;
    queryStream << "SELECT title, read_status FROM LibraryDB.dbo.books WHERE user_id = " << userId;
    std::string query = queryStream.str();

    // Execute SELECT query
    ret = SQLExecDirect(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error executing SELECT query.";
    }

    // Bind columns
    char title[255];
    int readStatus;
    std::ostringstream readBooks, unreadBooks;
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_CHAR, title, sizeof(title), NULL);
        SQLGetData(hstmt, 2, SQL_C_LONG, &readStatus, 0, NULL);

        if (readStatus == 1) {
            readBooks << title << ";";
        } else {
            unreadBooks << title << ";";
        }
    }

    result << readBooks.str() << "|" << unreadBooks.str();

    // Clean up
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return result.str();
}

std::string getBookRatings(const std::string &request) {
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLRETURN ret;
    std::ostringstream result;

    // Extract user_id from the request
    size_t userIdPos = request.rfind(' ');
    int userId = std::stoi(request.substr(userIdPos + 1));

    // Allocate environment handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating environment handle.";
    }

    // Set the ODBC version environment attribute
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error setting ODBC version.";
    }

    // Allocate connection handle
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating connection handle.";
    }

    // Connect to the database
    ret = SQLConnect(hdbc, (SQLCHAR*)"SQLServerDSN", SQL_NTS, (SQLCHAR*)"sa", SQL_NTS, (SQLCHAR*)"040406.Zap", SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Failed to connect to the database!";
    }

    // Allocate statement handle
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error allocating statement handle.";
    }

    // Query for book ratings (sorted and filtered)
    std::ostringstream queryStream;
    queryStream << "SELECT title, rating "
                << "FROM LibraryDB.dbo.books "
                << "WHERE user_id = " << userId << " AND rating > 0 "
                << "ORDER BY rating DESC";
    std::string query = queryStream.str();

    // Execute SELECT query
    ret = SQLExecDirect(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return "Error executing SELECT query.";
    }

    char title[255];
    float rating;

    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_CHAR, title, sizeof(title), NULL);
        SQLGetData(hstmt, 2, SQL_C_FLOAT, &rating, 0, NULL);

        result << title << ";" << rating << "\n";
    }

    // Clean up
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return result.str();
}

std::string registerUser(const std::string &request) {
    std::string username, mail, password;
    std::stringstream ss(request.substr(8));
    ss >> username >> mail >> password;
    
    
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLRETURN ret;
    
    // Підключення до бази даних
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating environment handle." << std::endl;
        return "Error";
    }
    
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error setting ODBC version." << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error";
    }
    
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating connection handle." << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error";
    }
    
    ret = SQLConnect(hdbc, (SQLCHAR*)"SQLServerDSN", SQL_NTS, (SQLCHAR*)"sa", SQL_NTS, (SQLCHAR*)"040406.Zap", SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Failed to connect to the database!" << std::endl;
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error";
    }
    
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating statement handle." << std::endl;
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error";
    }
    
    std::ostringstream checkUsernameQuery;
    checkUsernameQuery << "SELECT COUNT(*) FROM LibraryDB.dbo.users WHERE username = N'" << username << "'";
    
    SQLINTEGER count = 0;
    ret = SQLExecDirect(hstmt, (SQLCHAR*)checkUsernameQuery.str().c_str(), SQL_NTS);
    if (SQL_SUCCEEDED(ret)) {
        SQLBindCol(hstmt, 1, SQL_C_LONG, &count, 0, NULL);
        SQLFetch(hstmt);
        if (count > 0) {
            std::cerr << "Error: Username already exists." << std::endl;
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            SQLDisconnect(hdbc);
            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
            SQLFreeHandle(SQL_HANDLE_ENV, henv);
            return "Error";
        }
    } else {
        std::cerr << "Error executing username check query." << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error";
    }
    
    // Вставка нового користувача
    std::ostringstream insertQuery;
    insertQuery << "INSERT INTO LibraryDB.dbo.users (username, mail, password) VALUES ("
    << "N'" << username << "', "
    << "N'" << mail << "', "
    << "N'" << password << "')";
    
    SQLFreeStmt(hstmt, SQL_CLOSE);
    ret = SQLExecDirect(hstmt, (SQLCHAR*)insertQuery.str().c_str(), SQL_NTS);
    if (SQL_SUCCEEDED(ret)) {
        std::cout << "Registration successful!" << std::endl;
    } else {
        SQLCHAR sqlState[6], messageText[256];
        SQLINTEGER nativeError;
        SQLSMALLINT textLength;
        SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlState, &nativeError, messageText, sizeof(messageText), &textLength);
        std::cerr << "SQL Error: " << messageText << std::endl;
    }
    
    // Звільнення ресурсів
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
    
    username.clear();
    mail.clear();
    password.clear();
    SQLFreeStmt(hstmt, SQL_CLOSE);

    
    return "Success";
}

std::string loginUser(const std::string &request) {
    std::string username, password;
    std::stringstream ss(request.substr(5));
    ss >> username >> password;

    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLRETURN ret;

    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating environment handle." << std::endl;
        return "Error";
    }

    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error setting ODBC version." << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error";
    }

    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating connection handle." << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error";
    }

    ret = SQLConnect(hdbc, (SQLCHAR*)"SQLServerDSN", SQL_NTS, (SQLCHAR*)"sa", SQL_NTS, (SQLCHAR*)"040406.Zap", SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Failed to connect to the database!" << std::endl;
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error";
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating statement handle." << std::endl;
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error";
    }

    std::ostringstream loginQuery;
    loginQuery << "SELECT id FROM LibraryDB.dbo.users WHERE username = N'" << username
               << "' AND password = N'" << password << "'";

    SQLINTEGER user_id = 0;
    ret = SQLExecDirect(hstmt, (SQLCHAR*)loginQuery.str().c_str(), SQL_NTS);
    if (SQL_SUCCEEDED(ret)) {
        SQLBindCol(hstmt, 1, SQL_C_LONG, &user_id, 0, NULL);
        if (SQLFetch(hstmt) == SQL_SUCCESS) {
            // Повертаємо ID користувача
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            SQLDisconnect(hdbc);
            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
            SQLFreeHandle(SQL_HANDLE_ENV, henv);
            return "Success " + std::to_string(user_id);
        } else {
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            SQLDisconnect(hdbc);
            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
            SQLFreeHandle(SQL_HANDLE_ENV, henv);
            return "Invalid credentials";
        }
    } else {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return "Error";
    }
    
}

std::string handleDatabaseRequest(const std::string &request) {
    if (request.find("INSERT_BOOK") == 0) {
        return addBook(request);
    }
    else if(request.find("EDIT") == 0) {
        return editBook(request);
    }
    else if(request.find("GET_ALL_BOOKS") == 0) {
        return getAllBooks(request);
    }
    else if(request.find("DELETE_BOOK") == 0) {
        return deleteBook(request);
    }
    else if(request.find("SEARCH") == 0) {
        return searchBook(request);
    }
    else if(request.find("GET_READ_UNREAD") == 0) {
        return getReadUnreadBooks(request);
    }
    else if(request.find("GET_BOOK_RATINGS") == 0) {
        return getBookRatings(request);
    }
    else if(request.find("REGISTER") == 0) {
        return registerUser(request);
    }
    else if (request.find("LOGIN") == 0) {
        return loginUser(request);
    }
        
    return "Unknown request!";
}

        
void startServer() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    
    // Створення сокета
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port 8080..." << std::endl;

    while (true) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        memset(buffer, 0, sizeof(buffer));
        
        read(new_socket, buffer, 1024);
        std::string request(buffer);
        std::cout << "Received request: " << request << std::endl;

        std::string response = handleDatabaseRequest(request);
        send(new_socket, response.c_str(), response.size(), 0);

        close(new_socket);
    }
}

int main() {
    startServer();
    return 0;
}
