#include "httplib.h"
#include "library.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Helper to manual JSON serialization (since we don't have a JSON lib)
// In a real app, use nlohmann/json
std::string escape(const std::string &s) {
  std::string res;
  for (char c : s) {
    if (c == '"')
      res += "\\\"";
    else if (c == '\\')
      res += "\\\\";
    else
      res += c;
  }
  return res;
}

std::string bookToJson(const Book &b) {
  std::stringstream ss;
  ss << "{\"id\": " << b.id << ", \"title\": \"" << escape(b.title)
     << "\", \"author\": \"" << escape(b.author)
     << "\", \"isAvailable\": " << (b.isAvailable ? "true" : "false")
     << ", \"borrowedBy\": \"" << escape(b.borrowedBy) << "\", \"genre\": \""
     << escape(b.genre) << "\", \"coverUrl\": \"" << escape(b.coverUrl)
     << "\", \"dueDate\": " << b.dueDate << ", \"rating\": " << b.rating
     << ", \"ratingCount\": " << b.ratingCount << "}";
  return ss.str();
}

std::string booksToJson(const std::vector<Book> &books) {
  std::stringstream ss;
  ss << "[";
  for (size_t i = 0; i < books.size(); ++i) {
    ss << bookToJson(books[i]);
    if (i < books.size() - 1)
      ss << ", ";
  }
  ss << "]";
  return ss.str();
}

// Simple JSON parser for specific keys
std::string getJsonValue(const std::string &json, const std::string &key) {
  size_t pos = json.find("\"" + key + "\"");
  if (pos == std::string::npos)
    return "";
  size_t colonPos = json.find(":", pos);
  if (colonPos == std::string::npos)
    return "";

  size_t start = json.find_first_not_of(" :", colonPos + 1);
  if (start == std::string::npos)
    return "";

  if (json[start] == '\"') {
    start++; // skip quote
    size_t end = json.find("\"", start);
    if (end == std::string::npos)
      return "";
    return json.substr(start, end - start);
  } else {
    // Handle unquoted values (numbers)
    size_t end = json.find_first_of(",}", start);
    if (end == std::string::npos)
      return json.substr(start);
    return json.substr(start, end - start);
  }
}

int main() {
  httplib::Server svr;

  // CORS Headers
  auto set_cors = [](httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods",
                   "GET, POST, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
  };

  svr.Get("/books", [&](const httplib::Request &, httplib::Response &res) {
    res.set_content(booksToJson(getBooks()), "application/json");
    set_cors(res);
  });

  svr.Post("/books", [&](const httplib::Request &req, httplib::Response &res) {
    std::string title = getJsonValue(req.body, "title");
    std::string author = getJsonValue(req.body, "author");
    std::string genre = getJsonValue(req.body, "genre");
    std::string coverUrl = getJsonValue(req.body, "coverUrl");

    if (title.empty() || author.empty()) {
      res.status = 400;
      set_cors(res);
      return;
    }

    int id = rand() % 10000;
    addBook(id, title, author, genre, coverUrl);
    saveBooks(); // Persist changes
    res.status = 201;
    set_cors(res);
  });

  svr.Delete(R"(/books/(\d+))",
             [&](const httplib::Request &req, httplib::Response &res) {
               int id = std::stoi(req.matches[1]);
               if (deleteBook(id)) {
                 saveBooks(); // Persist changes
                 res.status = 200;
               } else {
                 res.status = 404;
               }
               set_cors(res);
             });

  // Borrow endpoint
  svr.Post(R"(/books/(\d+)/borrow)",
           [&](const httplib::Request &req, httplib::Response &res) {
             int id = std::stoi(req.matches[1]);
             std::string user = getJsonValue(req.body, "username");
             if (borrowBook(id, user)) {
               saveBooks();
               res.status = 200;
             } else {
               res.status = 400; // Book unavailable or not found
             }
             set_cors(res);
           });

  // Return endpoint
  svr.Post(R"(/books/(\d+)/return)",
           [&](const httplib::Request &req, httplib::Response &res) {
             int id = std::stoi(req.matches[1]);
             if (returnBook(id)) {
               saveBooks();
               res.status = 200;
             } else {
               res.status = 400;
             }
             set_cors(res);
           });

  // Rate endpoint
  svr.Post(R"(/books/(\d+)/rate)",
           [&](const httplib::Request &req, httplib::Response &res) {
             int id = std::stoi(req.matches[1]);
             std::string starsStr = getJsonValue(req.body, "stars");
             if (!starsStr.empty()) {
               int stars = std::stoi(starsStr);
               addRating(id, stars);
               saveBooks();
               res.status = 200;
             } else {
               res.status = 400;
             }
             set_cors(res);
           });

  svr.Options(R"(.*)", [&](const httplib::Request &, httplib::Response &res) {
    res.status = 204;
    set_cors(res);
  });

  svr.Post("/login", [&](const httplib::Request &req, httplib::Response &res) {
    std::string user = getJsonValue(req.body, "username");
    std::string pass = getJsonValue(req.body, "password");

    std::cout << "Login attempt: [" << user << "] / [" << pass << "]"
              << std::endl;

    if (checkLogin(user, pass)) {
      std::string role = "member";
      for (const auto &u : users) {
        if (u.username == user) {
          role = u.role;
          break;
        }
      }
      std::cout << "Login successful. Role: " << role << std::endl;
      res.set_content("{\"username\": \"" + escape(user) + "\", \"role\": \"" +
                          escape(role) + "\"}",
                      "application/json");
      res.status = 200;
    } else {
      std::cout << "Login failed" << std::endl;
      res.status = 401;
    }
    set_cors(res);
  });

  loadBooks(); // Load existing books on startup
  std::cout << "Server starting on port 8080..." << std::endl;
  svr.listen("0.0.0.0", 8080);
}
