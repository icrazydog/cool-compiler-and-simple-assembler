#include "semant.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"

extern int semant_debug;
extern char* curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol arg, arg2, Bool, concat, cool_abort, copy, Int, in_int, in_string,
    IO, length, Main, main_meth, No_class, No_type, Object, out_int, out_string,
    prim_slot, self, SELF_TYPE, Str, str_field, substr, type_name, val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void) {
    arg = idtable.add_string("arg");
    arg2 = idtable.add_string("arg2");
    Bool = idtable.add_string("Bool");
    concat = idtable.add_string("concat");
    cool_abort = idtable.add_string("abort");
    copy = idtable.add_string("copy");
    Int = idtable.add_string("Int");
    in_int = idtable.add_string("in_int");
    in_string = idtable.add_string("in_string");
    IO = idtable.add_string("IO");
    length = idtable.add_string("length");
    Main = idtable.add_string("Main");
    main_meth = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any
    //   user-defined class.
    No_class = idtable.add_string("_no_class");
    No_type = idtable.add_string("_no_type");
    Object = idtable.add_string("Object");
    out_int = idtable.add_string("out_int");
    out_string = idtable.add_string("out_string");
    prim_slot = idtable.add_string("_prim_slot");
    self = idtable.add_string("self");
    SELF_TYPE = idtable.add_string("SELF_TYPE");
    Str = idtable.add_string("String");
    str_field = idtable.add_string("_str_field");
    substr = idtable.add_string("substr");
    type_name = idtable.add_string("type_name");
    val = idtable.add_string("_val");
}

Symbol class__class::get_name() {
    return name;
}

Symbol class__class::get_parent() {
    return parent;
}
Features class__class::get_features() {
    return features;
}

ClassTable::ClassTable(Classes classes) : semant_errors(0), error_stream(cerr) {
    /* Fill this in */
    install_basic_classes();
    bool hasMainClass = false;

    // check class inheritance
    ClassP curClass;
    ClassListP rootClassList = NULL;
    ClassListP noParentClassList = NULL;
    for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
        curClass = (ClassP)classes->nth(i);
        if (curClass->get_name() == Main)
            hasMainClass = true;

        if (curClass->get_name() == SELF_TYPE) {
            semant_error(curClass) << "Class name can not be SELF_TYPE" << endl;
            return;
        }

        for (ClassListP tempClassList = baseClassList; tempClassList != NULL;
             tempClassList = tempClassList->tl()) {
            if (curClass->get_name() == tempClassList->hd()->get_name()) {
                semant_error(curClass) << "Class name can not be '"
                                       << curClass->get_name() << "'" << endl;
                return;
            }
        }

        for (int j = i - 1; j >= 0; j--) {
            ClassP prevClass = (ClassP)classes->nth(j);
            if (curClass->get_name() == prevClass->get_name()) {
                semant_error(curClass) << "Class " << curClass->get_name()
                                       << " mutiple defined" << endl;
                return;
            }
        }

        //cout<<"class -:"<<curClass->get_name()<<endl;
        if (curClass->get_parent() == Object) {
            curClass->notRecycle = true;
            curClass->parentP = baseClassList->hd();
            baseClassList->hd()->childPList = new ClassList(
                curClass, baseClassList->hd()->childPList);
            rootClassList = new ClassList(curClass, rootClassList);
        } else if (curClass->get_parent() == IO) {
            curClass->notRecycle = true;
            curClass->parentP = baseClassList->tl()->hd();
            baseClassList->tl()->hd()->childPList = new ClassList(
                curClass, baseClassList->tl()->hd()->childPList);
            rootClassList = new ClassList(curClass, rootClassList);
        } else {
            // check class exist
            bool findClass = false;

            if (!findClass) {
                for (int j = classes->first(); classes->more(j);
                     j = classes->next(j)) {
                    if (i != j && curClass->get_parent() ==
                                      ((ClassP)classes->nth(j))->get_name()) {
                        // finded and wait recycle check
                        findClass = true;
                        curClass->parentP = (ClassP)classes->nth(j);
                        ((ClassP)classes->nth(j))->childPList =
                            new ClassList(
                                curClass,
                                ((ClassP)classes->nth(j))->childPList);

                        break;
                    }
                }
            }

            if (!findClass) {
                curClass->notRecycle = true;
                noParentClassList =
                    new ClassList(curClass, noParentClassList);
                semant_error(curClass)
                    << "Bad inheritance: " << curClass->get_name()
                    << " parent '" << curClass->get_parent()
                    << "' can not resolve" << endl;
            }
        }
    }
    

    for (ClassListP tempClassList = rootClassList; tempClassList != NULL;
         tempClassList = tempClassList->tl()) {
        triversalClass(tempClassList->hd()->childPList);
    }
    //cout<<"init ok3"<<endl;
    for (ClassListP tempClassList = noParentClassList; tempClassList != NULL;
         tempClassList = tempClassList->tl()) {
        triversalClass(tempClassList->hd()->childPList);
    }

    for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
        curClass = (ClassP)classes->nth(i);
        if (!curClass->notRecycle) {
            semant_error(curClass) << "Bad inheritance: cycle inheritance '"
                                   << curClass->get_name() << "'" << endl;
        }
    }

    if (!hasMainClass) {
        semant_error() << "Class Main is not defined." << endl;
    }

    //cout<<"init ok"<<endl;
}

