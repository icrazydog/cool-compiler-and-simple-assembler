#include<iostream> 
#include <iomanip>
#include"stdio.h"
#include"conio.h"
#include"testc.h"
using namespace std; 
typedef class Cs *Program;
 STYLE style;
 class Cs{
     public:
     int d;
 };

 extern int extern_test();



Enumtest enumtest = EB;
int main(void) 
{ 
    Program p =new Cs();
    p->d = 1;
    cout<<"hello vscode"<<endl; 
    cout<< '\\' << oct << setfill('0') << setw(3)<<123<<dec << setfill(' ')<<15;

    style.a='a';
    cout<<style.a<<endl;

    cout<<p->d<<endl;

    if(enumtest == EB){
          cout<<"enumtest:EB"<<endl;

    }

    extern_test();
    getch();
    return 0;
} 