#include<iostream>
#include<vector>
#include<map>
#include<sstream>
#include<stack>
#include<math.h>

enum LapType{
	Lint=0,
	Lfloat,
	Lbool,
	Lstring,
	Lobject,
	Lfile,
	Lhandle,
	Lnative
};

using namespace std;

//定义部分
const string Symbols[30]={")","|","(","+","-","*","/","%",">","<","=","!=",">=","<=","^","<<",">>","&&","||","{","}","==",".","!","//","/*","*/","[","]",","};

const string KeyWords[12]={"function","return","if","break","continue","when","class","else","print","local","global","set"};

const string Basic_type[10]={"int","float","bool","string","void","Array","File","DLLHandle","NativeFunction","Object"};

const string Num[10]={"0","1","2","3","4","5","6","7","8","9"};

const string Bool[2]={"true","false"};

const string Logic[]={"&&","||"};

const string l1[] = { "+" , "-" };
const string l2[] = { "*","/","<<",">>","%" };
const string l3[] = { "^" };
const string l4[] = { "||","&&" };
const string l5[] = { "==",">","<",">=","<=","!=","!"};
const string l6[] = { "@" ,"."};

const string ct[4]={"int","float","bool","string"};

vector<string> error_list;

class Ins{
	public:
	string Value,arg1,arg2;
	Ins(string value,string a1="",string a2=""){
		Value=value;
		arg1=a1;
		arg2=a2;
	}
	string encode(){
		string temp=Value;
		if(arg1!=""){
			temp+=" "+arg1;
		}
		if(arg2!=""){
			temp+=" "+arg2;
		}
		return temp+'\n';
	}
};

vector<int> loop_head;

vector<Ins> out;

stringstream stream;

class var{
	public:
	int id;
	string type;
	var(int i,string t){
		id=i;
		type=t;
	}
};

vector<map<string,var> > domains;

vector<int> var_num;


int digui=0,ceng=-1,reg_max,var_count=0,label_count=0,size_add=0;

bool last_func=false,inner_call=false,inner_list=false;


class Token{
public:
	string Type,Value;
	int line,byte,small_count,const_id;
	vector<Token> sub;
	bool is_return;
	Token(string type,string value,int Line,int b,vector<Token> Sub=vector<Token>(),bool is=false,int small=0,int id=0){
		Type=type;
		Value=value;
		line=Line;
		byte=b;
		sub=Sub;
		is_return=is;
		small_count=small;
		const_id=id;
	}
	Token(){
		Type="";
		Value="";
		is_return=false;
		line=0;
		byte=0;
		sub=vector<Token>();
		small_count=0;const_id=0;
	}
	string Tostring(){
		string temp=Type+" "+Value+"\n";
		Token token;
		if(sub.size()>0){
			temp+="：[{]\n";
			for(int i=0;i<sub.size();i++){
				token=sub[i];
				temp+=token.Tostring()+"\n";
			}
			temp+="[}]\n";
		}
		return temp;
	}
	bool operator==(Token other){
		return other.Type==this->Type&&other.Value==this->Value;
	}
};

vector<Token> consts;

long long quickpower(long long x,long long y)
{
    long long ans=1,cnt=x;
    while(y)
    {
        if(y&1)
        {
            ans*=cnt;
        }
        cnt*=cnt;
        y>>=1;
    }
    return ans;
}

bool Is_in_s(const string *onstr,const string *onget,int count){
	bool temp=false;
	for(int i=0;i<count;i++){
		if((*onget)==onstr[i]){
			temp=true;
			break;
		}
	}
	return temp;
}

int stoi(string &onstr){
	int temp=0,max=onstr.length();
	for(int i=0;i<max;i++){
		temp+=(onstr[max-i-1]-'0')*(int)pow(10,i);
	}
	return temp;
}

int max(int n1,int n2){
	if(n1>n2){
		return n1;
	}
	return n2;
}

string Tostring(int value){
	string temp;
	stream<<value;
	stream>>temp;
	stream.clear();
	return temp;
}

