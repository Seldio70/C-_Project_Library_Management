#include "library.h"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Define the global library vector
vector<Book> library;
vector<User> users;
const string BOOK_FILE = "books.txt";
const string USER_FILE = "users.txt";

// Adds a new book to the library
void addBook(int id, string title, string author, string genre,
             string coverUrl) {
  Book b;
  b.id = id;
  b.title = title;
  b.author = author;
  b.isAvailable = true;
  b.borrowedBy = "";
  b.genre = genre;
  b.coverUrl = coverUrl;
  b.dueDate = 0;
  b.rating = 0.0f;
  b.ratingCount = 0;
  library.push_back(b);
}

// Returns a copy of the list of books
vector<Book> getBooks() { return library; }

// Deletes a book by ID
bool deleteBook(int id) {
  for (size_t i = 0; i < library.size(); i++) {
    if (library[i].id == id) {
      library.erase(library.begin() + i);
      return true;
    }
  }
  return false;
}

// Borrows a book
bool borrowBook(int id, string username) {
  // Borrowing limit: 3 books
  int currentBorrows = 0;
  for (const auto &b : library) {
    if (!b.isAvailable && b.borrowedBy == username) {
      currentBorrows++;
    }
  }

  if (currentBorrows >= 3) {
    return false; // Limit reached
  }

  for (auto &b : library) {
    if (b.id == id && b.isAvailable) {
      b.isAvailable = false;
      b.borrowedBy = username;
      // Set due date to 14 days from now
      b.dueDate = (long long)time(nullptr) + (14 * 24 * 60 * 60);
      return true;
    }
  }
  return false;
}

bool returnBook(int id) {
  for (auto &b : library) {
    if (b.id == id && !b.isAvailable) {
      b.isAvailable = true;
      b.borrowedBy = "";
      b.dueDate = 0;
      return true;
    }
  }
  return false;
}

void addRating(int id, int stars) {
  for (auto &b : library) {
    if (b.id == id) {
      float totalRating = b.rating * b.ratingCount;
      b.ratingCount++;
      b.rating = (totalRating + stars) / b.ratingCount;
      return;
    }
  }
}

// Saves books to file
void saveBooks() {
  ofstream file(BOOK_FILE);
  if (file.is_open()) {
    for (const auto &b : library) {
      // Format:
      // ID|Title|Author|isAvailable|borrowedBy|Genre|CoverURL|DueDate|Rating|RatingCount
      file << b.id << "|" << b.title << "|" << b.author << "|" << b.isAvailable
           << "|" << (b.borrowedBy.empty() ? "NONE" : b.borrowedBy) << "|"
           << (b.genre.empty() ? "General" : b.genre) << "|"
           << (b.coverUrl.empty() ? "NONE" : b.coverUrl) << "|" << b.dueDate
           << "|" << b.rating << "|" << b.ratingCount << endl;
    }
    file.close();
  }
}

// Loads books from file
void loadBooks() {
  library.clear();
  ifstream file(BOOK_FILE);
  if (file.is_open()) {
    string line;
    while (getline(file, line)) {
      if (line.empty())
        continue;

      vector<string> tokens;
      string token;
      stringstream ss(line);
      while (getline(ss, token, '|')) {
        tokens.push_back(token);
      }

      if (tokens.size() >= 10) { // Updated to reflect new fields
        Book b;
        b.id = stoi(tokens[0]);
        b.title = tokens[1];
        b.author = tokens[2];
        b.isAvailable = (tokens[3] == "1");
        b.borrowedBy = (tokens[4] == "NONE" ? "" : tokens[4]);
        b.genre = (tokens.size() > 5 ? tokens[5] : "General");
        b.coverUrl =
            (tokens.size() > 6 && tokens[6] != "NONE" ? tokens[6] : "");
        b.dueDate = (tokens.size() > 7 ? stoll(tokens[7]) : 0);
        b.rating = (tokens.size() > 8 ? stof(tokens[8]) : 0.0f);
        b.ratingCount = (tokens.size() > 9 ? stoi(tokens[9]) : 0);
        library.push_back(b);
      }
    }
    file.close();
  }
}

// --- User Management Implementation ---

// Loads users from file into memory
void loadUsers() {
  users.clear();
  ifstream file(USER_FILE);
  if (file.is_open()) {
    string u, p, r;
    while (file >> u >> p >> r) {
      users.push_back({u, p, r});
    }
    file.close();
  }

  // If no users loaded, add default admin
  if (users.empty()) {
    users.push_back({"seldio", "1234", "admin"});
  }
}

// Verifies a user's credentials against the loaded user list
bool checkLogin(string username, string password) {
  loadUsers();
  for (const auto &u : users) {
    if (u.username == username && u.password == password) {
      return true;
    }
  }
  return false;
}
