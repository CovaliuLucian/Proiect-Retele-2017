#ifndef Operatorh
#define Operatorh

#include "token.h"

class Operator : public Token
{
  public:
    Operator(string com)
    {
        command = com;
    }
};

#endif