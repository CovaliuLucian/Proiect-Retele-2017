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
        SYA(ParsePrep(input));
        // tree
        // exec
    }

  public: // to private
    string static Trim(string input)
    {
        int i = 0;
        while (input[i] == ' ')
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
                last = i + 1;
            }
            else
            {
                op = input.substr(i, 1);
                if (op == "|" || op == "&" || op == ";" || op == "(" || op == ")")
                {
                    toReturn.push(Operand(Trim(input.substr(last, i - last))));
                    toReturn.push(Operator(op));
                    last = i + 1;
                }
            }

            i++;
        }
        toReturn.push(Operand(Trim(input.substr(last, i - last))));

        return toReturn;
    }

    // while there are tokens to be read:
    // 	read a token.
    // 	if the token is a number, then push it to the output queue.
    // 	if the token is an operator, then:
    // 		while (there is an operator at the top of the operator stack with
    // 			greater precedence) or (the operator at the top of the operator stack has
    //                         equal precedence and
    //                         the operator is left associative) and
    //                       (the operator at the top of the stack is not a left bracket):
    // 				pop operators from the operator stack, onto the output queue.
    // 		push the read operator onto the operator stack.
    // 	if the token is a left bracket (i.e. "("), then:
    // 		push it onto the operator stack.
    // 	if the token is a right bracket (i.e. ")"), then:
    // 		while the operator at the top of the operator stack is not a left bracket:
    // 			pop operators from the operator stack onto the output queue.
    // 		pop the left bracket from the stack.
    // 		/* if the stack runs out without finding a left bracket, then there are
    // 		mismatched parentheses. */
    // if there are no more tokens to read:
    // 	while there are still operator tokens on the stack:
    // 		/* if the operator token on the top of the stack is a bracket, then
    // 		there are mismatched parentheses. */
    // 		pop the operator onto the output queue.
    // exit.
    queue<Token> static SYA(queue<Token> input)
    {
        queue<Token> toReturn = queue<Token>(), operatorStack = queue<Token>();

        while (!input.empty())
        {
            Token current = input.front();
            if (current.getType() == "Operand")
                toReturn.push(current);

            if (current.getType() == "Operator" && current.command != "(" && current.command != ")")
            {
                while (!operatorStack.empty() && operatorStack.front().command != "(")
                {
                    toReturn.push(operatorStack.front());
                    operatorStack.pop();
                }
                operatorStack.push(current);
            }

            if (current.command == "(")
                operatorStack.push(current);

            if (current.command == ")")
            {
                while (operatorStack.front().command != "(" && !operatorStack.empty())
                {
                    toReturn.push(operatorStack.front());
                    operatorStack.pop();
                }
                if (operatorStack.empty())
                {
                    throw "Mismatched parantheses";
                }
                else
                {
                    //toReturn.push(operatorStack.front());
                    operatorStack.pop();
                }
            }
            input.pop();
        }
        while(!operatorStack.empty())
        {
            if(operatorStack.front().command == "(" || operatorStack.front().command == ")")
                throw "Mismatched parantheses";
            toReturn.push(operatorStack.front());
            operatorStack.pop();
        }
        return toReturn;
    }
};