string Tostring(double value){
	string temp;
	stream<<value;
	stream>>temp;
	stream.clear();
	return temp;
}

string Tostring(float value){
	string temp;
	stream<<value;
	stream>>temp;
	stream.clear();
	return temp;
}

string Tostring(size_t value){
	string temp;
	stream<<value;
	stream>>temp;
	stream.clear();
	return temp;
}
/*
string itoh(int i,int encode_length){
	string temp="";
	while(i>0){
		temp=HEX[i%16]+temp;
		i/=16;
	}
	for(int i=temp.length();i<encode_length;i++){
		temp='0'+temp;
	}
	return temp;
}*/

int floatstoi(string *s){
	string onstr;
	for(int i=0;i<s->length();i++){
		if((*s)[i]=='.'){
			continue;
		}
		onstr+=(*s)[i];
	}
	return stoi(onstr);
}

string Position(int line,int byte){
	return Tostring(line)+"."+Tostring(byte);
}

void Parse_Token(FILE *fp,vector<Token> *token_list){
	string onstr(""),onget(" "),linshi,last_type,last_op("");
	char last_get;
	int line=1,byte=0,small_count;
	bool is_func=false,is_float=false;
	Token token;
	while(onget[0]!=EOF){
		onget[0]=fgetc(fp);
		byte++;
		if(linshi=="//"){
			while(onget[0]!=EOF&&onget[0]!='\n'){
				onget[0]=fgetc(fp);
				if(onget[0]=='\n'){
					line++;
					byte=0;
					token_list->push_back(Token("break","\n",line,1));
					break;
				}
			}
			linshi="";
			continue;
		}
		else if(onget[0]==EOF){
			if(onstr.length()>0){
				token_list->push_back(Token(last_type,onstr,line,byte));
			}
			break;
		}
		if(Is_in_s(KeyWords,&onstr,12)){
			last_type="keyword";
			if(onstr=="function"){
				is_func=true;
			}
		}
		else if(Is_in_s(Bool,&onstr,2)){
			last_type="bool";
		}
		else if(Is_in_s(Basic_type,&onstr,10)){
			last_type="type";
		}
		if(onget[0]=='\n'){
			if(onstr.length()>0){
				token_list->push_back(Token(last_type,onstr,line,byte));
				onstr="";
			}
			token_list->push_back(Token("break","\n",line,byte));
			line++;
			byte=0;
		}
		else if(onget[0]==' '){
			if(onstr.length()>0){
				token_list->push_back(Token(last_type,onstr,line,byte));
				onstr="";
			}
		}
		else if(Is_in_s(Symbols,&onget,30)){
			if(last_type!="op"&&onstr.length()>0){
				token_list->push_back(Token(last_type,onstr,line,byte));
				onstr="";
			}
			linshi=last_op+onget[0];
			last_op=onget;
			if(Is_in_s(Symbols,&linshi,30)&&linshi.size()>1){
				if(last_type=="op"){
					token_list->pop_back();
				}
				if(linshi!="//"){
					token_list->push_back(Token("op",linshi,line,byte));
				}
			}
			else{
				if(onget[0]=='('&&token_list->back().Type=="var"&&!is_func){
					token_list->push_back(Token("call",onget,line,byte));
				}
				else{
					token_list->push_back(Token("op",onget,line,byte));
					is_func=false;
				}
			}
			last_type="op";
		}
		else if(Is_in_s(Num,&onget,10)){
			small_count=0;
			if(last_type!="var"&&last_type!="type"){
				if(onstr.length()>0){
					token_list->push_back(Token(last_type,onstr,line,byte));
					onstr="";
				}
				onstr=onget;
				onget[0]=fgetc(fp);
				while(Is_in_s(Num,&onget,10)||onget[0]=='.'){
					onstr+=onget[0];
					if(onget[0]=='.'){
						if(!is_float){
							is_float=true;
						}
						else{
							error_list.push_back("错误："+Position(line,byte)+" 数字格式错误");
							break;
						}
					}
					else if(is_float){
						small_count++;
					}
					onget[0]=fgetc(fp);
				}
				if(!error_list.empty()){
					break;
				}
				if(is_float){
					token_list->push_back(Token("float",onstr,line,byte,vector<Token>(),false,small_count));
				}
				else{
					token_list->push_back(Token("int",onstr,line,byte,vector<Token>(),false,small_count));
				}
				is_float=false;
				onstr="";
				fseek(fp,-1,SEEK_CUR);
			}
			else{
				onstr+=onget[0];
			}
		}
		else if(onget[0]=='"'){
			onstr="";
			onget[0]=fgetc(fp);
			while(onget[0]!='"'){
				if(onget[0]=='\n'||onget[0]==EOF){
					error_list.push_back("错误："+Position(line,byte)+" 字符串末尾缺少<\">");
					break;
				}
				if(last_get=='\\'&&onget[0]=='"'){
					onstr+=onget[0];
					onget[0]=0;
				}
				else if(onget[0]==' '){
					onstr+="\\b";
				}
				else{
					onstr+=onget[0];
				}
				last_get=onget[0];
				onget[0]=fgetc(fp);
			}
			token_list->push_back(Token("string",onstr,line,byte));
			onstr="";
		}
		else{
			last_type="var";
			onstr+=onget[0];
		}
		if(error_list.size()>0){
			break;
		}
	}
}



