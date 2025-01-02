#include "book.h"

#include <climits>
#include <iomanip>
#include "cmd.h"
#include "log.h"

void Book::print() {
  std::cout << std::string(ISBN) << "\t" << std::string(bookname) << "\t" << std::string(author) << "\t" << std::string(keyword) << "\t";
  std::cout << std::fixed << std::setprecision(2) << price << "\t" << quantity << std::endl;
}

Bookstore::Bookstore() : bf("bookstore.db"), db_ISBN(bf), db_bookname(bf), db_author(bf), db_keyword(bf), db_bookid(bf) {}

Book Bookstore::askByISBN(ISBN_t ISBN) {
  BookPtr pos = db_ISBN.get(ISBN);
  Book ret{};
  bf.getT(pos, ret);
  return ret;
}

Book Bookstore::askByBookid(bookid_t bookid) {
  BookPtr pos = db_bookid.get(bookid);
  Book ret{};
  bf.getT(pos, ret);
  return ret;
}

bookid_t Bookstore::select(ISBN_t ISBN) {
  if (db_ISBN.exist(ISBN))
    return askByISBN(ISBN).bookid;
  // errf("bs::select creating new book\n");
  Book newbook = default_book();
  newbook.ISBN = ISBN;
  pos_t pos = bf.allocT(newbook);
  newbook.bookid = pos;
  bf.putT(pos, newbook);
  db_ISBN.insert(ISBN, pos);
  db_bookid.insert(newbook.bookid, pos);
  return newbook.bookid;
}

void Bookstore::eraseBook(BookPtr bp) {
  Book b{};
  bf.getT(bp, b);
  // printf("erase "); b.print();
  db_ISBN.erase(b.ISBN);
  if (not cstr_null(b.bookname))
    db_bookname.erase(b.bookname, b.bookid);
  if (not cstr_null(b.author))
    db_author.erase(b.author, b.bookid);
  if (not cstr_null(b.keyword))
    for (auto &k : split_keyword(b.keyword.data()))
      db_keyword.erase(string2keyword(k), b.bookid);
  db_bookid.erase(b.bookid);
}

void Bookstore::insertBook(BookPtr bp) {
  Book b{};
  bf.getT(bp, b);
  db_ISBN.insert(b.ISBN, bp);
  if (not cstr_null(b.bookname))
    db_bookname.insert(b.bookname, bp, b.bookid);
  if (not cstr_null(b.author))
    db_author.insert(b.author, bp, b.bookid);
  if (not cstr_null(b.keyword))
    for (auto &k : split_keyword(b.keyword.data()))
      db_keyword.insert(string2keyword(k), bp, b.bookid);
  db_bookid.insert(b.bookid, bp);
}

void Bookstore::modify(bookid_t bookid, const std::map<std::string, std::string> &map) {
  pos_t pos = db_bookid.get(bookid);
  Book b{};
  bf.getT(pos, b);
  Book newb = b;

  if (map.count("ISBN")) {
    newb.ISBN = string2ISBN(map.at("ISBN"));
    if (!(newb.ISBN != b.ISBN)) throw Error("");;
    if (!(! cstr_null(newb.ISBN))) throw Error("");;
    if (!(valid_ISBN(newb.ISBN))) throw Error("");;
    if (!(! db_ISBN.exist(newb.ISBN))) throw Error("");;
  }
  if (map.count("name")) {
    newb.bookname = string2bookname(map.at("name"));
    if (!(valid_bookname(newb.bookname))) throw Error("");;
    if (!(! cstr_null(newb.bookname))) throw Error("");;
  }
  if (map.count("author")) {
    newb.author = string2author(map.at("author"));
    if (!(valid_author(newb.author))) throw Error("");;
    if (!(! cstr_null(newb.author))) throw Error("");;
  }
  if (map.count("keyword")) {
    split_keyword(map.at("keyword"));
    newb.keyword = string2keyword(map.at("keyword"));
    if (!(valid_keyword(newb.keyword))) throw Error("");;
    if (!(! cstr_null(newb.keyword))) throw Error("");;
  }
  if (map.count("price")) {
    if (!(valid_price(map.at("price")))) throw Error("");;
    newb.price = (price_t)string2double(map.at("price"));
  }

  eraseBook(pos);
  bf.putT(pos, newb);
  insertBook(pos);
}

