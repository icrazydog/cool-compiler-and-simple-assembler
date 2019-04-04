/*
 mips assembler
 */
#include <cstring> 
#include <cstdlib>
#include <cmath>
#include <string> 
#include <iomanip> 
#include <iostream> 
#include <fstream> 

using namespace std;

#define BIT_MASK_5 0x0000001F
#define BIT_MASK_6 0x0000003F
#define BIT_MASK_16 0x0000FFFF
#define BIT_MASK_26 0x05FFFFFF

struct LineInfo{
    int lineno;
    string raw;
    int gen;
    //R J I for instruction P for pseudo instruction
    //T for tag,S for segment
    //D for data
    char type;
    int address;
    int instructionSize;
    string instruction;
    string params[3];
    struct LineInfo *next=NULL;
};
typedef struct LineInfo* LineInfoP;


LineInfoP parseLine(bool gen,int lineno,string line,LineInfoP lastLineInfoP);
void genMechineCode(LineInfoP lastLineInfoP);
int iIns(int op,int rs,int rt,int immed);
int rIns(int op,int rs,int rt,int rd,int shamt,int funct);
int jIns(int op,int addr);
int findReg(string &regName);
int findTag(LineInfoP lineInfoListP,string tag);
string trim(string s);
string getToken(string &s,const char *delim);
string toHex(int i);
int endianConvertLittleToBig(int littleInt);

int segType=0;
string reg[32] ={"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3","$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7","$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7","$t8","$t9","$gp","$sp","$fp","$ra"};
ofstream exefile;

int main(int argc,char* argv[]) {
    char filename[80];
    char filename2[80];
    string line;
    LineInfoP lineInfoListP=NULL,lineInfoP=NULL;

    if(argc!=3){
         cout<<"params error:\n    usage:assembler xxx.s xxx.bin";
         return 1;
    }
    strcpy(filename,argv[1]);
    strcpy(filename2,argv[2]);
    if(strcmp(filename,filename2)==0){
        cout<<"output filename can not be input filename";
        return 1;
    }
    if(strstr(filename2,".s")){
        cout<<"output filename can not contain .s";
        return 1;
    }

    ifstream file(filename,ios::in); 
    if (!file.is_open()){
        perror("open file fail");
        return 1;
    }
    
    exefile.open(filename2,ios::out|ios::binary);
    if (!exefile.is_open()){
        perror("create output file fail");
        return 1;
    }


    int lineno=1;
    while (getline(file,line)){
       lineInfoP = parseLine(false,lineno++,line,lineInfoP);
       if(lineInfoListP==NULL&&lineInfoP!=NULL){
           lineInfoListP = lineInfoP;
       }
    }
    file.close();

    //print all line
    cout<<"------------file content----------"<<endl;
    lineInfoP = lineInfoListP;
    while(lineInfoP!=NULL){
        if(!lineInfoP->gen)
            cout<<lineInfoP->lineno<<"# "<<lineInfoP->raw<<endl;
        lineInfoP= lineInfoP->next;
    }


    cout<<"------------tag list----------"<<endl;
    lineInfoP = lineInfoListP;
    while(lineInfoP!=NULL){
        if(lineInfoP->type=='T')
            cout<<lineInfoP->instruction<<" : 0x"<<setw(8)<<setfill('0')<<hex<<lineInfoP->address<<endl;
        lineInfoP= lineInfoP->next;
    }

    cout<<"------------assembly code----------"<<endl;
    lineInfoP = lineInfoListP;
    while(lineInfoP!=NULL){
        if(lineInfoP->type!='T'&&lineInfoP->type!='S'&&lineInfoP->type!='P')
            cout<<"0x"<<toHex(lineInfoP->address)<<":"<<lineInfoP->raw<<endl;
        lineInfoP= lineInfoP->next;
    }

    cout<<"------------code----------"<<endl;   
    genMechineCode(lineInfoListP);

    exefile.close();

}