void ClassTable::typeCheck(Classes) {
    methodTable = new SymbolTable<Symbol, ClassP>();
    attrTable = new SymbolTable<Symbol, Symbol>();
    Features featureList = baseClassList->hd()->get_features();

    methodTable->enterscope();
    attrTable->enterscope();

    for (int i = featureList->first(); featureList->more(i);
         i = featureList->next(i)) {
        Feature feature = featureList->nth(i);
        // add to scope
        feature->addToSymTable(baseClassList->hd());
    }
    _typeCheck(baseClassList->hd()->childPList);

    methodTable->exitscope();
    attrTable->exitscope();
}

void ClassTable::_typeCheck(ClassListP childList) {
    if (childList == NULL)
        return;

    //cout<<"class len:"<<list_length(childList)<<endl;
    for (; childList != NULL; childList = childList->tl()) {
        ClassP curClass = childList->hd();

        //cout<<"class:"<<curClass->get_name()<<endl;
        methodTable->enterscope();
        attrTable->enterscope();

        Features featureList = curClass->get_features();

          bool hasMainMethod = false;
        //cout<<"class f count:"<<featureList->len()<<endl;
        for (int i = featureList->first(); featureList->more(i);
            i = featureList->next(i)) {
            Feature feature = featureList->nth(i);
            if (feature->get_name() == main_meth) {
                hasMainMethod = true;
            }
            // add to scope
            feature->addToSymTable(curClass);
        }
        
        if (curClass->get_name() == Main && !hasMainMethod) {
            semant_error(curClass)
                << "Method 'main' of class 'Main' not found" << endl;
        }

        if (!curClass->isBaseClass)
            for (int i = featureList->first(); featureList->more(i);
                i = featureList->next(i)) {
                methodTable->enterscope();
                attrTable->enterscope();

                Feature feature = featureList->nth(i);
                feature->valid(curClass);

                methodTable->exitscope();
                attrTable->exitscope();
            }

        _typeCheck(curClass->childPList);

        methodTable->exitscope();
        attrTable->exitscope();
    }
}

void method_class::addToSymTable(ClassP curClass) {
    if (methodTable->probe(name)) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Method " << name << " multiple defined" << endl;
        return;
    }

    //cout<<"check method:"<<name<<endl;
    if (methodTable->lookup(name) != NULL) {
        ClassP* methodClassP = methodTable->lookup(name);
        ClassP methodClass = *methodClassP;

        Features featureList = methodClass->get_features();

        if (methodClass->get_name() == IO ||
            methodClass->get_name() == Object) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Method " << name << " can not override "
                << methodClass->get_name() << " method" << endl;
            return;
        }

        Formals overrideFormals;
        Symbol overrideReturnType;
        for (int i = featureList->first(); featureList->more(i);
             i = featureList->next(i)) {
            if (featureList->nth(i)->get_name() == name) {
                overrideFormals = featureList->nth(i)->get_formals();
                overrideReturnType = featureList->nth(i)->get_return_type();
                break;
            }
        }
        if (formals->len() != overrideFormals->len()) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Method " << name
                << " override parameter count not match parent" << endl;
            return;
        }
        // check type
        bool hasError = false;
        char *curMethodParam = "", *overrideMethodParam = "";
        for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
            Symbol overrideType = overrideFormals->nth(i)->get_type_decl();
            Symbol curType = formals->nth(i)->get_type_decl();
            //cout<<curType<<" "<<overrideType<<endl;
            if (overrideType != curType) {
                hasError = true;
                curMethodParam = concatStr(
                    concatStr(curMethodParam, curType->get_string()), ",");
                overrideMethodParam = concatStr(
                    concatStr(overrideMethodParam, overrideType->get_string()),
                    ",");
            }
        }

        if (hasError) {
            //cout<<"Method check error:"<<name<<endl;
            classtable->semant_error(curClass->get_filename(), this)
                << "Method " << name << " parameter type " << curMethodParam
                << " not match override parameter type " << overrideMethodParam
                << endl;
        }

        if (return_type != overrideReturnType) {
            hasError = true;
            classtable->semant_error(curClass->get_filename(), this)
                << "Method " << name << " return type not match override method"
                << endl;
        }

        if (hasError) {
            return;
        }
    }
    //cout<<"Method add:"<<name<<endl;
    ClassP* clsType = new ClassP(curClass);
    methodTable->addid(name, clsType);
}

