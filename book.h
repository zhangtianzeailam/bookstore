//
// Created by lenovo on 2025/1/1.
//

#ifndef BOOK_H
#define BOOK_H
#include "cstr.h"
#include "database.h"
#include "databaseplus.h"
#include <map>

#include <cstdint>

static constexpr bookid_t nullid = -1;

struct Book {
    ISBN_t ISBN;
    bookname_t bookname;
    author_t author;
    keyword_t keyword;
    quantity_t quantity;
    price_t price;
    totalcost_t totalcost;
    bookid_t bookid;
    void print();
};

static Book default_book() { return Book{ "", "", "", "", 0, 0, 0, nullpos }; }

typedef pos_t BookPtr;

class Bookstore {
private:
    Memoryriver bf;
    // DFDB(ISBN, 0);      // db_ISBN
    Database<ISBN_t, BookPtr> db_ISBN;
    DBMore<bookname_t, bookid_t, BookPtr, std::less<bookname_t>{}, std::equal_to<bookname_t>{}, std::equal_to<bookid_t>{}, 1> db_bookname;  // db_bookname
    DBMore<author_t, bookid_t, BookPtr, std::less<author_t>{}, std::equal_to<author_t>{}, std::equal_to<bookid_t>{}, 2> db_author;    // db_author
    DBMore<keyword_t, bookid_t, BookPtr, std::less<keyword_t>{}, std::equal_to<keyword_t>{}, std::equal_to<bookid_t>{}, 3> db_keyword;   // db_keyword
    Database<bookid_t, BookPtr, std::less<bookid_t>{}, std::equal_to<bookid_t>{}, 4> db_bookid;

    Bookstore();
    void eraseBook(BookPtr bp);
    void insertBook(BookPtr bp);

public:
    static Bookstore &getInstance() {
        static Bookstore me;
        return me;
    }
    Book askByISBN(ISBN_t ISBN);
    Book askByBookid(bookid_t bookid);
    bookid_t select(ISBN_t ISBN);
    void modify(bookid_t bookid, const std::map<std::string, std::string> &map);
    void import_book(bookid_t bookid, quantity_t quantity, totalcost_t totalcost);
    void showByISBN(ISBN_t ISBN);
    void showByName(bookname_t bookname);
    void showByAuthor(author_t author);
    void showByKeyword(keyword_t keyword);
    void showAll();
    void buy(ISBN_t ISBN, int quantity);
};

static Bookstore &bkst = Bookstore::getInstance();
#endif //BOOK_H
