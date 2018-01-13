//
// Created by lucian on 13.01.2018.
//

#include <sqlite3.h>

#include <iostream>
#include "DataBase.h"
#include "Crypto.h"


using namespace std;



int main()
{
    DataBase db = *new DataBase("SSH");

    db.Execute("drop table Users");
    db.Execute("create table Users (username varchar2(50) unique,password char(32),admin number)");
    //db.Execute("insert into Users values (\"Admin\", \"" + Crypto::sha256("admin") +"\" , 1)");
    db.AddAccount("Admin","admin",true);
    db.AddAccount("Luci","test",false);

    cout << db.CheckAccount("Admin") << " should be true\n";
    cout << db.CheckAccount("dsadsadsa") << " should be false\n";
    cout << db.CheckAccount("Admin","admin") << " should be true\n";
    cout << db.CheckAccount("Admin","dsadas") << " should be false\n";
    cout << db.CheckAccount("Addsadasmin","dsadas") << " should be false\n";

    return 0;
}