void attr_class::addToSymTable(ClassP curClass) {
    if (name == self) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Attribute " << name << " can not be self" << endl;
        return;
    } else if (attrTable->probe(name) != NULL) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Attribute " << name << " multiple defined" << endl;
        return;
    } else if (attrTable->lookup(name) != NULL) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Attribute " << name << " can not override" << endl;
        return;
    }

    if (type_decl == SELF_TYPE) {
        // Symbol* attrType = new Symbol(curClass->get_name());
        Symbol* attrType = new Symbol(SELF_TYPE);
        attrTable->addid(name, attrType);
    } else {
        Symbol* attrType = new Symbol(get_type_decl());
        attrTable->addid(name, attrType);
    }
}

// valid method begin
Symbol method_class::valid(ClassP curClass) {
    //cout<<"method:"<<name<<endl;
      Symbol returnType = return_type;

    if (curClass->get_name() == Main && name == main_meth &&
        formals->len() > 0) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Method main of class Main should have no parameters" << endl;
        returnType=Object;
        return returnType;
    }
    bool hasError =false;
    for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
        Formal curFormal = formals->nth(i);
        if (curFormal->get_name() == self) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Method " << name << " parameter can not be self" << endl;
                hasError=true;
            continue;
        }
        if (attrTable->probe(curFormal->get_name()) != NULL) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Method " << name << " parameter "<< curFormal->get_name()<<" multiple defined" << endl;
                hasError=true;
            continue;
        }
        if (curFormal->get_type_decl() == SELF_TYPE) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Method " << name << " parameter " << curFormal->get_name()
                << " type can not be SELF_TYPE" << endl;
                hasError=true;
            continue;
        }

        if (classtable->findClass(curFormal->get_type_decl()) == NULL) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Method " << name << " parameter " << curFormal->get_name()
                << " type can not resolve" << endl;
                hasError=true;
            continue;
        }

        Symbol* attrType = new Symbol(curFormal->get_type_decl());
        attrTable->addid(curFormal->get_name(), attrType);
    }
    if(hasError){
            returnType=Object;
            return returnType;
    }

    Symbol exprReturnType = expr->valid(curClass);

    //cout<<"Method:"<<name<<" "<<exprReturnType<<endl;

    if (returnType == SELF_TYPE) {
        if (returnType != exprReturnType) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Method " << name
                << " expression return type not conform method return type"
                << endl;
            returnType=Object;
            return returnType;
        }
    } else {
        ClassP returnTypeCP = classtable->findClass(returnType);
        if (returnTypeCP == NULL) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Method " << name << " return type can not resolve" << endl;
            returnType=Object;
            return returnType;
        }

        ClassP exprReturnTypeCP;
        if (exprReturnType == SELF_TYPE) {
            exprReturnTypeCP = curClass;
        } else {
            exprReturnTypeCP = classtable->findClass(exprReturnType);
        }

        if (!classtable->isSameOrInheriteFrom(exprReturnTypeCP, returnTypeCP)) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Method " << name
                << " expression return type not conform method return type"
                << endl;
            returnType=Object;
            return returnType;
        }
    }
    return returnType;
}

