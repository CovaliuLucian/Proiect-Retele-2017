#ifndef Operatorh
#define Operatorh

#include "token.h"

class Operator : public Token
{
  public:
    Operator(string com, int pr = 0)
    {
        command = com;
        priority = pr;
        type = "Operator";
    }
    string getType()
    {
        return type;
    }
};

#endif