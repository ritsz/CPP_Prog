#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>

using namespace std;

class MyStr {
    string str;
public:
    MyStr(string _str) : str(_str) {}
    void print() { cout << str << endl; }
};

struct Record {
    MyStr *name;
    MyStr *addr;
    int rollno;

    void AsString() {
        name->print();
        addr->print();
        cout << rollno << endl;
    }
};

class Database {
public:
    void Show() 
    {
        Record *elem = (Record *)entry;
        cout << "Name ";
        elem->name->print();
        cout << "Addr ";
        elem->addr->print();
        cout << "Roll no " << elem->rollno << endl;
    }
    void *entry;
};

int main()
{
    MyStr *name = new MyStr("RITSZ");
    MyStr *addr = new MyStr("BLR");
    int rollno = 23;

    Record element;
    element.rollno = rollno;
    element.addr = addr;
    element.name = name;

    element.AsString();

    Database *db = new Database();
    db->entry = malloc(100);;
    memcpy(db->entry, &element, sizeof element);
    db->Show();

}