Symbol attr_class::valid(ClassP curClass) {
    //cout<<"attr:"<<name<<endl;

    Symbol initType = init->valid(curClass);
    Symbol type = type_decl;
    //cout<<"attr initType:"<<name<<" "<<initType<<endl;

    ClassP typeCP;
    if (type == SELF_TYPE) {
        typeCP = curClass;
    } else {
        typeCP = classtable->findClass(type);
        if (typeCP == NULL) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Attribute " << name << " type " << type
                << " can not resolve" << endl;
            type=Object;
            return type;
        }
    }

    ClassP initTypeCP;
    if (initType == SELF_TYPE) {
        initTypeCP = curClass;
    } else {
        initTypeCP = classtable->findClass(initType);
    }

    if (initType != No_type &&
        !classtable->isSameOrInheriteFrom(initTypeCP, typeCP)) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Attribute " << name << " init type not conform attribute type"
            << endl;
        type=Object;
        return type;
    }

    return type;
}

Symbol no_expr_class::valid(ClassP curClass) {
    type = No_type;
    return type;
}
Symbol eq_class::valid(ClassP curClass) {
    Symbol type1 = e1->valid(curClass);
    Symbol type2 = e2->valid(curClass);
    if ((type1 == Int || type1 == Str || type1 == Bool || type2 == Int ||
         type2 == Str || type2 == Bool) &&
        type1 != type2) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator '=' when compare with [Int,Bool,String] type should "
               "be same"
            << endl;
        type = Object;
    } else {
        type = Bool;
    }

    return type;
}

Symbol plus_class::valid(ClassP curClass) {
    Symbol type1 = e1->valid(curClass);
    Symbol type2 = e2->valid(curClass);
    if (type1 != Int || type2 != Int) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator '+' applied to illegal type" << endl;
        type = Object;
    } else {
        type = Int;
    }
    return type;
}
Symbol sub_class::valid(ClassP curClass) {
    Symbol type1 = e1->valid(curClass);
    Symbol type2 = e2->valid(curClass);
    if (type1 != Int || type2 != Int) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator '-' applied to illegal type" << endl;
        type = Object;
    } else {
        type = Int;
    }
    return type;
}
Symbol mul_class::valid(ClassP curClass) {
    Symbol type1 = e1->valid(curClass);
    Symbol type2 = e2->valid(curClass);
    if (type1 != Int || type2 != Int) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator '*' applied to illegal type" << endl;
        type = Object;
    } else {
        type = Int;
    }
    return type;
}
Symbol divide_class::valid(ClassP curClass) {
    Symbol type1 = e1->valid(curClass);
    Symbol type2 = e2->valid(curClass);
    if (type1 != Int || type2 != Int) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator '/' applied to illegal type" << endl;
        type = Object;
    } else {
        type = Int;
    }
    return type;
}

Symbol neg_class::valid(ClassP curClass) {
    Symbol type1 = e1->valid(curClass);
    if (type1 != Int) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator '~' applied to illegal type" << endl;
        type = Object;
    } else {
        type = Int;
    }
    return type;
}
Symbol lt_class::valid(ClassP curClass) {
    Symbol type1 = e1->valid(curClass);
    Symbol type2 = e2->valid(curClass);
    if (type1 != Int || type2 != Int) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator '<' applied to illegal type" << endl;
        type = Object;
    } else {
        type = Bool;
    }

    return type;
}
Symbol leq_class::valid(ClassP curClass) {
    Symbol type1 = e1->valid(curClass);
    Symbol type2 = e2->valid(curClass);
    if (type1 != Int || type2 != Int) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator '<=' applied to illegal type" << endl;
        type = Object;
    } else {
        type = Bool;
    }

    return type;
}
Symbol comp_class::valid(ClassP curClass) {
    Symbol type1 = e1->valid(curClass);
    if (type1 != Bool) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator 'NOT' applied to illegal type" << endl;
        type = Object;
    } else {
        type = Bool;
    }
    return type;
}

