/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

int comment_parentthese_count = 0;

%}

/*
 * Define names for regular expressions here.
 */
DIGIT               [0-9]+
TYPE_IDENTIFIER     [A-Z][0-9a-zA-Z_]*
OBJECT_IDENTIFIER   [a-z][0-9a-zA-Z_]*
DARROW              =>
WHITESPACE          [ \f\r\t\v]+

%x COMMENT_STATE
%x STRING_STATE
%x STRING_RESUME_STATE
%%

 /*
  *  Nested comments
  */
"(*" {
  BEGIN(COMMENT_STATE);
   // printf("%s!",yytext); 
  comment_parentthese_count++;
}
"*)" {
  cool_yylval.error_msg = "Unmatched *)";
  return ERROR; 
}

<COMMENT_STATE>"(*" {
  comment_parentthese_count++;
}
<COMMENT_STATE>"*)" {
  comment_parentthese_count--;
  if(comment_parentthese_count==0)
    BEGIN(INITIAL);
}

<COMMENT_STATE><<EOF>> {
  cool_yylval.error_msg = "EOF in comment"; 
  BEGIN(INITIAL);
  return ERROR; 
}
<COMMENT_STATE>. {}

<COMMENT_STATE>\n {curr_lineno++;}



--.* {}

  /*
   * Keywords are case-insensitive except for the values true and false,
   * which must begin with a lower-case letter.
   */
(?i:class)      { return CLASS; }
(?i:else)       { return ELSE; }
(?i:fi)         { return FI; }
(?i:if)         { return IF; }
(?i:in)         { return IN; }
(?i:inherits)   { return INHERITS; }
(?i:isvoid)     { return ISVOID; }
(?i:let)        { return LET; }
(?i:loop)       { return LOOP; }
(?i:pool)       { return POOL; }
(?i:then)       { return THEN; }
(?i:while)      { return WHILE; }
(?i:case)       { return CASE; }
(?i:esac)       { return ESAC; }
(?i:new)        { return NEW; }
(?i:of)         { return OF; }
(?i:not)        { return NOT;   }  
t(?i:rue)       { cool_yylval.boolean = true; return BOOL_CONST;   }  
f(?i:alse)      { cool_yylval.boolean = false;return BOOL_CONST;   }  


 /*
  *  Intager
  */
{DIGIT} {
  cool_yylval.symbol = inttable.add_string(yytext);
  return INT_CONST;
}

 /*
  *  Identifiers
  */
{TYPE_IDENTIFIER} {
  cool_yylval.symbol = idtable.add_string(yytext);
  return TYPEID;
}
{OBJECT_IDENTIFIER} {
  cool_yylval.symbol = idtable.add_string(yytext);
  return OBJECTID;
}

 /*
  *  The single-character operators.
  */
"+" {return int('+');}
"~" {return int('~');}
"-" {return int('-');}
"*" {return int('*');}
"/" {return int('/');}
"<" {return int('<');}
"=" {return int('=');}

 /*
  *  Special symbol
  */
"(" {return int('(');}
")" {return int(')');}
"{" {return int('{');}
"}" {return int('}');}
":" {return int(':');}
"," {return int(',');}
";" {return int(';');}
"@" {return int('@');}
"." {return int('.');}

 /*
  *  The multiple-character operators.
  */
{DARROW}		{ return (DARROW); }
"<-" {return ASSIGN;}
"<=" {return LE;}


 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */
"\"" {
  BEGIN(STRING_STATE);
  string_buf_ptr = string_buf;
  *string_buf_ptr = '\0';
}

<STRING_STATE>"\"" {
  if(string_buf_ptr-string_buf>MAX_STR_CONST-1){
    cool_yylval.error_msg = "String constant too long";
    BEGIN(STRING_RESUME_STATE);
    return ERROR; 
  }

  
  *string_buf_ptr = '\0';
  cool_yylval.symbol = stringtable.add_string(string_buf);

  BEGIN(INITIAL);
  return STR_CONST;
}

<STRING_STATE><<EOF>> {   
  cool_yylval.error_msg = "EOF in string constant";
  BEGIN(INITIAL);
  return ERROR; 
}

<STRING_STATE>\n {
  curr_lineno++; 
  cool_yylval.error_msg = "Unterminated string constant";
  BEGIN(INITIAL);
  return ERROR; 
}

<STRING_STATE>(\0|"\\\0") { 
  cool_yylval.error_msg = "String contains null character";
  BEGIN(STRING_RESUME_STATE);
  return ERROR; 
}

<STRING_STATE>"\\n" { *string_buf_ptr++ = '\n'; }
<STRING_STATE>"\\t" { *string_buf_ptr++ = '\t'; }
<STRING_STATE>"\\b" { *string_buf_ptr++ = '\b'; }
<STRING_STATE>"\\f" { *string_buf_ptr++ = '\f'; }
<STRING_STATE>"\\".	{*string_buf_ptr++ = yytext[1];}
<STRING_STATE>"\\\n"	{curr_lineno++;*string_buf_ptr++ = '\n';}
<STRING_STATE>.	{*string_buf_ptr++ = *yytext;}

<STRING_RESUME_STATE>[^"\n] {}
<STRING_RESUME_STATE>"\"" {BEGIN(INITIAL);}
<STRING_RESUME_STATE>\n {
  curr_lineno++;
  BEGIN(INITIAL);
}

{WHITESPACE} {}
\n {curr_lineno++;}

. {
    cool_yylval.error_msg = yytext;
    return ERROR;
}

%%
