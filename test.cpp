#include<iostream>
using namespace std; 

int main(){
    string s = "abc";
    cout<<s.substr(0,0) + "*" + s.substr(1,3-0-1);
    return 0;
}