Symbol isvoid_class::valid(ClassP curClass) {
    e1->valid(curClass);
    type = Bool;
    return type;
}
Symbol loop_class::valid(ClassP curClass) {
    Symbol predType = pred->valid(curClass);
    body->valid(curClass);
    if (predType != Bool) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator 'loop' condition has illegal type" << endl;
    }
    type = Object;
    return type;
}
Symbol typcase_class::valid(ClassP curClass) {
    //cout<<"case:"<<curClass->get_name()<<endl;
    expr->valid(curClass);

    List<Entry>* branchTypeList = NULL;
    List<Entry>* listCursor;
    for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
        Case curCase = cases->nth(i);
        //cout<<"branch:"<<curCase->get_name()<<endl;
        listCursor = branchTypeList;
        while (listCursor != NULL) {
            //cout<<"branch type:"<<listCursor->hd()<<endl;
            if (curCase->get_type_decl() == listCursor->hd()) {
                classtable->semant_error(curClass->get_filename(), curCase)
                    << "Case branch type multiple defined  "
                    << curCase->get_type_decl() << endl;
                type = Object;
                return type;
            }
            listCursor = listCursor->tl();
        }
        //cout<<"multiple check end"<<endl;

        if (curCase->get_type_decl() == SELF_TYPE) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Case branch " << curCase->get_name()
                << " type can not be SELF_TYPE" << endl;
            type = Object;
            return type;
        }
        branchTypeList =
            new List<Entry>(curCase->get_type_decl(), branchTypeList);

        ClassP curCaseTypeCP = classtable->findClass(curCase->get_type_decl());
        if (curCaseTypeCP == NULL) {
            classtable->semant_error(curClass->get_filename(), curCase)
                << "Case branch type " << curCase->get_type_decl()
                << " can not resolve " << endl;
            type = Object;
            return type;
        }
        attrTable->enterscope();
        Symbol* attrType = new Symbol(curCase->get_type_decl());
        attrTable->addid(curCase->get_name(), attrType);

        //cout<<"get case type"<<endl;
        Symbol exprType = curCase->get_expr()->valid(curClass);
        //cout<<"get case type end"<<endl;

        attrTable->exitscope();

        ClassP exprTypeCP = classtable->findClass(exprType);
        if (exprTypeCP == NULL) {
            classtable->semant_error(curClass->get_filename(), curCase)
                << "Case branch expresion has illegal return type " << exprType
                << endl;
            type = Object;
            return type;
        }

        if (type == NULL) {
            type = exprType;
        } else {
            type = classtable
                       ->findCommonAncestor(classtable->findClass(type),
                                            exprTypeCP)
                       ->get_name();
        }
    }
    return type;
}

Symbol let_class::valid(ClassP curClass) {
    Symbol initType = init->valid(curClass);
    Symbol varType = type_decl;

    if (identifier == self) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator 'let' var can not be self " << endl;
        type = Object;
        return type;
    }
    //cout<<"let:"<<identifier<<" "<<varType<<" "<<initType<<endl;
    if (varType == SELF_TYPE) {
        if (initType != No_type && initType != varType) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Operator 'let' init expression type " << initType
                << " not match var type" << endl;
            type = Object;
            return type;
        }
    } else if (initType != No_type) {
        if (initType == SELF_TYPE) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Operator 'let' init expression type " << initType
                << " not match var type" << endl;
            type = Object;
            return type;
        }

        ClassP varTypeCP = classtable->findClass(varType);
        if (varTypeCP == NULL) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Operator 'let' var type " << varType << " can not resolve "
                << endl;
            type = Object;
            return type;
        }

        ClassP initTypeCP = classtable->findClass(initType);
        if (initTypeCP == NULL) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Operator 'let' init expression type " << initType
                << " can not resolve " << endl;
            type = Object;
            return type;
        }

        if (!classtable->isSameOrInheriteFrom(initTypeCP, varTypeCP)) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Operator 'let' init type not conform var type" << endl;
            type = Object;
            return type;
        }
    }

    attrTable->enterscope();
    Symbol* attrType = new Symbol(varType);
    attrTable->addid(identifier, attrType);
    Symbol bodyType = body->valid(curClass);
    attrTable->exitscope();
    type = bodyType;
    return type;
}

Symbol block_class::valid(ClassP curClass) {
    for (int i = body->first(); body->more(i); i = body->next(i)) {
        Expression expr = body->nth(i);
        //cout<<"block_class: "<<expr->get_name()<<endl;
        type = expr->valid(curClass);
    }
    return type;
}