int op_Level(const string op)
{
	if(Is_in_s(l1,&op,2))
		return 1;
	if (Is_in_s(l2, &op, 5))
		return 2;
	if (Is_in_s(l3, &op, 1))
		return 3;
	if (Is_in_s(l4, &op, 2))
		return 4;
	if (Is_in_s(l5, &op, 7))
		return 5;
	if (Is_in_s(l6, &op, 2))
		return 0;
	return -1;

}

vector<Token> Trans_exp(vector<Token>& token_list)//by奔跑的小蜗牛 优化和bug修复 魔凤啸天(hacklinjiuyue)
{
	stack<Token> s;
	vector<Token> out;
	int cur = 0,size = token_list.size();
	while (cur < size)
	{
		if (token_list[cur].Value == "(")
		{
			s.push(token_list[cur]);
		}

		else if (token_list[cur].Value == ")") //弹出内容直到遇到第一个左括号
		{
			while (s.top().Value != "(" && !
				   s.empty())
			{
				out.push_back(s.top());
				s.pop();
			}
			s.pop();//弹出左括号
		}
		else if(token_list[cur].Type == "op")
		{
			while (!s.empty())
			{
				if(op_Level(s.top().Value) < op_Level(token_list[cur].Value)){
					break;
				}
				out.push_back(s.top());
				s.pop();
			}
			s.push(token_list[cur]);
		}
		else{
			out.push_back(token_list[cur]);
		}
		cur++;
	}
	while (!s.empty())
	{
		out.push_back(s.top());
		s.pop();
	}
	return out;
}

