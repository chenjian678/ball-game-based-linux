#include <iostream>
using namespace std;

class Base1 {
public:
    int a;
    Base1(int val) : a(val) { 
        cout << "构造 Base1( a=" << a << " )" << endl; 
    }
    ~Base1() { 
        cout << "析构 Base1" << endl; 
    }
};

class Base2 {
public:
    int b;
    Base2(int val) : b(val) { 
        cout << "构造 Base2( b=" << b << " )" << endl; 
    }
    ~Base2() { 
        cout << "析构 Base2" << endl; 
    }
};

class Derived : public Base1, public Base2 {
public:
    int c;
    Derived(int v1, int v2, int v3) : Base1(v1), Base2(v2), c(v3) {
        cout << "构造 Derived( c=" << c << " )" << endl;
    }
    
    ~Derived() { 
        cout << "析构 Derived" << endl; 
    }
};

int main() {
    Derived d(10, 20, 30);
    return 0;
}