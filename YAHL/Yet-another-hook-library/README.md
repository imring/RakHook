# Yet another hook library

Hook library for x86-32, Windows

no dependencies

---

* Work with **cdecl**, **stdcall**, **thiscall** calling conventions
* Pass all parameters by reference to hook function
* Easy ignore original function call


# Examples

### Change params in hook function

```cpp
int _declspec(noinline) sum(int a, int b) {
    return a + b;
}

void hooked_sum(int& a, int& b) {
    a = 10;
    b = 20;
}

int main() {
    hook test(sum, hooked_sum);
    std::cout << sum(1, 2);

    return 0;
}
```
```
Output:
30
```

### Ignore original function
```cpp
int _declspec(noinline) sum(int a, int b) {
    return a + b;
}

int hooked_sum(int& a, int& b) {
	
    //If hook function have "void" return type, original function will be called
    //Otherwise no	
    return 42;
}

int main() {
    hook test(sum, hooked_sum);
    std::cout << sum(1, 2);

    return 0;
}
```
```
Output:
42
```


### Work well with compound types
```cpp
void _declspec(noinline) print(std::string message, int number) {
    std::cout << message << " and number " << number;
}

void hooked_print(std::string& message, int& number) {
    message += "_hooked";
    number = 1337;
}

int main() {
    hook test(print, hooked_print);
    print("hello_world", 640);
    
    return 0;
}
```
```
Output:
hello_world_hooked and number 1337
```


### Member function support
```cpp
struct dummy {
    void func(bool a) {
        std::cout << foo << " " << bar << " " << a;
    }
	
    std::string foo;
    int bar;
};

void hooked_dummy_func(dummy*& obj, bool& a) {
    obj->foo = "example hooked text";
    a = false;
}

int main() {
    hook test(&dummy::func, hooked_dummy_func);
	
    dummy tmp;
    tmp.foo = "Hello, world!!!";
    tmp.bar = 101010101;
	
    tmp.func(true);
	
    return 0;
}
```
```
Output:
example hooked text 101010101 0
```
