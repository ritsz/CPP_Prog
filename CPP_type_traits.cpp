#include <iostream>
#include <type_traits>

using namespace std;

/*
struct true_type {
	static constexpr bool value = true;
};

struct false_type {
	static constexpr bool value = false;
};

template<typename T>
struct is_floating {
	static constexpr bool value = false;
};

template<>
struct is_floating<float> {
	static constexpr bool value = true;
};
*/

template<class T, T v>
struct my_integral_constant {
	static constexpr T value = v;
	typedef T value_type;
};

using true_constant = my_integral_constant<bool, true>;
using false_constant = my_integral_constant<bool, false>;

template <class T, class U>
struct my_is_same : false_constant {};
template<class T>
struct my_is_same<T, T> : true_constant {};

/* PARTIAL TEMPLATE SPECIALIZATION SP typename T needed */
template<class T>
struct my_remove_volatile { typedef T type; };
template<class T>
struct my_remove_volatile<volatile T> { typedef T type; };

template<typename T>
struct my_remove_const { typedef T type; };
template<typename T>
struct my_remove_const<const T> { typedef T type; };

template<typename T>
struct my_remove_cv {
	typedef typename remove_volatile< typename remove_const<T>::type >::type type;
};

/* FULL SPECIALIZATION SO typename T not added */
template<typename T>
struct is_floating : my_integral_constant<bool, 
					  my_is_same<float,T>::value   || 
					  my_is_same<float, typename my_remove_cv<T>::type>::value> {};

int main()
{
	int a = 10;
	float b = 5.0;
	const float c = 6.0;

	//decltype(a);
	cout << is_floating<decltype(a)>::value << endl;
	cout << is_floating<decltype(b)>::value << endl;
	cout << is_floating<decltype(c)>::value << endl;
	cout << is_floating<my_remove_const<decltype(c)>::type>::value << endl;
}
