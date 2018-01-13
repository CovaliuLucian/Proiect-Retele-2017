//
// Created by luci on 13.01.2018.
//

#include "DataBase.h"
#include <string>
#include <iostream>
#include "Crypto.h"

using namespace std;

bool DataBase::Prepare(const string &name) {
    if (sqlite3_open(name.c_str(), &db) != SQLITE_OK) {
        sqlite3_close(db);
        cerr << "Error opening database: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    return true;
}

bool DataBase::Execute(const std::string &query) {

    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, query.c_str(), static_cast<int>(query.size()), &stmt, nullptr);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Error inserting: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    return true;
}

DataBase::DataBase(const std::string &name) {
    Prepare(name);
}

DataBase::DataBase() {

}

DataBase::~DataBase() {
    sqlite3_close(db);
}

bool DataBase::AddAccount(const std::string username, const std::string password, bool admin) {
    return Execute("insert into Users values (\"" +
                   username + "\",\"" +
                   Crypto::sha256(password) + "\"," +
                   (admin ? "1" : "0") + ");");
}

bool DataBase::CheckAccount(const std::string username) {
    sqlite3_stmt *stmt;

    string query = "select count(*) from users where username = '" + username + "';";

    sqlite3_prepare_v2(db, query.c_str(), static_cast<int>(query.size()), &stmt, nullptr);

    if(sqlite3_step(stmt) == SQLITE_ROW)
    {
        int count = sqlite3_column_int(stmt,0);
        sqlite3_finalize(stmt);
        return count == 1;
    }
    cerr << "Error in reading from database:" << sqlite3_errmsg(db);
}

bool DataBase::CheckAccount(const std::string username, const std::string password) {
    sqlite3_stmt *stmt;

    string query = "select password from users where username = '" + username + "';";

    sqlite3_prepare_v2(db, query.c_str(), static_cast<int>(query.size()), &stmt, nullptr);

    int rc = sqlite3_step(stmt);

    if(rc == SQLITE_ROW)
    {
        string storedPassword = string((const char *)sqlite3_column_text(stmt,0));
        sqlite3_finalize(stmt);
        return Crypto::sha256(password) == storedPassword || password == storedPassword;
    }

    if(rc == SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        return false;
    }

    cerr << "Error in reading from database:" << sqlite3_errmsg(db);
}


