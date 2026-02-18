#ifndef LIBRARY_H
#define LIBRARY_H

#include <string>
#include <vector>

struct Book {
  int id;
  std::string title;
  std::string author;
  bool isAvailable;       // true if the book can be borrowed
  std::string borrowedBy; // username of the person who borrowed it
  std::string genre;      // e.g., Fiction, Tech
  std::string coverUrl;   // URL to book cover image
  long long dueDate;      // Unix timestamp
  float rating;
  int ratingCount;
};

struct User {
  std::string username;
  std::string password;
  std::string role; // admin or member
};

// Book Management
extern std::vector<Book> library;
void addBook(int id, std::string title, std::string author, std::string genre,
             std::string coverUrl);

// User Management
extern std::vector<User> users;
void addRating(int id, int stars);
std::vector<Book> getBooks();
bool deleteBook(int id);
bool borrowBook(int id, std::string username);
bool returnBook(int id);
void saveBooks();
void loadBooks();

// User Management
void loadUsers();
bool checkLogin(std::string username, std::string password);

#endif
