import React, { useState, useEffect } from 'react'
import './App.css'

const StarRating = ({ rating, onRate, readonly }) => {
  const [hover, setHover] = useState(0);
  return (
    <div className="star-rating">
      {[1, 2, 3, 4, 5].map((star) => (
        <span
          key={star}
          className={`star ${star <= (hover || rating) ? 'filled' : ''} ${readonly ? 'readonly' : ''}`}
          onMouseEnter={() => !readonly && setHover(star)}
          onMouseLeave={() => !readonly && setHover(0)}
          onClick={() => !readonly && onRate(star)}
        >
          ★
        </span>
      ))}
    </div>
  );
};

const App = () => {
  const [books, setBooks] = useState([]);
  const [title, setTitle] = useState('');
  const [author, setAuthor] = useState('');
  const [genre, setGenre] = useState('');
  const [coverUrl, setCoverUrl] = useState('');
  const [isLoggedIn, setIsLoggedIn] = useState(false);
  const [username, setUsername] = useState('');
  const [userRole, setUserRole] = useState('member');
  const [password, setPassword] = useState('');
  const [error, setError] = useState('');
  const [borrowError, setBorrowError] = useState('');
  const [searchTerm, setSearchTerm] = useState('');
  const [activeTab, setActiveTab] = useState('all');
  const [filterGenre, setFilterGenre] = useState('All');

  const API_URL = 'http://localhost:8080';

  useEffect(() => {
    if (isLoggedIn) {
      fetchBooks();
    }
  }, [isLoggedIn]);

  const fetchBooks = async () => {
    try {
      const response = await fetch(`${API_URL}/books`);
      if (response.ok) {
        const data = await response.json();
        setBooks(data);
      }
    } catch (err) {
      console.error("Failed to fetch books", err);
    }
  };

  const handleLogin = async (e) => {
    e.preventDefault();
    try {
      const response = await fetch(`${API_URL}/login`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ username, password })
      });
      if (response.ok) {
        const data = await response.json();
        setIsLoggedIn(true);
        setUsername(data.username);
        setUserRole(data.role);
        setError('');
      } else {
        setError('Invalid credentials');
      }
    } catch (err) {
      setError('Login failed');
    }
  };

  const handleAddBook = async (e) => {
    e.preventDefault();
    try {
      const response = await fetch(`${API_URL}/books`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ title, author, genre, coverUrl })
      });
      if (response.ok) {
        setTitle('');
        setAuthor('');
        setGenre('');
        setCoverUrl('');
        fetchBooks();
      }
    } catch (err) {
      console.error("Failed to add book", err);
    }
  };

  const handleDelete = async (id) => {
    try {
      const response = await fetch(`${API_URL}/books/${id}`, {
        method: 'DELETE'
      });
      if (response.ok) {
        fetchBooks();
      }
    } catch (err) {
      console.error("Failed to delete book", err);
    }
  };

  const handleBorrow = async (id) => {
    setBorrowError('');
    try {
      const response = await fetch(`${API_URL}/books/${id}/borrow`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ username })
      });
      if (response.ok) {
        fetchBooks();
      } else {
        const errText = await response.text();
        setBorrowError("Cannot borrow: You might have reached the 3-book limit.");
      }
    } catch (err) {
      console.error("Failed to borrow book", err);
    }
  };

  const handleReturn = async (id) => {
    try {
      const response = await fetch(`${API_URL}/books/${id}/return`, {
        method: 'POST'
      });
      if (response.ok) {
        fetchBooks();
      }
    } catch (err) {
      console.error("Failed to return book", err);
    }
  };

  const handleRate = async (id, stars) => {
    try {
      const response = await fetch(`${API_URL}/books/${id}/rate`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ stars })
      });
      if (response.ok) {
        fetchBooks();
      }
    } catch (err) {
      console.error("Failed to rate book", err);
    }
  };

  const getDaysRemaining = (dueDate) => {
    if (!dueDate) return null;
    const now = Math.floor(Date.now() / 1000);
    const diff = dueDate - now;
    return Math.ceil(diff / (24 * 60 * 60));
  };

  const genres = ['All', ...new Set(books.map(b => b.genre || 'General'))];

  const displayedBooks = books.filter(book => {
    const matchesSearch = book.title.toLowerCase().includes(searchTerm.toLowerCase()) ||
      book.author.toLowerCase().includes(searchTerm.toLowerCase());
    const matchesGenre = filterGenre === 'All' || (book.genre || 'General') === filterGenre;
    const matchesTab = activeTab === 'all' || (activeTab === 'my' && book.borrowedBy === username);

    return matchesSearch && matchesGenre && matchesTab;
  });

  if (!isLoggedIn) {
    return (
      <div className="login-container">
        <div className="login-card">
          <header style={{ boxShadow: 'none', padding: 0, marginBottom: '2rem', justifyContent: 'center' }}>
            <h1>Library Login</h1>
          </header>
          {error && <p className="error">{error}</p>}
          <form onSubmit={handleLogin}>
            <div className="form-group">
              <label>Username</label>
              <input type="text" value={username} onChange={e => setUsername(e.target.value)} />
            </div>
            <div className="form-group">
              <label>Password</label>
              <input type="password" value={password} onChange={e => setPassword(e.target.value)} />
            </div>
            <button type="submit" className="btn-primary">Login</button>
          </form>
        </div>
      </div>
    );
  }

  return (
    <div className="app-container">
      <header>
        <h1>Library Management System</h1>
        <div className="user-profile">
          <div className="user-info-badge">
            <span className="user-name">Welcome, <strong>{username}</strong></span>
            <span className="user-role-tag">{userRole}</span>
          </div>
          <button className="btn-secondary" onClick={() => setIsLoggedIn(false)}>Logout</button>
        </div>
      </header>

      <main>
        {userRole === 'admin' && (
          <section className="card add-book-section">
            <h2>Add New Book</h2>
            <form onSubmit={handleAddBook}>
              <div className="add-book-grid">
                <input
                  placeholder="Title"
                  value={title}
                  onChange={(e) => setTitle(e.target.value)}
                  required
                />
                <input
                  placeholder="Author"
                  value={author}
                  onChange={(e) => setAuthor(e.target.value)}
                  required
                />
                <input
                  placeholder="Genre (e.g. Fiction, Tech)"
                  value={genre}
                  onChange={(e) => setGenre(e.target.value)}
                />
                <input
                  placeholder="Cover Image URL"
                  value={coverUrl}
                  onChange={(e) => setCoverUrl(e.target.value)}
                />
                <button type="submit" className="btn-success">Add Book</button>
              </div>
            </form>
          </section>
        )}

        <div className="content-filters">
          <div className="tabs">
            <button
              className={`tab-btn ${activeTab === 'all' ? 'active' : ''}`}
              onClick={() => setActiveTab('all')}
            >
              All Books
            </button>
            <button
              className={`tab-btn ${activeTab === 'my' ? 'active' : ''}`}
              onClick={() => setActiveTab('my')}
            >
              My Borrows
            </button>
          </div>

          <div className="genre-chips">
            {genres.map(g => (
              <span
                key={g}
                className={`genre-chip ${filterGenre === g ? 'active' : ''}`}
                onClick={() => setFilterGenre(g)}
              >
                {g}
              </span>
            ))}
          </div>
        </div>

        <section className="card book-list-section">
          <div className="list-header">
            <h2>{activeTab === 'all' ? 'Library Inventory' : 'Personal Collection'}</h2>
            <input
              className="search-input"
              type="text"
              placeholder="Search by title or author..."
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
            />
          </div>

          {borrowError && <div className="error-banner">{borrowError}</div>}

          <ul className="book-list">
            {displayedBooks.length === 0 ? (
              <li className="no-books">No books found in this category.</li>
            ) : (
              displayedBooks.map(book => {
                const days = getDaysRemaining(book.dueDate);
                return (
                  <li key={book.id} className={`book-card-item ${days !== null && days <= 2 ? 'overdue' : ''}`}>
                    <div className="book-cover-container">
                      {book.coverUrl ? (
                        <img src={book.coverUrl} alt={book.title} className="book-cover" />
                      ) : (
                        <div className="book-cover-placeholder">📖</div>
                      )}
                      {!book.isAvailable && (
                        <div className="borrowed-overlay">
                          {book.borrowedBy === username ? 'YOURS' : 'OUT'}
                        </div>
                      )}
                    </div>
                    <div className="book-details">
                      <div className="book-header">
                        <div className="book-id-chip">#{book.id}</div>
                        <span className="book-genre-chip">{book.genre || 'General'}</span>
                      </div>
                      <div className="book-title-row">
                        <h3 className="book-title">{book.title}</h3>
                        <p className="book-author">by {book.author}</p>
                      </div>

                      <div className="book-rating-row">
                        <StarRating
                          rating={Math.round(book.rating)}
                          onRate={(s) => handleRate(book.id, s)}
                          readonly={false}
                        />
                        <span className="rating-text">
                          {book.rating > 0 ? `${book.rating.toFixed(1)} (${book.ratingCount})` : 'No ratings'}
                        </span>
                      </div>

                      <div className="book-status-row">
                        {book.isAvailable ? (
                          <span className="status-tag available">Available</span>
                        ) : (
                          <div className="due-info">
                            <span className={`status-tag borrowed ${days <= 2 ? 'urgent' : ''}`}>
                              {days <= 0 ? 'OVERDUE' : `Due in ${days} days`}
                            </span>
                            <span className="borrower-name">@{book.borrowedBy}</span>
                          </div>
                        )}
                      </div>

                      <div className="book-card-actions">
                        {book.isAvailable ? (
                          <button className="btn-primary-sm" onClick={() => handleBorrow(book.id)}>Borrow</button>
                        ) : (
                          book.borrowedBy === username && (
                            <button className="btn-success-sm" onClick={() => handleReturn(book.id)}>Return</button>
                          )
                        )}
                        {userRole === 'admin' && (
                          <button className="btn-danger-sm" onClick={() => handleDelete(book.id)}>Delete</button>
                        )}
                      </div>
                    </div>
                  </li>
                );
              })
            )}
          </ul>
        </section>
      </main>
    </div>
  );
}

export default App