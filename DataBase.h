//
// Created by luci on 13.01.2018.
//

#ifndef PROIECT_RETELE_DATABASE_H
#define PROIECT_RETELE_DATABASE_H

#include <sqlite3.h>
#include <string>


class DataBase {
private:
    sqlite3 *db;


public:
    DataBase();
    DataBase(const std::string &name);
    bool Prepare(const std::string &name);
    bool Execute(const std::string & query);
    bool AddAccount(std::string username, std::string password, bool admin);
    bool CheckAccount(std::string username);
    bool CheckAccount(std::string username, std::string password);
    ~DataBase();
};


#endif //PROIECT_RETELE_DATABASE_H
