#include<iostream>
#include<vector>
#include<map>
#include<algorithm>
#include<sstream>
#include<stack>

using namespace std;

//定义部分

const string Symbols[25]={")","(","+","-","*","/","%",">","<","=","!=",">=","<=","^","<<",">>","&&","||","[","]","{","}","==",",","!"};

const string KeyWords[10]={"func","return","if","break","continue","while","for","class","else","elseif"};

const string Basic_type[6]={"int","float","double","bool","string","void"};

const string Num[10]={"0","1","2","3","4","5","6","7","8","9"};

const string Bool[2]={"true","false"};

const string Func_lack_msg[3]={"类型","函数名","'('"};

const string Five_basic[5]={"+","-","*","/","%"};

const string l1[] = { "+" , "-" };
const string l2[] = { "*","/","<<",">>","!","%" };
const string l3[] = { "^" };
const string l4[] = { "||","&&" };
const string l5[] = { "==",">","<",">=","<=","!=" };
const string l6[] = { "=" };
const string l7[] = { "@" };

class Class{
	map<string,string> member;
};

vector<string> error_list,conv_bool,conv_int,conv_float;

map<string,Class> Class_type;

map<string,vector<string> > convert;

stringstream stream;

string last_func_name,last_func_type;

bool last_func=false;

