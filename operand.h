#ifndef Operandh
#define Operandh

#include "token.h"

class Operand : public Token
{
  public:
    Operand(string com)
    {
        command = com;
        type = "Operand";
    }
    string getType()
    {
        return type;
    }
};

#endif