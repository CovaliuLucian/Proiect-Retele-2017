#ifndef Tokenh
#define Tokenh

#include <string>

using namespace std;

class Token
{
  public:
    string command;
    Token(string com)
    {
        command = com;
    }
    Token()
    {}
};

#endif