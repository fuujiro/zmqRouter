#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
using namespace std;

class Test{
    private:
        char* prefix = "world from ";
    public:
        char* sout(char* str);
};

char* Test::sout(char* str) {
    // strcat(prefix,str);
    return prefix;
}

// 封装C接口
extern "C"{
// 创建对象
    Test* test_new(){
        return new Test;
    }
    char* my_sout(Test* t, char* str){
        return t->sout(str);
    }
}
