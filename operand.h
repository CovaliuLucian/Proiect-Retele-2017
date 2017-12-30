#ifndef Operandh
#define Operandh

#include "token.h"

class Operand : public Token
{
  public:
    Operand(string com, int pr = 0)
    {
        command = com;
        priority = pr;
        type = "Operand";
    }
    string getType()
    {
        return type;
    }
};

#endif