void Bookstore::import_book(bookid_t bookid, quantity_t quantity, totalcost_t totalcost) {
  if (!(quantity > 0)) throw Error("");;
  if (!(totalcost > 0)) throw Error("");;
  pos_t pos = db_bookid.get(bookid);
  Book b{}; bf.getT(pos, b);
  assert(b.quantity <= INT_MAX - quantity);
  b.quantity += quantity;
  b.totalcost += totalcost;
  bf.putT(pos, b);
  Finance::getInstance().outcome(totalcost);
}

void Bookstore::showByISBN(ISBN_t ISBN) {
  if (!(valid_ISBN(ISBN))) throw Error("");;
  try {
    askByISBN(ISBN).print();
  } catch(const Error &) {
    std::cout<<"\n";
  }
}

void Bookstore::showByName(bookname_t bookname) {
  if (!(valid_bookname(bookname))) throw Error("");;
  try {
    auto bookptrs = db_bookname.get(bookname);
    std::vector<Book> books;
    std::transform(bookptrs.begin(), bookptrs.end(), std::back_inserter(books), [&](pos_t p) {
        Book ret{}; bf.getT(p, ret);
        return ret;
        });
    if (books.empty())
      std::cout<<"\n";
    else {
      std::sort(books.begin(), books.end(), [&](auto &u, auto &v) { return u.ISBN < v.ISBN; });
      for (auto &b : books)
        b.print();
    }
  } catch(const Error &) {
    std::cout<<"\n";
  }
}

void Bookstore::showByAuthor(author_t author) {
  if (!(valid_author(author))) throw Error("");;
  try {
    auto bookptrs = db_author.get(author);
    std::vector<Book> books;
    std::transform(bookptrs.begin(), bookptrs.end(), std::back_inserter(books), [&](pos_t p) {
        Book ret{}; bf.getT(p, ret);
        return ret;
        });
    if (books.empty())
      std::cout<<"\n";
    else {
      std::sort(books.begin(), books.end(), [&](auto &u, auto &v) { return u.ISBN < v.ISBN; });
      for (auto &b : books)
        b.print();
    }
  } catch(const Error &) {
    std::cout<<"\n";
  }
}

void Bookstore::showByKeyword(keyword_t keyword) {
  if (!(valid_keyword(keyword))) throw Error("");;
  try {
    auto bookptrs = db_keyword.get(keyword);
    std::vector<Book> books;
    std::transform(bookptrs.begin(), bookptrs.end(), std::back_inserter(books), [&](pos_t p) {
        Book ret{}; bf.getT(p, ret);
        return ret;
        });
    if (books.empty())
      std::cout<<"\n";
    else {
      std::sort(books.begin(), books.end(), [&](auto &u, auto &v) { return u.ISBN < v.ISBN; });
      for (auto &b : books)
        b.print();
    }
  } catch(const Error &) {
    std::cout<<"\n";
  }
}

void Bookstore::showAll() {
  try {
    auto bookptrs = db_bookid.getAll();
    std::vector<Book> books;
    std::transform(bookptrs.begin(), bookptrs.end(), std::back_inserter(books), [&](pos_t p) {
        Book ret{}; bf.getT(p, ret);
        return ret;
        });
    if (books.empty())
      std::cout<<"\n";
    else {
      std::sort(books.begin(), books.end(), [&](auto &u, auto &v) { return u.ISBN < v.ISBN; });
      for (auto &b : books)
        b.print();
    }
  } catch(const Error &) {
    std::cout<<"\n";
  }
}

void Bookstore::buy(ISBN_t ISBN, int quantity) {
  if (!(quantity > 0)) throw Error("");;
  pos_t pos = db_ISBN.get(ISBN);
  Book b{}; bf.getT(pos, b);
  if (!(quantity <= b.quantity)) throw Error("");;
  eraseBook(pos);
  b.quantity -= quantity;
  bf.putT(pos, b);
  insertBook(pos);
  std::cout<<std::fixed<<std::setprecision(2)<<quantity * b.price<<std::endl;
  Finance::getInstance().income(quantity * b.price);
}