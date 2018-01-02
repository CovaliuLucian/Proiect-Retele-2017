#include <string>
#include <stack>
#include <queue>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "token.h"
#include "operator.h"
#include "operand.h"
#include "tree.h"

using namespace std;

class Parser {
public:
    void static Parse(string input, int sd) {
        Execute(GenerateTree(SYA(ParsePrep(input))),sd);
    }

public: // to private
    string static TrimLeft(string input) {
        unsigned long i = 0;
        while (input[i] == ' ')
            i++;
        return input.substr(i);
    }
    string static TrimRight(string input) {
        unsigned long i = input.size();
        while (input[i] == ' ')
            i--;
        return input.substr(0,input.size()-i);
    }
    string static Trim(string input)
    {
        return TrimLeft(TrimRight(input));
    }

    queue<Token> static ParsePrep(const char *input) {
        return ParsePrep(string(input));
    }

    queue<Token> static ParsePrep(string input) {
        queue<Token> toReturn = queue<Token>();

        unsigned long last = 0, i = 0;
        while (i != input.size()) {
            string op = input.substr(i, 2);

            if (op == "&&" || op == "||" || op == "2>") {
                Operand newOp = *new Operand(Trim(input.substr(last, i - last)), op == "2>" ? 1 : 0);
                if(newOp.command!=" " && !newOp.command.empty())
                    toReturn.push(newOp);
                toReturn.push(Operator(op));
                i++;
                last = i + 1;
            } else {
                op = input.substr(i, 1);
                if (op == "|" || op == ";" || op == "(" || op == ")" || op == ">" || op == "<") {
                    Operand newOp = *new Operand(Trim(input.substr(last, i - last)), op == ">" || op == "<" ? 1 : 0);
                    if(newOp.command!=" " && !newOp.command.empty())
                        toReturn.push(newOp);
                    toReturn.push(Operator(op));
                    last = i + 1;
                }
            }

            i++;
        }
        Operand newOp = *new Operand(Trim(input.substr(last, i - last)));
        if(newOp.command!=" " && !newOp.command.empty())
            toReturn.push(newOp);

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
    queue<Token> static SYA(queue<Token> input) {
        queue<Token> toReturn = queue<Token>(), operatorStack = queue<Token>();

        while (!input.empty()) {
            Token current = input.front();
            if (current.getType() == "Operand")
                toReturn.push(current);

            if (current.getType() == "Operator" && current.command != "(" && current.command != ")") {
                while (!operatorStack.empty() && operatorStack.front().priority >= current.priority &&
                       operatorStack.front().command != "(") {
                    toReturn.push(operatorStack.front());
                    operatorStack.pop();
                }
                operatorStack.push(current);
            }

            if (current.command == "(")
                operatorStack.push(current);

            if (current.command == ")") {
                while (operatorStack.front().command != "(" && !operatorStack.empty()) {
                    toReturn.push(operatorStack.front());
                    operatorStack.pop();
                }
                if (operatorStack.empty()) {
                    throw "Mismatched parantheses";
                } else {
                    //toReturn.push(operatorStack.front());
                    operatorStack.pop();
                }
            }
            input.pop();
        }
        while (!operatorStack.empty()) {
            if (operatorStack.front().command == "(" || operatorStack.front().command == ")")
                throw "Mismatched parantheses";
            toReturn.push(operatorStack.front());
            operatorStack.pop();
        }
        return toReturn;
    }

    //     for each token in the postfix expression:
    //   if token is an operator:
    //     operand_2 ← pop from the stack
    //     operand_1 ← pop from the stack
    //     result ← evaluate token with operand_1 and operand_2
    //     push result back onto the stack
    //   else if token is an operand:
    //     push token onto the stack
    // result ← pop from the stack

    Tree static * GenerateTree(queue<Token> input) {
        stack<Tree*> Stack = stack<Tree*>();
        Tree *last;
        while (!input.empty()) {
            Token current = input.front();
            if (current.getType() == "Operator") {
                if (Stack.size() < 2)
                    throw "Invalid expression around " + current.command;
                Tree *op2 = Stack.top();
                Stack.pop();
                Tree *op1 = Stack.top();
                Stack.pop();
                Tree *newOp = new Tree(current, op1, op2);
                Stack.push(newOp);

                last = newOp;
            }
            if (current.getType() == "Operand") {
                Tree *newOp = new Tree(current);
                Stack.push(newOp);

                last = newOp;
            }
            input.pop();
        }

        return last;
    }

    bool static Execute(Tree* input, int sd) {

        if (input->token.getType() == "Operand" && input->left == nullptr && input->right == nullptr)
        {

            int sockets[2];
            if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0)
            {
                throw "Socketpair error";
            }

            int child = fork();

            if (child < 0)
            {
                throw "Fork error";
            }
            if (child) // parent
            {
                bool status;
                close(sockets[0]);

                int stat_loc, exitStatus;

                waitpid(child, &stat_loc, 0);
                status = stat_loc == 0 ? true : WEXITSTATUS(stat_loc) == 0;

                char output[1001];
                long len = read(sockets[1], output, 1000);

                if (len < 0)
                    throw "?";

                output[len] = 0;

                close(sockets[1]);

                Response res;
                res.setCode(100);
                res.setMessage(string(output));
                res.send(sd);

                return status;

            } else
            {
                close(sockets[1]);

                dup2(sockets[0], fileno(stdout));
                dup2(sockets[0], fileno(stderr));

                /*const char* args[30];
                string inputString = input->token.command;
                unsigned long last=0,i=0,nargs=0;
                while (!inputString.empty() && i<inputString.size())
                {
                    if(inputString[i]==' ')
                    {
                        args[nargs++] = inputString.substr(last,i-last).c_str();
                        last = i;
                    }
                    i++;
                }
                args[nargs]= nullptr;
                if(execvp(args[0], (char*const*)args) < 0)*/
                if(execlp(input->token.command.c_str(), input->token.command.c_str(), (char*) nullptr) < 0)
                    perror(nullptr);
                fflush(stderr);

                close(sockets[0]);
                exit(1);
            }
        } else
            if(input->token.getType() == "Operator" && input->left != nullptr && input->right != nullptr)
            {
                bool status = true;
                if(input->token.command == "&&")
                {
                    bool leftResult = Execute(input->left, sd),rightResult;
                    if(leftResult)
                    {
                        rightResult = Execute(input->right, sd);
                        status = leftResult && rightResult;
                    }
                    else
                        status = false;
                    return status;
                }
                if(input->token.command == "||")
                {
                    bool leftResult = Execute(input->left, sd),rightResult;
                    if(!leftResult)
                    {
                        rightResult = Execute(input->right, sd);
                        status = leftResult || rightResult;
                    }
                    return status;
                }
                if(input->token.command == ";")
                {
                    bool leftResult = Execute(input->left, sd),rightResult = Execute(input->right, sd);
                    status = leftResult || rightResult;
                    return status;
                }

                // should't get here
                throw "Invalid expression around " + input->token.command;
            } else
            if(input->token.command==";" && input->left != nullptr && input->right == nullptr)
            {
                return Execute(input->left, sd);
            }

        throw "Invalid expression around " + input->token.command;
    }
};