vector<Token> Fold(vector<Token> &token_list){
	vector<Token> temp,*arg_list=NULL,*arg=NULL;
	Token token;
	string name;
	int kh,line,byte;
	bool is_return=false;
	for(int i=0;i<token_list.size();i++){
		token=token_list[i];
		if(token.Value=="/*"){
			i++;
			for(;i<token_list.size();i++){
				token=token_list[i];
				if(token.Value=="*/"){
					break;
				}
			}
			if(i==token_list.size()){
				error_list.push_back("错误："+Position(token.line,token.byte)+" 注释语句缺少结束符'/*'");
				break;
			}
		}
		else if(token.Value=="*/"){
			error_list.push_back("错误："+Position(line,byte)+" 注释语句缺少起始符'/*'");
			break;
		}
		else if(token.Type=="call"){
			kh=1;
			arg=new vector<Token>();
			arg_list=new vector<Token>();
			token=temp.back();
			temp.pop_back();
			name=token.Value;
			line=token.line;
			byte=token.byte;
			i++;
			for(;i<token_list.size();i++){
				token=token_list[i];
				if(token.Value=="("){
					kh++;
				}
				else if(token.Value==")"){
					kh--;
				}
				if(kh==0){
					if(arg->size()>0){
						arg_list->push_back(Token("arg",name,line,byte,Fold(*arg)));
					}
					break;
				}
				else if(token.Value==","&&kh==1){
					arg_list->push_back(Token("arg",name,line,byte,Fold(*arg)));
					delete arg;
					arg=new vector<Token>();
				}
				else{
					arg->push_back(token);
				}
			}
			if(kh>0){
				error_list.push_back("错误："+Position(line,byte)+" 函数调用缺少')'");
				break;
			}
			temp.push_back(Token("callbox",name,line,byte,*arg_list));
			delete arg_list;
			delete arg;
		}
		else if(token.Value=="("){
			temp.push_back(token);
			i++;
			line=token.line;
			byte=token.byte;
			kh=1;
			for(;i<token_list.size();i++){
				token=token_list[i];
				temp.push_back(token);
				if(token.Value=="("){
					kh++;
				}
				else if(token.Value==")"){
					kh--;
				}
				if(kh==0){
					break;
				}
			}
			if(kh>0){
				error_list.push_back("错误："+Position(line,byte)+" 表达式缺少')'");
				break;
			}
		}
		else if(token.Value=="["){
			kh=1;
			arg=new vector<Token>();
			arg_list=new vector<Token>();
			line=token.line;
			byte=token.byte;
			i++;
			for(;i<token_list.size();i++){
				token=token_list[i];
				if(token.Value=="["){
					kh++;
				}
				else if(token.Value=="]"){
					kh--;
				}
				if(kh==0){
					if(arg->size()>0){
						arg_list->push_back(Token("exp","",token.line,token.byte,Fold(*arg)));
					}
					break;
				}
				else{
					arg->push_back(token);
				}
			}
			if(kh>0){
				error_list.push_back("错误："+Position(line,byte)+" 数组索引缺少']'");
				break;
			}
			temp.push_back(Token("op","@",line,byte,*arg_list));
			delete arg_list;
			delete arg;
		}
		else{
			temp.push_back(token);
		}
		if(error_list.size()>0){
			break;
		}
	}
	return temp;
}

string encodeConst(int type){
	string temp;
	switch(type){
	case 0:
		temp="int";
	break;
	case 1:
		temp="float";
	break;
	case 2:
		temp="bool";
	break;
	case 3:
		temp="str";
	break;
	}
	return "add_const_"+temp;
}

void Parse_exp(vector<Token> &exp,bool is_set,map<string,var> &domain,bool is_global){
	vector<Token> s;
	int i=0,max=exp.size(),x,m;
	Token op1,op2,op;
	string next_type,is_ref;
	map<string,var>::iterator iter;
	for(;i<max;i++){
		op=exp[i];
		if(op.Type==""){
			break;
		}
		if(op.Type=="op"){
			op2=s.back();
			s.pop_back();
			if(op.Value=="!"){
				next_type=op2.Type;
				out.push_back(Ins("not"));
			}
			else{
				op1=s.back();
				next_type=op1.Type;
				s.pop_back();
				if(op.Value=="+"){
					out.push_back(Ins("add"));
				}
				else if(op.Value=="-"){
					out.push_back(Ins("sub"));
				}
				else if(op.Value=="*"){
					out.push_back(Ins("mul"));
				}
				else if(op.Value=="/"){
					out.push_back(Ins("div"));
				}
				else if(op.Value=="%"){
					out.push_back(Ins("mod"));
				}
				else if(op.Value=="@"){
					if(is_set&&exp[i+1].Type==""){
						out.push_back(Ins("set_index"));
					}
					else{
						out.push_back(Ins("index"));
					}
				}
				else if(op.Value=="."){
					//
				}
			}
			s.push_back(Token(next_type,"",op2.line,op2.byte,vector<Token>()));
		}
		else{
			if(op.Type=="var"){
				iter=domain.find(op.Value);
				if(iter==domain.end()){
					error_list.push_back("错误："+Position(op.line,op.byte)+" 变量'"+op.Value+"'未在作用域中定义");
					break;
				}
				else{
					if(is_set){
						is_ref="t";
					}
					else{
						is_ref="f";
					}
					if(exp[i+1].Value!="."){
						op.Type=iter->second.type;
						if(is_global){
							out.push_back(Ins("push_var_global",Tostring(iter->second.id),is_ref));
						}
						else{
							out.push_back(Ins("push_var_local",Tostring(iter->second.id),is_ref));
						}
					}
				}
			}
			else{
				x=0;
				m=consts.size();
				for(;x<m;x++){
					if(consts[x]==op){
						break;
					}
				}
				out.push_back(Ins("push_const",Tostring(x)));
				s.push_back(op);
			}
		}
	}
}