/*
    PASS 1: get address for instrucation
*/
LineInfoP parseLine(bool gen,int lineno,string line,LineInfoP lastLineInfoP){
    LineInfoP newlineInfoP = new LineInfo;
    string childLine;
    newlineInfoP->lineno=lineno;
    newlineInfoP->raw = line;
    newlineInfoP->gen = gen;

    line=trim(line);
    if(line==".text"){
        segType=1;
        newlineInfoP->type='S';
        newlineInfoP->instructionSize=0;
    }else if(line==".data"){
        if(segType!=1){
            cerr<<"Assembling fail:.data must after .text"<<endl;
            exit(0);
        }
        segType=2;
        newlineInfoP->type='S';
        newlineInfoP->instructionSize=0;
     }else if(line.find("la ")==0){
        newlineInfoP->type='P';
        newlineInfoP->instruction="la";
        newlineInfoP->instructionSize=0;
        line = line.substr(3);
        newlineInfoP->params[0]=getToken(line,",");
        newlineInfoP->params[1]=getToken(line,",");

        if(lastLineInfoP!=NULL){
            newlineInfoP->address = lastLineInfoP->address +  lastLineInfoP->instructionSize;
            lastLineInfoP->next = newlineInfoP;
        }

       int highBit =  atoi(newlineInfoP->params[1].c_str())>>16;
       char highBitStr[6];
        sprintf(highBitStr, "%d", highBit);
        
       string tempLine = "lui $at,";
       tempLine.append(highBitStr);
       LineInfoP newlineInfoP2= parseLine(true,lineno,tempLine,newlineInfoP);
       tempLine ="ori "+ newlineInfoP->params[0]+",$at,"+newlineInfoP->params[1];
       return parseLine(true,lineno,tempLine,newlineInfoP2);
        

    }else if(line.find("lui ")==0){
        newlineInfoP->type='I';
        newlineInfoP->instruction="lui";
        newlineInfoP->instructionSize=4;
        line = line.substr(4);
        newlineInfoP->params[0]=getToken(line,",");
        newlineInfoP->params[1]=line;

    }else if(line.find("ori ")==0){
        newlineInfoP->type='I';
        newlineInfoP->instruction="ori";
        newlineInfoP->instructionSize=4;
        line = line.substr(4);
        newlineInfoP->params[0]=getToken(line,",");
        newlineInfoP->params[1]=getToken(line,",");
        newlineInfoP->params[2]=line;


    }else if(line.find("addi ")==0){
        newlineInfoP->type='I';
        newlineInfoP->instruction="addi";
        newlineInfoP->instructionSize=4;
        line = line.substr(5);
        newlineInfoP->params[0]=getToken(line,",");
        newlineInfoP->params[1]=getToken(line,",");
        newlineInfoP->params[2]=line;


    }else if(line.find("lw ")==0){
        newlineInfoP->type='I';
        newlineInfoP->instruction="lw";
        newlineInfoP->instructionSize=4;
        line = line.substr(3);
        newlineInfoP->params[0]=getToken(line,",");
        newlineInfoP->params[1]=line;


    }else if(line.find("sw ")==0){
        newlineInfoP->type='I';
        newlineInfoP->instruction="sw";
        newlineInfoP->instructionSize=4;
        line = line.substr(3);
        newlineInfoP->params[0]=getToken(line,",");
        newlineInfoP->params[1]=line;


    }else if(line.find(".space ")==0){
        newlineInfoP->type='D';
        newlineInfoP->instruction=".space";
        newlineInfoP->params[0]=trim(line.substr(6,line.length()-6));
        int byte = atoi(newlineInfoP->params[0].c_str());
        newlineInfoP->instructionSize=((int)ceil(byte/4.0))*4;
    }else if(line.find(".word ")==0){
        newlineInfoP->type='D';
        newlineInfoP->instruction=".word";
        newlineInfoP->params[0]=trim(line.substr(5,line.length()-5));
       newlineInfoP->instructionSize= 4;
    }else if(line.find(".ascii ")==0){
        newlineInfoP->type='D';
        newlineInfoP->instruction=".ascii";
        newlineInfoP->params[0]=trim(line.substr(6,line.length()-6));
        if(newlineInfoP->params[0].length()<=2){
            cerr<<"Assembling fail: illegal data at line"<<lineno<<endl;
            exit(0);
        }
        newlineInfoP->params[0] = newlineInfoP->params[0].substr(1,newlineInfoP->params[0].length()-2);
        int byte = newlineInfoP->params[0].length();
        newlineInfoP->instructionSize=((int)ceil(byte/4.0))*4;
    }else if(line.find(".asciiz ")==0){
        newlineInfoP->type='D';
        newlineInfoP->instruction=".asciiz";
        newlineInfoP->params[0]=trim(line.substr(7,line.length()-7));
        if(newlineInfoP->params[0].length()<=2){
            cerr<<"Assembling fail: illegal data at line "<<lineno<<endl;
            exit(0);
        }
        newlineInfoP->params[0] = newlineInfoP->params[0].substr(1,newlineInfoP->params[0].length()-2);
        int byte = newlineInfoP->params[0].length()+1;
        newlineInfoP->instructionSize=((int)ceil(byte/4.0))*4;
    }else{
        int pos = line.find_first_of(":");
        if(pos!=-1){
            newlineInfoP->type='T';
            newlineInfoP->instruction=trim(line.substr(0,pos));
            newlineInfoP->instructionSize=0;

            if(pos+1<line.length()){
                childLine = trim(line.substr(pos+1,line.length()-pos-1));
            }
        }else if(segType==1){
            newlineInfoP->type='\0';
            newlineInfoP->instructionSize=4;
        }
       
    }
    if(lastLineInfoP!=NULL){
        newlineInfoP->address = lastLineInfoP->address +  lastLineInfoP->instructionSize;
        lastLineInfoP->next = newlineInfoP;
    }
    if(!childLine.empty())
        return parseLine(true,lineno,childLine,newlineInfoP);
    else
        return newlineInfoP;
}
/*
 PASS 2: generate instrucation MechineCode
*/
void genMechineCode(LineInfoP lineInfoListP){
    LineInfoP lineInfoP=lineInfoListP;
    while(lineInfoP!=NULL){
        if(lineInfoP->type=='R'){
            int op,rs,rt,rd,shamt,funct;
            if(lineInfoP->instruction=="jr"){
                op=0;
                rt=0;
                rd=0;
                shamt=0;
                funct=8;
                rs=findReg(lineInfoP->params[0]);
                if(rs==-1){
                    cerr<<"Assembling fail:unknow register at line "<<lineInfoP->lineno<<endl;
                    exit(0);
                }
            }else{
                cerr<<"Assembling fail:unknow instruction at line "<<lineInfoP->lineno<<endl;
                exit(0);
            }
            int code= rIns(op, rs, rt, rd, shamt, funct);
            cout<<"0x"<<toHex(lineInfoP->address)<<":"
                <<"0x"<<toHex(code)<<endl;
            code = endianConvertLittleToBig(code);
            exefile.write((char*) &code, 4);
        }else if(lineInfoP->type=='I'){
            int op,rs,rt,immed;
            if(lineInfoP->instruction=="lui"){
                op=15;
                rs=0;
                rt=findReg(lineInfoP->params[0]);
                if(rt==-1){
                    cerr<<"1Assembling fail:unknow register at line "<<lineInfoP->lineno<<endl;
                    exit(0);
                }

                char firstChar = *lineInfoP->params[1].c_str();
                if(firstChar>='0'&&firstChar<='9'){
                     immed=atoi(lineInfoP->params[1].c_str());
                }else{
                    //find label
                    immed = findTag(lineInfoListP,lineInfoP->params[1]);
                }
               
            }else if(lineInfoP->instruction=="ori"){
                op=13;
                rt=findReg(lineInfoP->params[0]);
                if(rt==-1){
                    cerr<<"Assembling fail:unknow register at line "<<lineInfoP->lineno<<endl;
                    exit(0);
                }

                rs=findReg(lineInfoP->params[1]);
                if(rs==-1){
                    cerr<<"Assembling fail:unknow register at line "<<lineInfoP->lineno<<endl;
                    exit(0);
                }

                char firstChar = *lineInfoP->params[2].c_str();
                if(firstChar>='0'&&firstChar<='9'){
                     immed=atoi(lineInfoP->params[2].c_str());
                }else{
                    //find label
                    immed = findTag(lineInfoListP,lineInfoP->params[2]);
                }

            }else if(lineInfoP->instruction=="lw"){
                op=35;
                rt=findReg(lineInfoP->params[0]);
                if(rt==-1){
                    cerr<<"Assembling fail:unknow register at line "<<lineInfoP->lineno<<endl;
                    exit(0);
                }
                
                string param = lineInfoP->params[1];
                immed=atoi(getToken(param,"(").c_str());
                rs=findReg(param.erase(param.length()-1));
                if(rs==-1){
                    cerr<<"Assembling fail:unknow register at line "<<lineInfoP->lineno<<endl;
                    exit(0);
                }

            }else if(lineInfoP->instruction=="sw"){
                op=43;
                rt=findReg(lineInfoP->params[0]);
                if(rt==-1){
                    cerr<<"Assembling fail:unknow register at line "<<lineInfoP->lineno<<endl;
                    exit(0);
                }

                string param = lineInfoP->params[1];
                immed=atoi(getToken(param,"(").c_str());
                rs=findReg(param.erase(param.length()-1));
                if(rs==-1){
                    cerr<<"Assembling fail:unknow register at line "<<lineInfoP->lineno<<endl;
                    exit(0);
                }
            }else if(lineInfoP->instruction=="addi"){
                op=8;
                rt=findReg(lineInfoP->params[0]);
                if(rt==-1){
                    cerr<<"Assembling fail:unknow register at line "<<lineInfoP->lineno<<endl;
                    exit(0);
                }

                rs=findReg(lineInfoP->params[1]);
                if(rs==-1){
                    cerr<<"Assembling fail:unknow register at line "<<lineInfoP->lineno<<endl;
                    exit(0);
                }

                char firstChar = *lineInfoP->params[2].c_str();
                if(firstChar>='0'&&firstChar<='9'){
                     immed=atoi(lineInfoP->params[2].c_str());
                }else{
                    //find label
                    immed = findTag(lineInfoListP,lineInfoP->params[2]);
                }
            }else{
                cerr<<"Assembling fail:unknow instruction at line "<<lineInfoP->lineno<<endl;
                exit(0);
            }
            int code= iIns(op, rs, rt, immed);
            cout<<"0x"<<toHex(lineInfoP->address)<<":"
                <<"0x"<<toHex(code)<<endl;
            code = endianConvertLittleToBig(code);
            exefile.write((char*) &code, 4);
        }else if(lineInfoP->type=='J'){
            int op,addr;
            if(lineInfoP->instruction=="j"){
                op=2;
                char firstChar = *lineInfoP->params[0].c_str();
                if(firstChar>='0'&&firstChar<='9'){
                    addr =atoi(lineInfoP->params[0].c_str()); 
                }else{
                    //find label
                    addr = findTag(lineInfoListP,lineInfoP->params[0]);
                }
                if(addr<0){
                    cerr<<"Assembling fail:error address at line "<<lineInfoP->lineno<<endl;
                    exit(0);
                }
            }else if(lineInfoP->instruction=="jal"){
                op=3;
                char firstChar = *lineInfoP->params[0].c_str();
                if(firstChar>='0'&&firstChar<='9'){
                    addr =atoi(lineInfoP->params[0].c_str()); 
                }else{
                    //find label
                    addr = findTag(lineInfoListP,lineInfoP->params[0]);
                }
                if(addr<0){
                    cerr<<"Assembling fail:error address at line "<<lineInfoP->lineno<<endl;
                    exit(0);
                }
            }else{
                cerr<<"Assembling fail:unknow instruction at line "<<lineInfoP->lineno<<endl;
                exit(0);
            }

            int code= jIns(op, addr);
            cout<<"0x"<<toHex(lineInfoP->address)<<":"
                <<"0x"<<toHex(code)<<endl;
            code = endianConvertLittleToBig(code);
            exefile.write((char*) &code, 4);
        }else if(lineInfoP->type=='D'){
            if(lineInfoP->instruction==".word"){
                int code  = strtol(lineInfoP->params[0].c_str(), NULL, 16);

                cout<<"0x"<<toHex(lineInfoP->address)<<":"
                    <<"0x"<<toHex(code)<<endl;
                code = endianConvertLittleToBig(code);
                exefile.write((char*) &code, 4);
            }else if(lineInfoP->instruction==".space"){
                int size = lineInfoP->instructionSize/4;
                
                int i=0;
                while(i<size){
                    cout<<"0x"<<toHex(lineInfoP->address+i)<<":"
                        <<"0x"<<toHex(0)<<endl;
                    exefile.write("\0\0\0\0", 4);
                    i++;
                }
            }else if(lineInfoP->instruction==".ascii"){
                const char *strP =lineInfoP->params[0].c_str();
                int size = lineInfoP->instructionSize/4;
                int len = lineInfoP->params[0].length();

                int i=0;
                int j=0;
                int code;
                int c;
                while(i<size){
                    code=0;
                    while(j<len){
                       c = *strP++;
                       j++;
                       c = c << ((4-j)<<3);
                       code = code | c;
                       if(j%4==0)
                           break;
                    }
                    cout<<"0x"<<toHex(lineInfoP->address+i)<<":"
                        <<"0x"<<toHex(code)<<endl;
                    code = endianConvertLittleToBig(code);
                    exefile.write((char*) &code, 4);
                    i++;
                }
            }else if(lineInfoP->instruction==".asciiz"){
                const char *strP =lineInfoP->params[0].c_str();
                int size = lineInfoP->instructionSize/4;
                int len = lineInfoP->params[0].length();
                
                int i=0;
                int j=0;
                int code;
                int c;
                while(i<size){
                    code=0;
                    while(j<len){
                       c = *strP++;
                       j++;
                       c = c << ((4-j)<<3);
                       code = code | c;
                       if(j%4==0)
                           break;
                    }
                    cout<<"0x"<<toHex(lineInfoP->address+i)<<":"
                        <<"0x"<<toHex(code)<<endl;
                    code = endianConvertLittleToBig(code);
                    exefile.write((char*) &code, 4);
                    i++;
                }
            }else{
                cerr<<"Assembling fail:unknow data instruction at line "<<lineInfoP->lineno<<endl;
                exit(0);
            }
        }
        lineInfoP=lineInfoP->next;
    }
}

