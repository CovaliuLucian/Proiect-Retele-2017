#ifndef Operatorh
#define Operatorh

#include "token.h"

class Operator : public Token
{
  public:
    Operator(string com)
    {
        command = com;
        type = "Operator";
    }
    string getType()
    {
        return type;
    }
};

#endif