Symbol cond_class::valid(ClassP curClass) {
    Symbol predType = pred->valid(curClass);
    Symbol thenType = then_exp->valid(curClass);
    Symbol elseType = else_exp->valid(curClass);

    if (predType != Bool) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator 'if' condition type illegal " << endl;
        type = Object;
        return type;
    }

    ClassP thenTypeCP;
    if (thenType == SELF_TYPE) {
        thenTypeCP = curClass;
    } else {
        thenTypeCP = classtable->findClass(thenType);
    }

    if (thenTypeCP == NULL) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator 'if' then condition expression type " << thenType
            << " can not resolve " << endl;
        type = Object;
        return type;
    }

    ClassP elseTypeCP;
    if (elseType == SELF_TYPE) {
        elseTypeCP = curClass;
    } else {
        elseTypeCP = classtable->findClass(elseType);
    }

    if (elseTypeCP == NULL) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Operator 'if' else condition expression type " << elseType
            << " can not resolve " << endl;
        type = Object;
        return type;
    }
    type = classtable->findCommonAncestor(thenTypeCP, elseTypeCP)->get_name();
    //cout<<"cond:"<<type<<" "<<thenType<<" "<<elseType<<endl;
    return type;
}

Symbol dispatch_class::valid(ClassP curClass) {
    Symbol exprType = expr->valid(curClass);

    ClassP exprTypeCP;
    if (exprType == SELF_TYPE) {
        exprTypeCP = curClass;
    } else {
        exprTypeCP = classtable->findClass(exprType);
        if (exprTypeCP == NULL) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Dispatch type" << exprType << " can not resolve " << endl;
            type = Object;
            return type;
        }
    }
    //cout<<"dispatch class:"<<exprType<<endl;
    Feature feature = classtable->findMethod(exprTypeCP, name);
    if (feature == NULL) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Method " << name << " can not resolve " << endl;
        type = Object;
        return type;
    }
    Formals formals = feature->get_formals();
    if (formals->len() != actual->len()) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Parameter count can not match method " << endl;
        type = Object;
        return type;
    }
    bool hasError = false;
    char *methodParam = "", *dispatchParam = "";
    ClassP dispatchTypeCP;
    for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
        Symbol methodType = formals->nth(i)->get_type_decl();
        Symbol dispatchType = actual->nth(i)->valid(curClass);
        if (dispatchType == SELF_TYPE) {
            dispatchTypeCP = curClass;
        } else {
            dispatchTypeCP = classtable->findClass(dispatchType);
        }
        if (!classtable->isSameOrInheriteFrom(
                dispatchTypeCP, classtable->findClass(methodType))) {
            hasError = true;
        }
        methodParam =
            concatStr(concatStr(methodParam, methodType->get_string()), ",");
        dispatchParam = concatStr(
            concatStr(dispatchParam, dispatchType->get_string()), ",");
    }

    if (hasError) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Dispatch parameter types " << dispatchParam
            << " not conform method '" << name << "' parameter types "
            << methodParam << endl;
        type = Object;
        return type;
    }

    type = feature->get_return_type();
    if (type == SELF_TYPE) {
        type = exprType;
    }
    return type;
}
Symbol static_dispatch_class::valid(ClassP curClass) {
    Symbol exprType = expr->valid(curClass);

    ClassP exprTypeCP;
    if (exprType == SELF_TYPE) {
        exprType = curClass->get_name();
        exprTypeCP = curClass;
    } else {
        exprTypeCP = classtable->findClass(exprType);
        if (exprTypeCP == NULL) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Dispatch type" << exprType << " can not resolve " << endl;
            type = Object;
            return type;
        }
    }

    ClassP staticTypeCP = classtable->findClass(type_name);
    if (staticTypeCP == NULL) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Static dispatch type" << type_name << " can not resolve "
            << endl;
        type = Object;
        return type;
    }

    if (!classtable->isSameOrInheriteFrom(exprTypeCP, staticTypeCP)) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Dispatch type not conform to static dispatch type" << endl;
        type = Object;
        return type;
    }

    Feature feature = classtable->findMethod(staticTypeCP, name);
    if (feature == NULL) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Method " << name << " can not resolve " << endl;
        type = Object;
        return type;
    }
    Formals formals = feature->get_formals();
    if (formals->len() != actual->len()) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Parameters count can not match method " << endl;
        type = Object;
        return type;
    }
    bool hasError = false;
    char *methodParam = "", *dispatchParam = "";
    for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
        Symbol methodType = formals->nth(i)->get_type_decl();
        Symbol dispatchType = actual->nth(i)->valid(curClass);

        if (!classtable->isSameOrInheriteFrom(
                classtable->findClass(dispatchType),
                classtable->findClass(methodType))) {
            hasError = true;
        }
        methodParam =
            concatStr(concatStr(methodParam, methodType->get_string()), ",");
        dispatchParam = concatStr(
            concatStr(dispatchParam, dispatchType->get_string()), ",");
    }

    if (hasError) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Pispatch parameter types " << dispatchParam
            << " not match Method '" << name << "' parameter types "
            << methodParam << endl;
        type = Object;
        return type;
    }

    type = feature->get_return_type();
    if (type == SELF_TYPE)
        type = exprType;

    return type;
}