/*
    I instruction
     6      5     5     16
    |------|-----|-----|------------------|
    |  op  | rs  | rt  | immediate        |
    |------|-----|-----|------------------|
*/
int iIns(int op,int rs,int rt,int immed){
    return ((op & BIT_MASK_6) <<26) | ((rs & BIT_MASK_5) <<21) | 
    ((rt & BIT_MASK_5) <<16) | (immed & BIT_MASK_16);
}

/*
    R instruction
     6      5     5     5     5     6
    |------|-----|-----|-----|-----|------|
    |  op  | rs  | rt  | rd  |shamt|funct |
    |------|-----|-----|-----|-----|------|
*/
int rIns(int op,int rs,int rt,int rd,int shamt,int funct){
    return ((op & BIT_MASK_6) <<26) | ((rs & BIT_MASK_5) <<21) | 
    ((rt & BIT_MASK_5) <<16) | ((rd & BIT_MASK_5) <<11) |
    ((shamt & BIT_MASK_5) <<6) | (funct & BIT_MASK_6);
}
/*
    J instruction
     6      26
    |------|------------------------------|
    | op   |           address            |
    |------|------------------------------|
*/
int jIns(int op,int addr){
   return ((op & BIT_MASK_6) <<26) | (addr & BIT_MASK_26);
}

