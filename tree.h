#ifndef Treeh
#define Treeh

#include <utility>

#include "token.h"
class Tree
{
  public:
    Tree* left;
    Tree* right;
    Token token;

    Tree() 
    {
        left = nullptr;
        right = nullptr;
    }
    explicit Tree(Token t)
    {
        left = nullptr;
        right = nullptr;
        token = std::move(t);
    }

    Tree(Token t, Tree* l, Tree* r)
    {
        token = std::move(t);
        left = l;
        right = r;
    }
};
#endif