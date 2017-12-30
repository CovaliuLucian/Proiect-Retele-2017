#ifndef Treeh
#define Treeh
#include "token.h"
class Tree
{
  public:
    Tree* left;
    Tree* right;
    Token token;

    Tree() 
    {
        left = NULL;
        right = NULL;
    }
    Tree(Token t)
    {
        left = NULL;
        right = NULL;
        token = t;
    }
    Tree(Token t, Tree* l, Tree* r)
    {
        token = t;
        left = l;
        right = r;
    }
};
#endif