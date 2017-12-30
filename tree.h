#include "token.h"
class Tree
{
  public:
    Tree *left;
    Tree *right;
    Token token;

    Tree() {}
    Tree(Token t)
    {
        token = t;
    }
    Tree(Token t, Tree l, Tree r)
    {
        token = t;
        left = &l;
        right = &r;
    }
};