Symbol new__class::valid(ClassP curClass) {
    if (type_name == SELF_TYPE) {
        type = SELF_TYPE;
        // type = curClass->get_name();
    } else {
        if (classtable->findClass(type_name) == NULL) {
            classtable->semant_error(curClass->get_filename(), this)
                << "Operator 'new' class '" << type_name << "' can not resolve"
                << endl;
            type = Object;
        } else {
            type = type_name;
        }
    }
    return type;
}

Symbol string_const_class::valid(ClassP curClass) {
    type = Str;
    return type;
}
Symbol int_const_class::valid(ClassP curClass) {
    type = Int;
    return type;
}
Symbol bool_const_class::valid(ClassP curClass) {
    type = Bool;
    return type;
}

Symbol assign_class::valid(ClassP curClass) {
    Symbol* objType = attrTable->lookup(name);
    Symbol exprType = expr->valid(curClass);

    if (objType == NULL) {
        classtable->semant_error(curClass->get_filename(), this)
            << "Var '" << name << "' can not resolve" << endl;
        type = Object;
        return type;
    }

    //cout<<"assign: "<<name<<" "<< exprType <<" "<<*objType <<endl;
    ClassP objTypeCP;
    if (*objType == SELF_TYPE) {
        objTypeCP = curClass;
    } else {
        objTypeCP = classtable->findClass(*objType);
    }

    ClassP exprTypeCP;
    if (exprType == SELF_TYPE) {
        exprTypeCP = curClass;
    } else {
        exprTypeCP = classtable->findClass(exprType);
    }

    if (!classtable->isSameOrInheriteFrom(exprTypeCP, objTypeCP)) {
        //cout<<"assign var:"<<objTypeCP->get_name() << "
        // expr:"<<exprTypeCP->get_name()<<endl;

        classtable->semant_error(curClass->get_filename(), this)
            << "Var " << name << " assign type '" << exprType
            << "' not match var type '" << *objType << "'" << endl;

        type = Object;
    } else {
        type = exprType;
    }
    return type;
}

Symbol object_class::valid(ClassP curClass) {
    if (name == self) {
        // type = curClass->get_name();
        type = SELF_TYPE;
    } else {
        Symbol* objType = attrTable->lookup(name);
        if (attrTable->lookup(name) == NULL) {
            classtable->semant_error(curClass->get_filename(), this)
                << "var '" << name << "' can not resolve" << endl;
            type = Object;
        } else {
            type = *objType;
        }
    }

    return type;
}

// valid method end

bool ClassTable::isSameOrInheriteFrom(ClassP child, ClassP ancestor) {
    if (child == NULL || ancestor == NULL) {
        classtable->semant_error()
            << "system error:Inherite check params NULL" << endl;
        return false;
    }

    while (child != NULL) {
        if (child->get_name() == ancestor->get_name())
            return true;
        child = child->parentP;
    }
    return false;
}

ClassP ClassTable::findCommonAncestor(ClassP class1, ClassP class2) {
    ClassP class2Raw = class2;
    while (class1 != NULL) {
        while (class2 != NULL) {
            //cout<<"Ancestor: "<<class1->get_name() <<" "<<class2->get_name();
            if (class1 == class2) {
                return class1;
            }
            class2 = class2->parentP;
        }
        class2 = class2Raw;
        class1 = class1->parentP;
    }
    return NULL;
}

Feature ClassTable::findMethod(ClassP classp, Symbol method) {
    while (classp != NULL) {
        Features featureList = classp->get_features();
        for (int i = featureList->first(); featureList->more(i);
            i = featureList->next(i)) {
            Feature feature = featureList->nth(i);
            //cout<<"find feature:"<<feature->get_name() <<" in
            // "<<classp->get_name()<<endl;
            if (feature->get_name() == method &&
                feature->get_return_type() != NULL) {
                return feature;
            }
        }
        
        classp = classp->parentP;
    }
    return NULL;
}

ClassP ClassTable::findClass(Symbol type) {
    return _findClass(baseClassList, type);
}

