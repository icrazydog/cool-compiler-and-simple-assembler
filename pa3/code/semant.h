#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include <string>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#define TRUE 1
#define FALSE 0

class ClassTable;

ClassTable* classtable;
SymbolTable<Symbol,ClassP> *methodTable;
SymbolTable<Symbol,Symbol> *attrTable;

typedef ClassTable *ClassTableP;
typedef List<class__class> *ClassListP;
typedef List<class__class> ClassList;



// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  int semant_errors;
  ostream& error_stream;
  ClassListP baseClassList;
  
  void install_basic_classes();

  void _typeCheck(ClassListP childList);
  ClassP _findClass(ClassListP classList, Symbol type);
  void triversalClass(ClassListP classList);

public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
  void typeCheck(Classes);

  bool isSameOrInheriteFrom(ClassP child, ClassP ancestor);
  ClassP findCommonAncestor(ClassP class1, ClassP class2);
  Feature findMethod(ClassP classp,Symbol method);
  ClassP findClass(Symbol type);
};


char * concatStr(char * a,char * b){
     char *newStr = (char *) malloc(strlen(a) + strlen(b));
    strcpy(newStr, a);
    strcat(newStr, b);
    return newStr;
}

#endif

