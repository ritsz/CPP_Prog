#include <iostream>
#include <utility>

using namespace std;

void nextCall(int& var) 
{
	var++;
	cout << "VAR " << var << endl;
}

void nextCall(const int& var)
{
	cout << "CONST VAR " << var << endl; 
}

void nextCall(int&& var)
{
	cout << "MOVE VAR " << var << endl;
}

/* This function, even though it looks like iit always accepts lvalue references, could also accpet
 * const references. Depending of the type T, different nextCall could be called.
 */
template <class T>
void forwardFunc(T & var)
{
	nextCall(var);
}

/* Rvalue references wuth template types can accept both
 * lvalue refernces as well as rvalue references (and even const references)
 */
template <typename T>
void func(T && var)	/* This is not a rvalue reference always, depends on type T sent, could be made into a lvalue refernce */
{
	nextCall(std::forward<T>(var));
	//nextCall(var);  	/* If we don't forward the data correctly. Calling func with rvalue, converts into a lvalue catagory inside this function. */
			   	/* Calling func(102), still calls nextCall int& instead of nextCall int&&, To call the correct functions, a perfect forwarding is
 				 * required.
 				 */
}


int main()
{
	const int x = 42;
	forwardFunc(x);	/* Calls netxCall const int & */
	func(x);	/* Goes to func as l value refernce and calls nextCall const int& */

	int d = 12;
	forwardFunc(d); /* Calls netxCall int & */
	func(d);	/* Goes to func as l value reference and calls nextCall int& */	

	func(102);
}