void Grammar_check(vector<Token> &tokens){
	int i=0,posi;
	int max=tokens.size();
	Token token,next,type;
	bool err=false,result;
	int tab=0,last_tab=0;
	int line,byte,m;
	map<string,var> domain;
	vector<Token> exp=vector<Token>();
	map<string,var>::iterator iter;
	domains.push_back(map<string,var>());
	var_num.push_back(0);
	for(;i<max;i++){
		if(tokens[i].Value[0]=='\n'){
			continue;
		}
		tab=0;
		while(tokens[i].Value[0]=='\t'){
			tab++;
			i++;
		}
		m=domains.size();
		if(tab>m-1){
			for(;m<tab;m++){
				domains.push_back(domains[m-1]);
				var_num.push_back(0);
			}
		}
		else if(tab!=last_tab){
			for(;tab<last_tab;last_tab--){
				domains.pop_back();
				var_num.pop_back();
			}
		}
		token=tokens[i];
		line=token.line;
		byte=token.byte;
		if(token.Type=="keyword"){
			if(token.Value=="print"){
				i++;
				exp=vector<Token>();
				for(;i<max;i++){
					token=tokens[i];
					if(token.Value[0]=='\n'){
						break;
					}
					if(token.Type=="keyword"){
						err=true;
						error_list.push_back("错误："+Position(line,byte)+" 表达式中不能出现关键字");
						break;
					}
					exp.push_back(token);
				}
				if(err){
					break;
				}
				if(exp.size()==0){
					error_list.push_back("错误："+Position(line,byte)+" 表达式不能为空");
					break;
				}
				exp=Trans_exp(exp);
				exp.push_back(Token());
				Parse_exp(exp,false,domains[tab],tab==0);
				if(!error_list.empty()){
					break;
				}
				out.push_back(Ins("print"));
			}
			else if(token.Value=="set"){
                i++;
                token=tokens[i];
                iter=domains[0].find(token.Value);
                string t="";
                if(iter==domains[0].end()){
					iter=domains[tab].find(token.Value);
					if(iter==domains[tab].end()){
						error_list.push_back("错误："+Position(line,byte)+" 变量'"+token.Value+"'未在此作用域中定义");
						break;
					}
					t="local";
                }
                else{
					t="global";
                }
				i++;
				exp=vector<Token>();
				for(;i<max;i++){
					token=tokens[i];
					if(token.Value[0]=='\n'){
						break;
					}
					if(token.Type=="keyword"){
						err=true;
						error_list.push_back("错误："+Position(line,byte)+" 表达式中不能出现关键字");
						break;
					}
					exp.push_back(token);
				}
				if(err){
					break;
				}
				if(exp.size()==0){
					error_list.push_back("错误："+Position(line,byte)+" 表达式不能为空");
					break;
				}
				exp=Trans_exp(exp);
				exp.push_back(Token());
				Parse_exp(exp,false,domains[tab],tab==0);
				if(!error_list.empty()){
					break;
				}
				out.push_back(Ins("set_var_"+t,Tostring(iter->second.id)));
			}
			else{
				i++;
				type=tokens[i];
				if(!Is_in_s(Basic_type,&type.Value,10)){
					error_list.push_back("错误："+Position(type.line,type.byte)+" 类型'"+type.Value+"'无效");
					break;
				}
				i++;
				next=tokens[i];
				if(token.Value=="local"){
					iter=domains[tab].find(next.Value);
					posi=var_num[tab]++;
					domains[tab].insert(pair<string,var>(next.Value,var(posi,type.Value)));
					result=iter==domains[tab].end();
				}
				else{
					iter=domains[0].find(next.Value);
					posi=var_num[0]++;
					domains[tab].insert(pair<string,var>(next.Value,var(posi,type.Value)));
					result=iter==domains[0].end();
				}
				if(!result){
					error_list.push_back("错误："+Position(line,byte)+" 变量'"+next.Value+"'已在此作用域中定义");
					break;
				}
				i++;
				exp=vector<Token>();
				type.Type=token.Value;
				for(i;i<max;i++){
					token=tokens[i];
					if(token.Value[0]=='\n'){
						break;
					}
					if(token.Type=="keyword"){
						err=true;
						error_list.push_back("错误："+Position(line,byte)+" 表达式中不能出现关键字");
						break;
					}
					exp.push_back(token);
				}
				if(err){
					break;
				}
				if(exp.size()==0){
					error_list.push_back("错误："+Position(line,byte)+" 表达式不能为空");
					break;
				}
				exp=Trans_exp(exp);
				exp.push_back(Token());
				Parse_exp(exp,false,domains[tab],tab==0);
				if(!error_list.empty()){
					break;
				}
				out.push_back(Ins("store_var_"+type.Type,Tostring(posi)));
			}
		}
		last_tab=tab;
	}
}