int findTag(LineInfoP lineInfoListP,string tag){
    LineInfoP lineInfoP = lineInfoListP;
    while(lineInfoP!=NULL){
        if(lineInfoP->type=='T'&&
        lineInfoP->instruction==tag){
            return lineInfoP->address;
        }
        lineInfoP= lineInfoP->next;
    }
    return -1;
}
int findReg(string &regName){
    for(int i=0;i<32;i++){
        if(reg[i]==regName){
            return i;
        }
    }
    return -1;
}

string getToken(string &s,const char *delim){
    if (s.empty()) 
    {
        return trim(s);
    }
    int idx = s.find_first_of(delim);
    if(idx==-1){
        return s;
    }
    string token = s.substr(0,idx);
    s = s.substr(token.length()+1);
   
    return trim(token);
}


string trim(string s){
    if (s.empty()) 
    {
        return s;
    }
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}


string toHex(int i){
    char temp[9];
    sprintf(temp,"%08X\n",i);
    temp[8]='\0';
    return temp;
}

int endianConvertLittleToBig(int littleInt){
    unsigned char *p = (unsigned char*)&littleInt; 
    int sum = 0;
    for (int i = 0; i < 4; i++)
    {
        sum += ((int)*(p + i)) << (24-i*8);
    }
    return sum;
}