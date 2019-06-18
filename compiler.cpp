#include<iostream>
#include<vector>
#include<map>
#include<algorithm>
#include<sstream>
#include<stack>
#include<math.h>

using namespace std;

//定义部分

const string Symbols[28]={")","(","+","-","*","/","%",">","<","=","!=",">=","<=","^","<<",">>","&&","||","[","]","{","}","==",",","!","//","/*","*/"};

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

vector<string> last_func_name,last_func_type;

int digui=0,ceng=-1;

bool last_func=false,inner_call=false,inner_list=false;

class Token{
public:
	string Type,Value;
	int line,byte;
	vector<Token> sub;
	bool is_return;
	Token(string type,string value,int Line,int b,vector<Token> Sub=vector<Token>(),bool is=false){
		Type=type;
		Value=value;
		line=Line;
		byte=b;
		sub=Sub;
		is_return=is;
	}
	Token(){
		Type="";
		Value="";
		is_return=false;
		line=0;
		byte=0;
		sub=vector<Token>();
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
	vector<int> arr_len;
	data(string t,int c=-1,vector<var> arg=vector<var>(),vector<Token> i=vector<Token>(),vector<int> len=vector<int>()){
		type=t;
		argc=c;
		args=arg;
		init=i;
		arr_len=len;
	}
	data(){
		argc=-1;
		type="";
		args=vector<var>();
		init=vector<Token>();
		arr_len=vector<int>();
	}
};

vector<var> *last_arg=NULL;

vector<int> empty_dep_list;

map<string,data> global;

Token Check_exp(vector<Token> &token_list,map<string,data> *domain);

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

int stoi(string &onstr){
	int temp=0,max=onstr.length();
	for(int i=0;i<max;i++){
		temp+=(onstr[max-i-1]-'0')*(int)pow(10,i);
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
	string onstr(""),onget(" "),linshi,last_type,last_op("");
	char last_get;
	int line=1,byte=0;
	bool is_func=false,is_float=false;
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
		else if(Is_in_s(Symbols,&onget,28)){
			if(last_type!="op"&&onstr.length()>0){
				token_list->push_back(Token(last_type,onstr,line,byte));
				onstr="";
			}
			linshi=last_op+onget[0];
			last_op=onget;
			if(Is_in_s(Symbols,&linshi,28)&&linshi.size()>1){
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
					onget[0]=fgetc(fp);
				}
				if(!error_list.empty()){
					break;
				}
				if(is_float){
					token_list->push_back(Token("float",onstr,line,byte,vector<Token>()));
				}
				else{
					token_list->push_back(Token("int",onstr,line,byte,vector<Token>()));
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

void Type_check_list(vector<Token> &on_list,string type,int *on_depth,int dest_depth,map<string,data> *domain,vector<int> &dep_list,string pos){
	int *depth=NULL,len;
	size_t size;
	Token token;
	string on_type;
	if(on_depth==NULL){
		depth=new int(0);
	}
	else{
		depth=on_depth;
	}
	if(*depth<dest_depth-1){
		on_type="list";
		len=dep_list[*depth];
	}
	else{
		len=dep_list[0];
		on_type=type;
	}
	size=on_list.size();
	if(len!=size){
		error_list.push_back("错误："+pos+" 静态数组初始化列表中成员列表长度与定义长度不相符 "+Position(size,len));
		return;
	}
	for(int i=0;i<on_list.size();i++){
		token.sub=Trans_exp(on_list[i].sub);
		inner_list=true;
		token=Check_exp(token.sub,domain);
		if(!error_list.empty()){
			break;
		}
		inner_list=false;
		if(token.Type!=on_type){
			cout<<token.Type<<on_type<<"\n";
			error_list.push_back("错误："+Position(token.line,token.byte)+" 静态数组初始化列表中成员类型不相符");
			break;
		}
		if(on_type=="list"){
			(*depth)++;
			Type_check_list(token.sub,type,depth,dest_depth,domain,dep_list,pos);
			(*depth)--;
		}
	}
	if(*depth==0){
		delete depth;
	}
}

Token Check_exp(vector<Token> &token_list,map<string,data> *domain){
	vector<Token> stack,trans_list;
	Token token,op1,op2;
	string op,type1="",type2="";
	map<string,data>::iterator iter;
	data on_data;
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
				if(op=="="){
					if(inner_call){
						error_list.push_back("错误："+Position(op2.line,op2.byte)+" 在函数列表中无法进行赋值");
						break;
					}
					if(op1.Type!="var"){
						error_list.push_back("错误："+Position(op2.line,op2.byte)+" 非变量无法进行赋值");
						break;
					}
					if(!iter->second.arr_len.empty()){
						if(type2!="list"){
							error_list.push_back("错误："+Position(op2.line,op2.byte)+" 变量'"+iter->first+"'作为静态数组赋值必须使用列表");
							break;
						}
						Type_check_list(op2.sub,type1,NULL,iter->second.arr_len.size(),domain,iter->second.arr_len,Position(op2.line,op2.byte));
						if(!error_list.empty()){
							break;
						}
					}
					else{
						if(type1!=type2&&!Check_convert(type2,type1)){
							error_list.push_back("错误："+Position(op2.line,op2.byte)+" 类型'"+type2+"'无法转换为'"+type1+"'类型进行'='运算");
							break;
						}
					}
				}
				else if(type1!=type2&&!Check_convert(type2,type1)){
					error_list.push_back("错误："+Position(op2.line,op2.byte)+" 类型'"+type2+"'无法转换为'"+type1+"'类型进行'"+op+"'运算");
					break;
				}
			}
			stack.push_back(Token(type1,"",op2.line,op2.byte,vector<Token>()));
		}
		else if(token.Type=="callbox"){
			iter=domain->find(token.Value);
			if(iter==domain->end()){
				iter=global.find(token.Value);
				if(iter==global.end()){
					error_list.push_back("错误："+Position(token.line,token.byte)+" 函数'"+token.Value+"'未在该作用域中定义，也不是一个全局函数");
					break;
				}
			}
			on_data=iter->second;
			if(on_data.argc!=token.sub.size()){
				error_list.push_back("错误："+Position(token.line,token.byte)+" 调用函数'"+iter->first+"'所需的参数个数不匹配");
				break;
			}
			inner_call=true;
			for(int i=0;i<on_data.argc;i++){
				trans_list=Trans_exp(token.sub[i].sub);
				op1=Check_exp(trans_list,domain);
				if(!error_list.empty()){
					break;
				}
				if(op1.Type!=on_data.args[i].type&&!Check_convert(op1.Type,on_data.args[i].type)){
					error_list.push_back("错误："+Position(token.line,token.byte)+" 调用函数'"+iter->first+"'时，参数"+Tostring(i+1)+"类型'"+op1.Type+"'与定义类型'"+on_data.args[i].type+"'不匹配");
					break;
				}
			}
			if(!error_list.empty()){
				break;
			}
			inner_call=false;
			stack.push_back(Token(on_data.type,"",token.line,token.byte,vector<Token>()));
		}
		else if(token.Type=="arg"){
			stack.push_back(Check_exp(token.sub,domain));
			if(!error_list.empty()){
				break;
			}
		}
		else{
			if(token.Type=="var"){
				if(domain->find(token.Value)==domain->end()&&global.find(token.Value)==global.end()){
					error_list.push_back("错误："+Position(token.line,token.byte)+" 变量'"+token.Value+"'未在该作用域中定义，也不是一个全局变量");
					break;
				}
			}
			stack.push_back(token);
		}
	}
	if(!error_list.empty()||stack.size()==0){
		return Token();
	}
	token=stack.back();
	if(stack.size()>1){
		error_list.push_back("错误："+Position(token.line,token.byte)+" 变量无法在没有运算符的情况下并列出现");
		return Token();
	}
	return token;
}

int Check_format(vector<Token> &token_list,int index,map<string,data> *domain,vector<string> *on_var){//无错返回目标index,否则返回-1
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
		last_func_type.push_back(token.Value);
		index++;
		token=token_list[index];
		if(global.find(token.Value)!=global.end()||domain->find(token.Value)!=domain->end()){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 变量'"+token.Value+"'被重复定义");
			return -1;
		}
		last_func_name.push_back(token.Value);
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
		domain->insert(pair<string,data>(last_func_name.back(),data(last_func_type.back(),args.size(),args)));
		last_func=true;
		on_var->push_back(last_func_name.back());
		last_arg=new vector<var>(args);
		digui++;
	}
	else if(token.Value=="while"||token.Value=="if"||token.Value=="elseif"){
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
	ceng++;
	string onstr,t;
	Token token,on_token;
	vector<Token> *on=NULL,s,*on_init=NULL;
	map<string,data> *local=NULL;
	vector<string> on_var;
	vector<int> *on_list=NULL;
	var x=var("","");
	bool last_return=false,is_return=false,is_arr=false,is_block_return=false;
	int equal_count,line=-1,byte=-1;
	if(last_func){
		last_func=false;
		local=new map<string,data>();
		local->insert(*domain->find(last_func_name.back()));
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
			if(token.Value=="return"){
				if(last_return){
					error_list.push_back("错误："+Position(token.line,token.byte)+" 函数返回值不能为空");
					break;
				}
				else if(digui==0){
					error_list.push_back("错误："+Position(token.line,token.byte)+" 非函数体内不能出现'return'");
					break;
				}
				last_return=true;
				is_return=true;
			}
			else if(token.Value=="if"||token.Value=="while"||token.Value=="else"||token.Value=="elseif"){
				if(last_return){
					error_list.push_back("错误："+Position(token.line,token.byte)+" '"+token.Value+"'后不能为空");
					break;
				}
				last_return=true;
			}
			i=Check_format(token_list,i,local,&on_var);
		}
		else{
			if(onstr=="type"){
				if(!Is_in_s(Basic_type,&token.Value,6)&&Class_type.find(token.Value)==Class_type.end()){
					error_list.push_back("错误："+Position(token.line,token.byte)+" '"+token.Value+"'不是一个有效的类型");
					break;
				}
				i++;
				t=token.Value;
				token=token_list[i];
				is_arr=false;
				if(token.Type=="list"){
					if(token.sub.empty()){
						error_list.push_back("错误："+Position(token.line,token.byte)+" 静态数组定义长度不能为空");
						break;
					}
					on_list=new vector<int>();
					for(int ii=0;ii<token.sub.size();ii++){
						on_token=token.sub[ii];
						if(on_token.sub.empty()){
							error_list.push_back("错误："+Position(on_token.line,on_token.byte)+" 静态数组定义长度不能为空");
							break;
						}
						on_token=on_token.sub[0];
						if(on_token.Type!="int"){
							error_list.push_back("错误："+Position(on_token.line,on_token.byte)+" 静态数组定义长度必须是整数常数");
							break;
						}
						on_list->push_back(stoi(on_token.Value));
					}
					is_arr=true;
					i++;
				}
				if(!error_list.empty()){
					break;
				}
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
						onstr=token.Value;
						if(global.find(onstr)!=global.end()||local->find(onstr)!=local->end()){
							error_list.push_back("错误："+Position(token.line,token.byte)+" '"+onstr+"'被重复定义");
							break;
						}
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
						on_init=new vector<Token>(Trans_exp(*on));
						delete on;
						if(is_arr){
							local->insert(pair<string,data>(onstr,data(t,-1,vector<var>(),*on_init,*on_list)));
						}
						else{
							local->insert(pair<string,data>(onstr,data(t,-1,vector<var>(),*on_init)));
						}
						Check_exp(*on_init,local);
						delete on_init;
						if(!error_list.empty()){
							break;
						}
						on_var.push_back(onstr);
					}
				}
				on_token=Token();
				if(is_arr){
					delete on_list;
				}
			}
			else if(onstr=="codebox"){
				if(token_list[i-1].Type=="var"){
					error_list.push_back("错误："+Position(token.line,token.byte)+" 错误的'{'位置");
					break;
				}
				if(!token.is_return&&!is_return){
					is_block_return=token.is_return;
					line=token.line;
					byte=token.byte;
				}
				Grammar_check(token.sub,temp,local);
				if(!error_list.empty()){
					break;
				}
				if(digui-1==ceng){
					last_func_type.pop_back();
					last_func_name.pop_back();
					digui--;
				}
			}
			else{
				on=new vector<Token>();
				equal_count=0;
				for(;i<token_list.size();i++){
					token=token_list[i];
					if(token.Value=="="){
						equal_count++;
					}
					if(equal_count>1){
						error_list.push_back("错误："+Position(token.line,token.byte)+" 表达式中不允许出现多个'='");
						break;
					}
					else if(token.Value==","||token.Type=="break"||token.Type=="codebox"){
						s.push_back(Token("exp","",token.line,token.byte,Trans_exp(*on)));
						onstr=token.Type;
						token=s.back();
						if(token.sub.size()>=1){
							if(equal_count==0&&!last_return&&token.sub[0].Type!="callbox"){
								error_list.push_back("错误："+Position(token.line,token.byte)+" 表达式无法作为单独的语句存在");
								break;
							}
							on_token=Check_exp(token.sub,local);
							if(!error_list.empty()){
								break;
							}
							if(is_return){
								onstr=last_func_type.back();
								if(on_token.Type!=onstr&&!Check_convert(onstr,on_token.Type)){
									error_list.push_back("错误："+Position(token.line,token.byte)+" 类型为'"+onstr+"'的函数无法返回类型'"+on_token.Type+"'");
									break;
								}
							}
						}
						else if(!token.sub.empty()){
							if(token.sub[0].Type!="callbox"&&!last_return){
								token=token.sub[0];
								error_list.push_back("错误："+Position(token.line,token.byte)+" 表达式无法作为单独的语句存在");
								break;
							}
						}
						else{
							if(is_return){
								if(on_token.Type==""&&last_func_type.back()!="void"){
									error_list.push_back("错误："+Position(token.line,token.byte)+" 函数返回值不能为空");
									break;
								}
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
						on->push_back(token);
					}
				}
				if(on!=NULL){
					delete on;
				}
			}
			last_return=false;
			is_return=false;
		}
		if(error_list.size()>0){
			break;
		}
	}
	if(local!=NULL&&last_func){
		delete local;
	}
	if(line!=-1&&!is_block_return){
		error_list.push_back("错误："+Position(line,byte)+" 语句块缺少'return'语句");
	}
	ceng--;
	for(int i=0;i<on_var.size();i++){
		local->erase(on_var[i]);
	}
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
						arg_list->push_back(Token("element","",token.line,token.byte,Fold(*arg)));
					}
					break;
				}
				else if(token.Value==","&&kh==1){
					arg_list->push_back(Token("element","",token.line,token.byte,Fold(*arg)));
					delete arg;
					arg=new vector<Token>();
				}
				else{
					arg->push_back(token);
				}
			}
			if(kh>0){
				error_list.push_back("错误："+Position(line,byte)+" 列表定义缺少']'");
				break;
			}
			temp.push_back(Token("list","",line,byte,*arg_list));
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
				if(token.Value=="return"){
					is_return=true;
				}
				else if(token.Value=="}"){
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

			temp.push_back(Token("codebox","",line,byte,Fold(*arg),is_return));
			is_return=false;
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
		last_func=false;
		Grammar_check(token_list,&temp,&global);
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
	convert["void"]=vector<string>();
	for(int i=1;i<argc;i++){
		Compile_file(argv[i]);
		if(error_list.size()>0){
			printf("在%s：\n",argv[i]);
			for(int i=0;i<error_list.size();i++){
				printf("%s\n",error_list[i].c_str());
			}
			break;
		}
	}
	system("pause");
}
