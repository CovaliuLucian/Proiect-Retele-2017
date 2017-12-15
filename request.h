#include<string>

using namespace std;

class Request
{
private:
    string request;
public:
    Request(string r)
    {
        request = r;
    }
    Request(char* r)
    {
        request = string(r);
    }
    Request()
    {}
    string getRequest()
    {
        return request;
    }
};