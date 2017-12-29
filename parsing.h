#include <string>
#include <queue>

#include "token.h"
#include "operator.h"
#include "operand.h"

using namespace std;

class Parser
{
  public:
    void static Parse(string input)
    {
        ParsePrep(input);
        // SYA
        // tree
        // exec
    }

  public: // to private
    string static Trim (string input)
    {
        int i=0;
        while(input[i] == ' ')
            i++;
        return input.substr(i);
    }
    queue<Token> static ParsePrep(const char *input)
    {
        return ParsePrep(string(input));
    }

    queue<Token> static ParsePrep(string input)
    {
        queue<Token> toReturn = queue<Token>();

        int last = 0, i = 0;
        while (i != input.size())
        {
            string op = input.substr(i, 2);

            if (op == "&&" || op == "||")
            {
                toReturn.push(Operand(Trim(input.substr(last, i - last))));
                toReturn.push(Operator(op));
                i++;
                last = i+1;
            }
            else
            {
                op = input.substr(i, 1);
                if (op == "|" || op == "&" || op == ";")
                {
                    toReturn.push(Operand(Trim(input.substr(last, i - last))));
                    toReturn.push(Operator(op));
                    last = i+1;
                }
            }

            i++;
        }
        toReturn.push(Operand(Trim(input.substr(last, i - last))));


        return toReturn;
    }

    queue<string> static SYA(queue<Token> input)
    {
        //
    }
};