void Compile_file(string File_name,char* temp_name){
	FILE *fp=fopen(File_name.c_str(),"r");
	if(fp==NULL){
		cout<<"错误：文件"<<File_name<<"不存在\n";
		return;
	}
	vector<Token> token_list;
	vector<string> temp;
	Parse_Token(fp,&token_list);
	int m=token_list.size();
	Token token;
	for(int i=0;i<m;i++){
		if(Is_in_s(ct,&token_list[i].Type,4)){
			token=token_list[i];
			int x=0,max=consts.size();
			bool on=true;
			for(;x<max;x++){
				if(consts[x]==token){
					on=false;
					break;
				}
			}
			if(on){
				consts.push_back(token);
				if(token.Type=="int"){
					out.push_back(Ins(encodeConst(Lint),token.Value));
				}
				else if(token.Type=="float"){
					out.push_back(Ins(encodeConst(Lfloat),token.Value));
				}
				else if(token.Type=="bool"){
					out.push_back(Ins(encodeConst(Lbool),token.Value));
				}
				else if(token.Type=="string"){
					out.push_back(Ins(encodeConst(Lstring),token.Value));
				}
			}
		}
	}
	fclose(fp);
	if(error_list.empty()){
		token_list=Fold(token_list);
	}
	token=token_list.back();
	token_list.push_back(Token("break","\n",token.line+1,0));
	if(error_list.empty()){
		last_func=false;
		Grammar_check(token_list);
		//语法解析
	}
	if(error_list.empty()){
		fp=fopen(temp_name,"w+");
		for(int i=0;i<out.size();i++){
			fputs(out[i].encode().c_str(),fp);
		}
		fclose(fp);
	}
}

//

int main(int argc,char* argv[]){
	if(argc>1){
		string on=string(argv[1]);
		if(on=="-h"){
			printf("Lapc by hacklinjiuyue v0.74a\n--------------------\n  lapc -h for help\n  lapc x.lap to compile the file\n--------------------\n");
		}
		else{
			//string t,int c=-1,vector<var> arg=vector<var>(),vector<Token> i=vector<Token>(),vector<int> len=vector<int>(),int ID=0
			Compile_file(argv[1],argv[2]);
			if(error_list.size()>0){
				printf("在%s：\n",argv[1]);
				for(int i=0;i<error_list.size();i++){
					printf("%s\n",error_list[i].c_str());
				}
			}
			system("pause");
		}
	}
}
