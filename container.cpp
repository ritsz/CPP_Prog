#include <vector>
#include <queue>
#include <string>
#include <unordered_map>
#include <iostream>

using namespace std;

int main()
{
    unordered_map<string, string> cache;
    cache["one"] = "hello";
    cache["two"] = "Hello";

    for(auto it = cache.begin(); it != cache.end(); it++)
        cout << it->first << " " << it->second << endl;

}