ClassP ClassTable::_findClass(ClassListP classList, Symbol type) {
    if (classList == NULL)
        return NULL;
    if (classList->hd()->get_name() == type) {
        return classList->hd();
    }
    ClassP classP = _findClass(classList->hd()->childPList, type);
    if (classP != NULL)
        return classP;
    return _findClass(classList->tl(), type);
}

void ClassTable::triversalClass(ClassListP classList) {
    if (classList == NULL)
        return;
    //cout<<"triversalClass:"<<classList->hd()->get_name()<<endl;
    classList->hd()->notRecycle = true;
    triversalClass(classList->hd()->childPList);
    //cout<<"triversalClass ok1"<<endl;
    triversalClass(classList->tl());
   //cout<<"triversalClass ok2"<<endl;
}

void ClassTable::install_basic_classes() {
    // The tree package uses these globals to annotate the classes built below.
    // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");

    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.

    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    //
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class = class_(
        Object, No_class,
        append_Features(
            append_Features(single_Features(method(cool_abort, nil_Formals(),
                                                   Object, no_expr())),
                            single_Features(method(type_name, nil_Formals(),
                                                   Str, no_expr()))),
            single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
        filename);
    ((ClassP)Object_class)->isBaseClass = true;

    //
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = class_(
        IO, Object,
        append_Features(
            append_Features(
                append_Features(
                    single_Features(method(out_string,
                                           single_Formals(formal(arg, Str)),
                                           SELF_TYPE, no_expr())),
                    single_Features(method(out_int,
                                           single_Formals(formal(arg, Int)),
                                           SELF_TYPE, no_expr()))),
                single_Features(
                    method(in_string, nil_Formals(), Str, no_expr()))),
            single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
        filename);
    ((ClassP)IO_class)->isBaseClass = true;
    ((ClassP)IO_class)->parentP = (ClassP)Object_class;
    ((ClassP)Object_class)->childPList = new ClassList(
        (ClassP)IO_class, ((ClassP)Object_class)->childPList);

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer.
    //
    Class_ Int_class =
        class_(Int, Object, single_Features(attr(val, prim_slot, no_expr())),
               filename);
    baseClassList = new ClassList((ClassP)Int_class, baseClassList);
    ((ClassP)Int_class)->isBaseClass = true;
    ((ClassP)Int_class)->parentP = (ClassP)Object_class;
    ((ClassP)Object_class)->childPList = new ClassList(
        (ClassP)Int_class, ((ClassP)Object_class)->childPList);
    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
        class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),
               filename);
    baseClassList = new ClassList((ClassP)Bool_class, baseClassList);
    ((ClassP)Bool_class)->isBaseClass = true;
    ((ClassP)Bool_class)->parentP = (ClassP)Object_class;
    ((ClassP)Object_class)->childPList = new ClassList(
        (ClassP)Bool_class, ((ClassP)Object_class)->childPList);
    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //
    Class_ Str_class = class_(
        Str, Object,
        append_Features(
            append_Features(
                append_Features(
                    append_Features(
                        single_Features(attr(val, Int, no_expr())),
                        single_Features(attr(str_field, prim_slot, no_expr()))),
                    single_Features(
                        method(length, nil_Formals(), Int, no_expr()))),
                single_Features(method(concat, single_Formals(formal(arg, Str)),
                                       Str, no_expr()))),
            single_Features(
                method(substr,
                       append_Formals(single_Formals(formal(arg, Int)),
                                      single_Formals(formal(arg2, Int))),
                       Str, no_expr()))),
        filename);
    baseClassList = new ClassList((ClassP)Str_class, baseClassList);
    ((ClassP)Str_class)->isBaseClass = true;
    ((ClassP)Str_class)->parentP = (ClassP)Object_class;
    ((ClassP)Object_class)->childPList = new ClassList(
        (ClassP)Str_class, ((ClassP)Object_class)->childPList);

    // Make IO Class Second in baseClassList
    baseClassList = new ClassList((ClassP)IO_class, baseClassList);
    // Make Object Class First in baseClassList
    baseClassList = new ClassList((ClassP)Object_class, baseClassList);
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c) {
    return semant_error(c->get_filename(), c);
}

ostream& ClassTable::semant_error(Symbol filename, tree_node* t) {
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error() {
    semant_errors++;
    return error_stream;
}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */

void program_class::semant() {
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    classtable = new ClassTable(classes);

    /* some semantic analysis code may go here */

    if (classtable->errors()) {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }

    classtable->typeCheck(classes);

    if (classtable->errors()) {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }
}