class Token{
public:
	string Type,Value;
	int line,byte;
	vector<Token> sub;
	bool is_float;
	Token(string type,string value,int Line,int b,vector<Token> Sub=vector<Token>(),bool is=false){
		Type=type;
		Value=value;
		line=Line;
		byte=b;
		sub=Sub;
		is_float=is;
	}
	Token(){
		Type="";
		Value="";
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
};

class var{
public:
	string type,name;
	var(string t,string n){
		type=t;
		name=n;
	}
};

class data{
public:
	string type;
	int argc;
	vector<var> args;
	vector<Token> init;
	data(string t,int c=-1,vector<var> arg=vector<var>(),vector<Token> i=vector<Token>()){
		type=t;
		argc=c;
		args=arg;
		init=i;
	}
};

vector<var> *last_arg=NULL;

map<string,data> global;

//
//

void debug_write(string msg){
	FILE *fp=fopen(".\\log.txt","a+");
	fputs((msg+"\n").c_str(),fp);
	fclose(fp);
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

string Position(int line,int byte){
	return Tostring(line)+"."+Tostring(byte);
}

void Parse_Token(FILE *fp,vector<Token> *token_list){
	string onstr(""),onget(" "),linshi,last_type;
	char last_get;
	int line=1,byte=0;
	bool is_float=false,is_func=false;
	while(onget[0]!=EOF){
		onget[0]=fgetc(fp);
		byte++;
		if(onget[0]==EOF){
			if(onstr.length()>0){
				token_list->push_back(Token(last_type,onstr,line,byte));
			}
			break;
		}
		else if(onget[0]=='\t'){
			continue;
		}
		if(Is_in_s(KeyWords,&onstr,10)){
			last_type="keyword";
			if(onstr=="func"){
				is_func=true;
			}
		}
		else if(Is_in_s(Bool,&onstr,2)){
			last_type="bool";
		}
		else if(Is_in_s(Basic_type,&onstr,6)){
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
		else if(Is_in_s(Symbols,&onget,25)){
			if(last_type!="op"&&onstr.length()>0){
				token_list->push_back(Token(last_type,onstr,line,byte));
				onstr="";
			}
			last_type="op";
			linshi=onstr+onget[0];
			if(Is_in_s(Symbols,&linshi,25)&&linshi.length()>1){
				token_list->push_back(Token("op",linshi,line,byte));
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
		}
		else if(Is_in_s(Num,&onget,10)){
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
						}
					}
					onget[0]=fgetc(fp);
				}
				if(is_float){
					token_list->push_back(Token("float",onstr,line,byte,vector<Token>(),is_float));
				}
				else{
					token_list->push_back(Token("int",onstr,line,byte,vector<Token>(),is_float));
				}
				is_float=false;
				onstr="";
				if(onget[0]!=EOF){
					if(onget[0]=='\n'){
						token_list->push_back(Token("break","\n",line,byte));
						line++;
					}
					else{
						token_list->push_back(Token("op",onget,line,byte));
					}
				}
			}
			else{
				onstr+=onget[0];
			}
		}
		else if(onget[0]=='"'){
			onstr="";
			onget[0]=fgetc(fp);
			while(onget[0]!='"'){
				if(onget[0]==';'||onget[0]=='\n'||onget[0]==EOF){
					error_list.push_back("错误："+Position(line,byte)+" 字符串末尾缺少<\">");
					break;
				}
				if(last_get=='\\'){
					onstr+=onget[0];
					onget[0]=0;
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
	if (Is_in_s(l2, &op, 6))
		return 2;
	if (Is_in_s(l3, &op, 1))
		return 3;
	if (Is_in_s(l4, &op, 2))
		return 4;
	if (Is_in_s(l5, &op, 6))
		return 5;
	if (Is_in_s(l6, &op, 1))
		return 0;
	if (Is_in_s(l7, &op, 1))
		return 6;
	return -1;

}

bool Check_convert(string on_type,string to_type){
	map<string,vector<string> >::iterator iter=convert.find(to_type);
	bool temp=false;
	for(int i=0;i<iter->second.size();i++){
		if(iter->second[i]==on_type){
			temp=true;
			break;
		}
	}
	return temp;
}

void Check_exp(vector<Token> &token_list,map<string,data> *domain){
	vector<Token> stack;
	Token token,op1,op2;
	string op,type1,type2;
	map<string,data>::iterator iter;
	int equal_count=0;
	for(int i=0;i<token_list.size();i++){
		token=token_list[i];
		op=token.Value;
		if(token.Type=="op"){
			op2=stack.back();
			stack.pop_back();
			if(op2.Type=="var"){
				iter=domain->find(op2.Value);
				if(iter==domain->end()){
					iter=global.find(op2.Value);
					if(iter==global.end()){
						error_list.push_back("错误："+Position(op2.line,op2.byte)+" 变量'"+op2.Value+"'未定义");
						break;
					}
				}
				if(iter->second.argc!=-1){
					error_list.push_back("错误："+Position(op2.line,op2.byte)+" 函数指针'"+op2.Value+"'不能参与运算");
					break;
				}
				type2=iter->second.type;
			}
			else{
				type2=op2.Type;
			}
			if(op=="!"){
				if(op2.Type!="bool"&&!Check_convert(op2.Type,"bool")){
					error_list.push_back("错误："+Position(op2.line,op2.byte)+" 类型'"+op2.Type+"'无法转换为'bool'类型进行非运算");
					break;
				}
			}
			else{
				if(stack.size()==0){
					error_list.push_back("错误："+Position(token.line,token.byte)+" 运算符'"+op+"'缺少第二个操作值");
					break;
				}
				op1=stack.back();
				stack.pop_back();
				if(op1.Type=="var"){
					iter=domain->find(op1.Value);
					if(iter==domain->end()){
						iter=global.find(op1.Value);
						if(iter==global.end()){
							error_list.push_back("错误："+Position(op1.line,op1.byte)+" 变量'"+op1.Value+"'未定义");
							break;
						}
					}
					if(iter->second.argc!=-1){
						error_list.push_back("错误："+Position(op1.line,op1.byte)+" 函数'"+op1.Value+"'不能参与运算");
						break;
					}
					type1=iter->second.type;
				}
				else{
					type1=op1.Type;
				}
				if(type1!=type2&&!Check_convert(type2,type1)){
					error_list.push_back("错误："+Position(op2.line,op2.byte)+" 类型'"+type2+"'无法转换为'"+type1+"'类型进行'"+op+"'运算");
					break;
				}
			}
			stack.push_back(Token(type1,"",op2.line,op2.byte,vector<Token>(),false));
		}
		else{
			stack.push_back(token);
		}
	}
	if(!error_list.empty()){
		return;
	}
	token=stack.back();
	if(stack.size()>1){
		error_list.push_back("错误："+Position(token.line,token.byte)+" 变量无法在没有运算符的情况下并列出现");
		return;
	}
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

int Check_format(vector<Token> &token_list,int index,map<string,data> *domain){//无错返回目标index,否则返回-1
	Token token=token_list[index];

	bool is_block=false;
	int count=0;
	if(token.Value=="func"){
		bool is_comma=false,is_right=false;
		int len;
		vector<string> name,type;
		vector<var> args;
		len=token_list.size()-index-1;
		if(token_list.size()-index-1<3){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义缺少"+Func_lack_msg[len]);
			return -1;
		}
		index++;
		token=token_list[index];
		if(!Is_in_s(Basic_type,&token.Value,6)){
			error_list.push_back("错误："+Position(token.line,token.byte)+" '"+token.Value+"'不是一个有效的类型");
			return -1;
		}
		last_func_type=token.Value;
		index++;
		token=token_list[index];
		if(global.find(token.Value)!=global.end()||domain->find(token.Value)!=domain->end()){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 变量'"+token.Value+"'被重复定义");
			return -1;
		}
		last_func_name=token.Value;
		index++;
		token=token_list[index];
		if(token.Value!="("){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 该位置要求的符号为'('，不是'"+token.Value+"'");
			return -1;
		}
		index++;
		for(;index<token_list.size();index++){
			token=token_list[index];
			if(token.Value==")"){
				is_right=true;
				break;
			}
			else if(is_comma){
				is_comma=false;
				if(token.Type=="var"){
					name.push_back(token.Value);
				}
				else{
					error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义缺少变量名");
					break;
				}
			}
			else if(token.Value==","){
				is_comma=true;
			}
			else if(token.Type=="var"){
				name.push_back(token.Value);
			}
			else{
				error_list.push_back("错误："+Position(token.line,token.byte)+" 非变量不能出现在参数名集中");
				break;
			}
		}
		if(error_list.size()>0){
			return -1;
		}
		if(!is_right){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义的参数名集缺少')'");
			return -1;
		}
		index++;
		token=token_list[index];
		if(token.Value!="<"){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 该位置要求的符号为'<'，不是'"+token.Value+"'");
			return -1;
		}
		is_right=false;
		is_comma=false;
		index++;
		for(;index<token_list.size();index++){
			token=token_list[index];
			if(token.Value==">"){
				is_right=true;
				break;
			}
			else if(is_comma){
				is_comma=false;
				if(token.Type=="type"){
					if(!Is_in_s(Basic_type,&token.Value,6)){
						error_list.push_back("错误："+Position(token.line,token.byte)+" '"+token.Value+"'不是一个有效的类型");
						break;
					}
					type.push_back(token.Value);
				}
				else{
					error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义的参数类型集缺少类型");
					break;
				}
			}
			else if(token.Value==","){
				is_comma=true;
			}
			else if(token.Type=="type"){
				if(!Is_in_s(Basic_type,&token.Value,6)&&Class_type.find(token.Value)==Class_type.end()){
					error_list.push_back("错误："+Position(token.line,token.byte)+" '"+token.Value+"'不是一个有效的类型");
				}
				type.push_back(token.Value);
			}
			else{
				error_list.push_back("错误："+Position(token.line,token.byte)+" 非类型不能出现在参数类型集中");
				break;
			}
		}
		if(error_list.size()>0){
			return -1;
		}
		if(!is_right){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义的参数类型集缺少'>'");
			return -1;
		}
		if(name.size()!=type.size()){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义的参数个数与参数类型的个数不符 "+Position(name.size(),type.size()));
			return -1;
		}
		index++;
		if(token_list[index].Type!="codebox"){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义规定需要代码块");
			return -1;
		}
		index--;
		for(int i=0;i<name.size();i++){
			args.push_back(var(type[i],name[i]));
		}
		domain->insert(pair<string,data>(last_func_name,data(last_func_type,args.size(),args)));
		last_func=true;
		last_arg=new vector<var>(args);
	}
	else if(token.Value=="while"||token.Value=="if"){
		index++;
		for(;index<token_list.size();index++){
			if(token_list[index].Type=="codebox"){
				is_block=true;
				break;
			}
			count++;
		}
		if(!is_block){
			error_list.push_back("错误："+Position(token.line,token.byte)+" '"+token.Value+"'规定需要代码块");
			return -1;
		}
		index-=count+1;
	}
	else if(token.Value=="else"){
		index--;

		while(token_list[index].Type=="break"&&index>0){
			index--;
			count++;
		}
		if(token_list[index].Type!="codebox"){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 'else'规定需要有从属的if语句块");
			return -1;
		}
		while(token_list[index].Type!="keyword"&&index>0){
			index--;
			count++;
		}
		if(token_list[index].Value!="if"){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 'else'规定需要有从属的if语句");
			return -1;
		}
		index+=count+2;
		count=0;
		for(;index<token_list.size();index++){
			if(token_list[index].Type=="codebox"){
				is_block=true;
				break;
			}
			count++;
		}
		if(!is_block){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 'else'规定需要代码块");
			return -1;
		}
		index-=count+1;
	}
	return index;
}

void Grammar_check(vector<Token> &token_list,vector<string> *temp,map<string,data> *domain){
	string onstr;
	Token token;
	vector<Token> *on=NULL,s;
	map<string,data> *local=NULL;
	var x=var("","");
	bool last_f=last_func,last_return=false;
	int equal_count;
	if(last_func){
		last_func=false;
		local=new map<string,data>();
		for(int i=0;i<last_arg->size();i++){
			x=(*last_arg)[i];
			local->insert(pair<string,data>(x.name,data(x.type)));
		}
		delete last_arg;
	}
	else{
		local=domain;
	}
	for(int i=0;i<token_list.size();i++){
		token=token_list[i];
		onstr=token.Type;
		if(onstr=="keyword"){
			last_return=true;
			i=Check_format(token_list,i,local);
		}
		else{
			if(onstr=="type"){
				if(!Is_in_s(Basic_type,&token.Value,6)&&Class_type.find(token.Value)==Class_type.end()){
					error_list.push_back("错误："+Position(token.line,token.byte)+" '"+token.Value+"'不是一个有效的类型");
					break;
				}
				last_func_type=token.Value;
				i++;
				for(;i<token_list.size();i++){
					token=token_list[i];
					if(token.Type=="break"){
						break;
					}
					if(token.Type!="var"){
						error_list.push_back("错误："+Position(token.line,token.byte)+" '"+token.Value+"'不是一个有效的变量名");
						break;
					}
					else{
						last_func_name=token.Value;
						if(global.find(last_func_name)!=global.end()||local->find(last_func_name)!=local->end()){
							error_list.push_back("错误："+Position(token.line,token.byte)+" '"+last_func_name+"'被重复定义");
							break;
						}
						i++;
						on=new vector<Token>();
						for(;i<token_list.size();i++){
							token=token_list[i];
							if(token.Value==","){
								break;
							}
							else if(token.Type=="break"){
								i--;
								break;
							}
							on->push_back(token);
						}
						local->insert(pair<string,data>(last_func_name,data(last_func_type,-1,vector<var>(),Trans_exp(*on))));
						delete on;
					}
				}
			}
			else if(onstr=="codebox"){
				if(token_list[i-1].Type=="var"){
					error_list.push_back("错误："+Position(token.line,token.byte)+" 错误的'{'位置");
					break;
				}
				Grammar_check(token.sub,temp,local);
			}
			else{
				on=new vector<Token>();
				equal_count=0;
				for(;i<token_list.size();i++){
					token=token_list[i];
					if(token.Value==","||token.Type=="break"||token.Type=="codebox"){
						s.push_back(Token("exp","",token.line,token.byte,Trans_exp(*on)));
						onstr=token.Type;
						token=s.back();
						if(token.sub.size()>1){
							if(equal_count==0&&!last_return){
								error_list.push_back("错误："+Position(token.line,token.byte)+" 表达式无法作为单独的语句存在");
								break;
							}
							Check_exp(token.sub,local);
							if(!error_list.empty()){
								break;
							}
						}
						else if(!token.sub.empty()){
							if(token.sub[0].Type!="callbox"&&equal_count==0&&!last_return){
								token=token.sub[0];
								error_list.push_back("错误："+Position(token.line,token.byte)+" 表达式无法作为单独的语句存在");
								break;
							}
						}
						if(onstr=="break"){
							break;
						}
						if(onstr=="codebox"){
							i--;
							break;
						}
						delete on;
						equal_count=0;
						on=new vector<Token>();
					}
					else{
						if(token.Value=="="){
							equal_count++;
						}
						if(equal_count>1){
							error_list.push_back("错误："+Position(token.line,token.byte)+" 表达式中不允许出现多个'='");
							break;
						}
						on->push_back(token);
					}
				}
				if(on!=NULL){
					delete on;
				}
			}
			last_return=false;
		}
		if(error_list.size()>0){
			break;
		}
	}
	if(local!=NULL&&last_f){
		delete local;
	}
}

vector<Token> Fold(vector<Token> &token_list){
	vector<Token> temp,*arg_list=NULL,*arg=NULL;
	Token token;
	string name;
	int kh,line,byte;
	for(int i=0;i<token_list.size();i++){
		token=token_list[i];
		if(token.Type=="call"){
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
		else if(token.Value=="{"){
			arg=new vector<Token>();
			kh=1;
			i++;
			line=token.line;
			byte=token.byte;
			for(;i<token_list.size();i++){
				token=token_list[i];
				line=token.line;
				byte=token.byte;
				if(token.Value=="}"){
					kh--;
				}
				else if(token.Value=="{"){
					kh++;
				}
				if(kh==0){
					break;
				}
				arg->push_back(token);
			}
			if(kh>0){
				error_list.push_back("错误："+Position(line,byte)+" 代码块缺少'}'");
				break;
			}

			temp.push_back(Token("codebox","",line,byte,Fold(*arg)));
			delete arg;
			delete arg_list;
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

void Compile_file(string File_name){
	FILE *fp=fopen(File_name.c_str(),"r");
	if(fp==NULL){
		cout<<"错误：文件"<<File_name<<"不存在\n";
		return;
	}
	vector<Token> token_list;
	token_list.push_back(Token("break","\n",0,0));
	token_list.push_back(Token("break","\n",0,0));
	vector<string> temp;
	Parse_Token(fp,&token_list);
	if(error_list.size()==0){
		token_list=Fold(token_list);
	}
	Token token=token_list.back();
	token_list.push_back(Token("break","\n",token.line+1,0));
	if(error_list.size()==0){
		Grammar_check(token_list,&temp,new map<string,data>());
	}
	fclose(fp);
}

//

int main(int argc,char* argv[]){
	conv_bool.push_back("int");
	conv_bool.push_back("float");
	convert["bool"]=conv_bool;
	conv_int.push_back("bool");
	conv_int.push_back("float");
	convert["int"]=conv_int;
	conv_float.push_back("bool");
	conv_float.push_back("int");
	convert["float"]=conv_float;
	convert["string"]=vector<string>();
	FILE *fp=fopen(".\\log.txt","w");
	fputs("",fp);
	fclose(fp);
	for(int i=1;i<argc;i++){
		Compile_file(argv[i]);
		if(error_list.size()>0){
			cout<<"在"<<argv[i]<<"：\n";
			for(int i=0;i<error_list.size();i++){
				cout<<error_list[i]<<"\n";
			}
			error_list.clear();
		}
	}
	system("pause");
}
