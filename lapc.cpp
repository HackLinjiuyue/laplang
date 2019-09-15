#include<iostream>
#include<vector>
#include<map>
#include<sstream>
#include<stack>
#include<math.h>
#include<set>

enum LapType{
	Lint=0,
	Lfloat,
	Lstring,
	Lbool,
	Lobject,
	Lfile,
	Lhandle,
};

bool is_reflect=false;

using namespace std;
string last_face="";
map<string,string> TypeMap;
//定义部分
class var{
	public:
	int id;
	string type;
	var(int i,string t){
		id=i;
		type=t;
	}
};

vector<string> warn_list;

vector<int> if_stack;

const string Symbols[31]={")","(","+","-","*","/","%",">","<","=","!=",">=","<=","^","<<",">>","&&","||","{","}","==",".","!","//","/*","*/","[","]",",","&","|"};

const string KeyWords[17]={"interface","function","return","if","break","continue","when","import","else","local","global","set","call","delete","#path","#reflect","while"};

const string Basic_type[10]={"int","float","bool","string","void","Array","File","DLLHandle","Object","NativeFunction"};

const string Num[10]={"0","1","2","3","4","5","6","7","8","9"};

const string Bool[2]={"true","false"};

const string Logic[]={"&&","||"};

const string l1[] = { "+" , "-" };
const string l2[] = { "*","/","<<",">>","%","~"};
const string l3[] = { "^","&","|"};
const string l4[] = { "||","&&" };
const string l5[] = { "==",">","<",">=","<=","!=","!"};
const string l6[] = { "@" ,"."};

const string ct[4]={"int","float","bool","string"};

vector<string> error_list;

set<string> depends;

vector<int> last_p;

string c_path;

string ftype="";

bool bk=0,is_std=true;

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

class function{
public:
	int line;
	bool is_interface;
	bool is_local;
	bool is_builtin;
	string type;
	vector<var> args;
	function(int l,bool i,string t,vector<var> a,bool is_l=false,bool b=false){
		line=l;
		is_interface=i;
		type=t;
		args=a;
		is_local=is_l;
		is_builtin=b;
	};
	function(){
	}
};

vector<map<string,var> > domains;

vector<int> var_num;

map<string,function> functions;


int digui=0,ceng=-1,reg_max,var_count=0,label_count=0,size_add=0;

int ssize=0;

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
	return Tostring(line)+":"+Tostring(byte);
}

void Parse_Token(FILE *fp,vector<Token> *token_list){
	string onstr(""),onget(" "),linshi,last_type,last_op("");
	char last_get;
	int line=1,byte=0,small_count;
	bool is_func=false,is_float=false;
	Token token;
	while(true){
		onget[0]=fgetc(fp);
		if(onget[0]==EOF){
			if(onstr.length()>0){
				token_list->push_back(Token(last_type,onstr,line,byte));
			}
			break;
		}
		byte++;
		if(onget[0]=='\t'){
			token_list->push_back(Token("","\t",line,byte));
			continue;
		}
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
		else if(Is_in_s(KeyWords,&onstr,17)){
			last_type="keyword";
			if(onstr=="function"){
				is_func=true;
			}
		}
		else if(Is_in_s(Bool,&onstr,2)){
			last_type="bool";
		}
		if(onget[0]=='\n'){
			if(onstr.length()>0){
				if(onstr[0]=='"'){
					last_type="string";
				}
				else{
					bool temp=true;
					for(int i=0;i<onstr.length();i++){
						if(!Is_in_s(Num,&onstr,10)&&onstr[0]!='.'){
							temp=false;
							break;
						}
						if(onstr[0]=='.'){
							is_float=true;
						}
					}
					if(temp){
						if(is_float){
							last_type="float";
						}
						else{
							last_type="int";
						}
					}
					is_float=false;
				}
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
		else if(Is_in_s(Symbols,&onget,31)){
			if(last_type!="op"&&onstr.length()>0){
				token_list->push_back(Token(last_type,onstr,line,byte));
				onstr="";
			}
			linshi=last_op+onget[0];
			last_op=onget;
			if(Is_in_s(Symbols,&linshi,31)&&linshi.size()>1){
				if(last_type=="op"){
					token_list->pop_back();
				}
				if(linshi!="//"){
					token_list->push_back(Token("op",linshi,line,byte));
				}
			}
			else{
				if(onget[0]=='('&&token_list->back().Type=="var"){
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
			onstr="\"";
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
			token_list->push_back(Token("string",onstr+"\"",line,byte));
			onstr="";
		}
		else{
			last_type="var";
			onstr+=onget[0];
		}
		if(error_list.size()>0){
			return;
		}
	}
}



int op_Level(const string op)
{
	if(Is_in_s(l1,&op,2))
		return 1;
	if (Is_in_s(l2, &op, 6))
		return 2;
	if (Is_in_s(l3, &op, 3))
		return 3;
	if (Is_in_s(l4, &op, 2))
		return 4;
	if (Is_in_s(l5, &op, 7))
		return 5;
	if (Is_in_s(l6, &op, 2))
		return 7;
	return -1;

}

vector<Token> Trans_exp(vector<Token>& token_list)//by奔跑的小蜗牛 优化和bug修复 魔凤啸天(hacklinjiuyue)
{
	stack<Token> s;
	vector<Token> out;
	int cur = 0,size = token_list.size();
	while (cur < size)
	{
		//cout<<token_list[cur].Type<<" "<<token_list[cur].Value<<endl;
		if (token_list[cur].Value == "(")
		{
			s.push(token_list[cur]);
		}

		else if (token_list[cur].Value == ")") //弹出内容直到遇到第一个左括号
		{
			while (!s.empty())
			{
				if(s.top().Value == "("){
					break;
				}
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
	//cout<<endl;
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
		//cout<<token.Type<<" "<<token.Value<<endl;
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
			i++;
			temp.push_back(token);
			line=token.line;
			byte=token.byte;
			kh=1;
			arg=new vector<Token>();
			for(;i<token_list.size();i++){
				token=token_list[i];
				if(token.Value=="("){
					kh++;
				}
				else if(token.Value==")"){
					kh--;
				}
				if(kh==0){
					break;
				}
				arg->push_back(token);
			}
			if(kh>0){
				error_list.push_back("错误："+Position(line,byte)+" 表达式缺少')'");
				break;
			}
			*arg=Fold(*arg);
			for(int c=0;c<arg->size();c++){
				temp.push_back((*arg)[c]);
			}
			temp.push_back(Token("op",")",token.line,token.byte));
			delete arg;
		}
		else if(token.Value=="["){
			kh=1;
			arg=new vector<Token>();
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
					break;
				}
				arg->push_back(token);
			}
			if(kh>0){
				error_list.push_back("错误："+Position(line,byte)+" 数组下标缺少']'");
				break;
			}
			if(arg->size()==0){
				error_list.push_back("错误："+Position(line,byte)+" 数组下标缺少值");
				break;
			}
			temp.push_back(Token("op","@",line,byte,Fold(*arg)));
			delete arg;
		}
		else{
			if(i>0&&i<token_list.size()-1){
				if(token_list[i-1].Type=="int"&&token.Value=="."&&token_list[i+1].Type=="int"){
					temp.pop_back();
					token.Type="float";
					token.Value=token_list[i-1].Value+"."+token_list[i+1].Value;
					i++;
				}
			}
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
	case 3:
		temp="bool";
	break;
	case 2:
		temp="str";
	break;
	}
	return "add_const_"+temp;
}

string Parse_exp(vector<Token> &exp,bool is_set,map<string,var> &domain,bool is_global,bool is_call=false){
	vector<Token> s;
	map<string,function>::iterator Fiter;
	int i=0,max=exp.size(),x,m;
	Token op1,op2,op;
	string next_type,is_ref;
	map<string,var>::iterator iter;
	int size;
	for(;i<max;i++){
		op=exp[i];
		if(op.Type==""){
			break;
		}
		if(op.Type=="tab"){
			continue;
		}
		if(op.Type=="op"){
			if(s.empty()){
				error_list.push_back("错误："+Position(op.line,op.byte)+" 符号'"+op.Value+"'语法错误");
				return "";
			}
			op2=s.back();
			s.pop_back();
			if(op.Value=="!"){
				next_type=op2.Type;
				out.push_back(Ins("not"));
			}
			else if(op.Value=="@"){
				op.sub=Trans_exp(op.sub);
				if(Parse_exp(op.sub,is_set,domain,is_global)!="int"){
					error_list.push_back("错误："+Position(op.line,op.byte)+" 数组下标必须为整数");
					return "";
				}
				out.push_back(Ins("index"));
				next_type="Object";
			}
			else{
				if(s.empty()){
					error_list.push_back("错误："+Position(op.line,op.byte)+" 符号'"+op.Value+"'语法错误");
					return "";
				}
				op1=s.back();
				next_type=op1.Type;
				s.pop_back();
				if(op1.Type!=op2.Type&&op1.Type!="Object"&&op2.Type!="Object"){
					error_list.push_back("错误："+Position(op.line,op.byte)+" 运算类型不符 "+op1.Type+"/"+op2.Type);
					return "";
				}
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
				else if(op.Value==">>"){
					out.push_back(Ins("move_right"));
				}
				else if(op.Value=="<<"){
					out.push_back(Ins("move_left"));
				}
				else if(op.Value=="^"){
					out.push_back(Ins("xor"));
				}
				else if(op.Value=="&"){
					out.push_back(Ins("bit_and"));
				}
				else if(op.Value=="|"){
					out.push_back(Ins("bit_or"));
				}
				else if(op.Value=="~"){
					out.push_back(Ins("ops"));
				}
				else if(op.Value==">"){
					out.push_back(Ins("bigger"));
					next_type="bool";
				}
				else if(op.Value=="<"){
					out.push_back(Ins("smaller"));
					next_type="bool";
				}
				else if(op.Value=="=="){
					out.push_back(Ins("equal"));
					next_type="bool";
				}
				else if(op.Value=="&&"){
					out.push_back(Ins("and"));
					next_type="bool";
				}
				else if(op.Value=="||"){
					out.push_back(Ins("or"));
					next_type="bool";
				}
				else if(op.Value==">="){
					out.push_back(Ins("smaller"));
					out.push_back(Ins("not"));
					next_type="bool";
				}
				else if(op.Value=="<="){
					out.push_back(Ins("bigger"));
					out.push_back(Ins("not"));
					next_type="bool";
				}
				else if(op.Value=="!="){
					out.push_back(Ins("equal"));
					out.push_back(Ins("not"));
					next_type="bool";
				}
				else if(op.Value=="."){
					//
				}
			}
			s.push_back(Token(next_type,"",op2.line,op2.byte,vector<Token>()));
		}
		else{
			if(op.Type=="var"){
				if(op.Value=="null"){
					s.push_back(Token("Object","null",op.line,op.byte));
					out.push_back(Ins("push_null"));
				}
				else{
					iter=domain.find(op.Value);
					if(iter==domain.end()){
						error_list.push_back("错误："+Position(op.line,op.byte)+" 变量'"+op.Value+"'未在作用域中定义");
						return "";
					}
					else{
						if(is_set){
							is_ref="true";
						}
						else{
							is_ref="f";
						}
						if(iter->second.type=="Array"||iter->second.type=="File"){
							is_ref="true";
						}
						if(i<exp.size()-1){
							if(!(exp[i+1].Value=="."&&exp[i+1].Type=="op")){
								op.Type=iter->second.type;
								if(is_global){
									out.push_back(Ins("push_var_global",Tostring(iter->second.id)));
								}
								else{
									out.push_back(Ins("push_var_local",Tostring(iter->second.id)));
								}
							}
						}
						else{
							if(is_global){
								out.push_back(Ins("push_var_global",Tostring(iter->second.id)));
							}
							else{
								out.push_back(Ins("push_var_local",Tostring(iter->second.id)));
							}
						}
						is_ref="false";
					}
					s.push_back(Token(iter->second.type,op.Value,op.line,op.byte));
				}
			}
			else if(op.Type=="callbox"){//动参不进行安全检查
				Fiter=functions.find(op.Value);
				bool arr_push=false;
				int over=99999;
				if(Fiter==functions.end()){
					error_list.push_back("错误："+Position(op.line,op.byte)+" 函数'"+op.Value+"'未定义");
					return "";
				}
				if(!is_call){
					if(Fiter->second.type=="void"){
						error_list.push_back("错误："+Position(op.line,op.byte)+" 'void'类型的函数'"+op.Value+"'不能参与计算");
						return "";
					}
				}
				size=op.sub.size();
				if(op.Value=="PushValue"){
					over=1;
					if(size<2){
						error_list.push_back("错误："+Position(op.line,op.byte)+" 函数'"+op.Value+"'调用所给定的参数数量与定义的不匹配 "+Tostring(size)+"/"+Tostring(Fiter->second.args.size()));
						return "";
					}
					arr_push=true;
				}
				else if(op.Value=="Reflect_CallFunction"){
					if(!is_reflect){
						error_list.push_back("错误："+Position(op.line,op.byte)+" 函数'"+op.Value+"'不能在未启用反射的情况下使用");
						return "";
					}
					over=0;
					arr_push=true;
					if(!size){
						error_list.push_back("错误："+Position(op.line,op.byte)+" 函数'"+op.Value+"'调用所给定的参数数量与定义的不匹配 "+Tostring(size)+"/"+Tostring(Fiter->second.args.size()));
						return "";
					}
				}
				else if(Fiter->second.args.size()!=size){
					error_list.push_back("错误："+Position(op.line,op.byte)+" 函数'"+op.Value+"'调用所给定的参数数量与定义的不匹配 "+Tostring(size)+"/"+Tostring(Fiter->second.args.size()));
					return "";
				}
				for(int v=0;v<size;v++){
					op.sub[v].sub=Trans_exp(op.sub[v].sub);
					next_type=Parse_exp(op.sub[v].sub,is_set,domain,is_global);
					if(!error_list.empty()){
						return "";
					}
					if(arr_push&&v>over){
						continue;
					}
					if(next_type!=Fiter->second.args[v].type&&Fiter->second.args[v].type!="Object"&&next_type!="Object"){
						error_list.push_back("错误："+Position(op.line,op.byte)+" 函数'"+op.Value+"'调用所给定的参数"+Tostring(v+1)+"类型与定义的不匹配 "+next_type+"/"+Fiter->second.args[v].type);
						return "";
					}
				}
				if(!error_list.empty()){
					return "";
				}
				if(!Fiter->second.is_interface){
					out.push_back(Ins("goto",Tostring(Fiter->second.line),Tostring(size)));
				}
				else{
					if(op.Value=="Len"||op.Value=="StrLen"){
						out.push_back(Ins("len"));
					}
					else if(op.Value=="File"){
						out.push_back(Ins("open_file"));
					}
					else if(op.Value=="Int"){
						out.push_back(Ins("int"));
					}
					else if(op.Value=="Type"){
						out.push_back(Ins("type"));
					}
					else if(op.Value=="Float"){
						out.push_back(Ins("float"));
					}
					else if(op.Value=="IsNull"){
						out.push_back(Ins("is_null"));
					}
					else if(op.Value=="Opposite"||op.Value=="负"){
						out.push_back(Ins("ops"));
					}
					else if(op.Value=="Fgetc"){
						out.push_back(Ins("fgetc"));
					}
					else if(op.Value=="Fclose"){
						out.push_back(Ins("close_file"));
					}
					else if(op.Value=="Fwrite"){
						out.push_back(Ins("fwrite"));
					}
					else if(op.Value=="Asc"){
						out.push_back(Ins("asc"));
					}
					else if(op.Value=="Inc"){
						out.push_back(Ins("inc"));
					}
					else if(op.Value=="Dec"){
						out.push_back(Ins("dec"));
					}
					else if(op.Value=="Print"){
						out.push_back(Ins("print"));
					}
					else if(op.Value=="Array"){
						out.push_back(Ins("push_arr"));
					}
					else if(op.Value=="FillArray"){
						out.push_back(Ins("arr_fill"));
					}
					else if(op.Value=="PushValue"){
						out.push_back(Ins("arr_push",Tostring(op.sub.size()-1)));
					}
					else if(op.Value=="Reflect_CallFunction"){
						out.push_back(Ins("ref_call",Tostring(op.sub.size()-1)));
					}
					else if(op.Value=="PopValue"){
						out.push_back(Ins("arr_pop"));
					}
					else if(op.Value=="InsertValue"){
						out.push_back(Ins("arr_insert"));
					}
					else if(op.Value=="RemoveValue"){
						out.push_back(Ins("arr_remove"));
					}
					else if(op.Value=="Argv"){
						out.push_back(Ins("get_command_arg"));
					}
					else if(op.Value=="Execute"){
						out.push_back(Ins("exec"));
					}
					else if(op.Value=="DLLOpen"){
						out.push_back(Ins("dlopen"));
					}
					else if(op.Value=="DLLClose"){
						out.push_back(Ins("dlclose"));
					}
					else if(op.Value=="SetArray"){
						out.push_back(Ins("set_index"));
					}
					else if(op.Value=="DLLGetFunction"){
						if(TypeMap.find(Fiter->second.type)==TypeMap.end()){
							out.push_back(Ins("dlsym","4"));
						}
						else{
							out.push_back(Ins("dlsym",TypeMap[Fiter->second.type]));
						}
					}
					else{
						if(Fiter->second.is_local){
							out.push_back(Ins("push_var_local",Tostring(Fiter->second.line)));
						}
						else{
							out.push_back(Ins("push_var_global",Tostring(Fiter->second.line)));
						}
						out.push_back(Ins("call_native",Tostring(Fiter->second.args.size())));
					}
				}
				s.push_back(Token(Fiter->second.type,"",op.line,op.byte));
			}
			else{
				x=0;
				m=consts.size();
				for(;x<m;x++){
					if(consts[x]==op){
						break;
					}
				}
				if(op.Value.length()>0){
					out.push_back(Ins("push_const",Tostring(x)));
				}
				else{
					if(op.Type=="string"){
					out.push_back(Ins("push_empty_str"));
					}
				}
				s.push_back(op);
			}
		}
	}
	if(s.size()>1){
		error_list.push_back("错误："+Position(op.line,op.byte)+" 表达式中缺少运算符");
		return "";
	}
	if(s[0].Type=="NativeFunction"&&exp[0].Value!="DLLGetFunction"){
		last_face=exp[0].Value;
	}
	return s[0].Type;
}

void Import(string path,bool builtin);

int Grammar_check(vector<Token> &tokens,bool innerFX=false,map<string,var> give=map<string,var>(),int start=0,int tab_num=0,int jumpto=0,bool inner_loop=false,bool is_builtin=false){
	ssize++;
	int i=start,posi,last_i,last_pos=999;
	int max=tokens.size();
	Token token,next,type,o;
	bool err=false,result;
	int tab=0,last_tab=0;
	int line,byte,m;
	map<string,var> domain=give,g;
	vector<var> farg;
	vector<Token> exp=vector<Token>();
	map<string,var>::iterator iter;
	string next_type,l,last_word,rt_type;
	vector<string> del_var;
	bool is_return=false;
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
			for(;m-1<tab;m++){
				if(innerFX){
					domains.push_back(give);
					var_num.push_back(give.size());
				}
				else{
					domains.push_back(domains[m-1]);
					var_num.push_back(0);
				}
			}
		}
		else{
			for(;tab<m-1;m--){
				domains.pop_back();
				var_num.pop_back();
			}
		}
		if(tab<=tab_num){
			if(bk){
				bk=i;
			}
			i-=tab;
			ssize--;
			return i;
		}
		token=tokens[i];
		if(token.Value=="\n"){
			continue;
		}
		line=token.line;
		byte=token.byte;
		if(token.Type=="keyword"){
			if(bk){
				i++;
				while(tokens[i].Type!="keyword"){
					if(tokens[i].Value[0]=='\n'){
							i--;
						break;
					}
					i++;
				}
				continue;
			}
			l=token.Value;
			if(token.Value=="set"){
                i++;
                token=tokens[i];
                iter=domains[0].find(token.Value);
                string t="",o_t="";
                if(iter==domains[0].end()){
					iter=domains[tab].find(token.Value);
					if(iter==domains[tab].end()){
						error_list.push_back("错误："+Position(line,byte)+" 变量'"+token.Value+"'未在此作用域中定义");
						return i;
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
						return max;
					}
					exp.push_back(token);
				}
				if(err){
					return max;
				}
				if(exp.size()==0){
					error_list.push_back("错误："+Position(line,byte)+" 表达式不能为空");
					return max;
				}
				exp=Trans_exp(exp);
				exp.push_back(Token());
				o_t=Parse_exp(exp,true,domains[tab],tab==0);
				if(!error_list.empty()){
					return max;
				}
				if(o_t!="Object"&&o_t!=iter->second.type&&iter->second.type!="Object"){
					error_list.push_back("错误："+Position(line,byte)+" 变量'"+iter->first+"'类型与表达式的类型不匹配 "+iter->second.type+"/"+o_t);
					return max;
				}
				out.push_back(Ins("set_var_"+t,Tostring(iter->second.id)));
			}
			else if(token.Value=="#reflect"){
				is_reflect=true;
			}
			else if(token.Value=="function"){
					del_var=vector<string>();
					if(ssize>1){
						error_list.push_back("错误："+Position(token.line,token.byte)+" 函数不允许定义在其他指令中");
						return i;
					}
					farg=vector<var>();
					if(innerFX){
						error_list.push_back("错误："+Position(token.line,token.byte)+" 函数不允许嵌套定义");
						return max;
					}
					i++;
					type=tokens[i];
					next_type=type.Value;
					if(!Is_in_s(Basic_type,&type.Value,10)){
						error_list.push_back("错误："+Position(type.line,type.byte)+" 类型'"+type.Value+"'无效");
						return max;
					}
					i++;
					next=tokens[i];
					if(Is_in_s(KeyWords,&next.Value,17)){
						error_list.push_back("错误："+Position(next.line,next.byte)+" 函数名不能为关键字");
						break;
					}
                    if(functions.find(next.Value)!=functions.end()){
						error_list.push_back("错误："+Position(next.line,next.byte)+" 函数'"+next.Value+"'不允许重复定义");
						break;
                    }
                    token=next;
                    int m=token.sub.size();
                    if(m>0){
						for(int f=0;f<m;f++){
							type=token.sub[f].sub[0];
							if(!Is_in_s(Basic_type,&type.Value,10)){
								error_list.push_back("错误："+Position(type.line,type.byte)+"类型'"+type.Value+"'无效");
								break;
							}
							o=token.sub[f].sub[1];
							if(o.Type!="var"){
								error_list.push_back("错误："+Position(o.line,o.byte)+"变量名'"+o.Value+"'无效");
								break;
							}
							if(give.find(o.Value)!=give.end()){
								error_list.push_back("错误："+Position(o.line,o.byte)+"变量名'"+o.Value+"'不能重复");
								break;
							}
							give.insert(pair<string,var>(o.Value,var(f,type.Value)));
							farg.push_back(var(f/2,type.Value));
							del_var.push_back(o.Value);
						}
						if(!error_list.empty()){
							break;
						}
                    }
                    functions.insert(pair<string,function>(token.Value,function(out.size()+2,false,next_type,farg)));
                    ftype=next_type;
                    i++;
                    out.push_back(Ins("jump"));
                    last_pos=out.size()-1;
                    last_i=i;
                    i=Grammar_check(tokens,true,give,i,0,jumpto,inner_loop)-1;
                    for(int b=0;b<del_var.size();b++){
						give.erase(del_var[b]);
                    }
                    ftype="";
                    if(!error_list.empty()){
						break;
                    }
                    if(i==last_i){
						error_list.push_back("错误："+Position(token.line,token.byte)+" 函数必须实现");
						break;
                    }
                    out[last_pos].arg1=Tostring(out.size()+1);
			}
			else if(token.Value=="interface"){
					if(ssize>1){
						error_list.push_back("错误："+Position(token.line,token.byte)+" 接口函数不允许定义在其他指令中");
						return i;
					}
					farg=vector<var>();
					g=map<string,var>();
					if(innerFX){
						error_list.push_back("错误："+Position(token.line,token.byte)+" 接口函数不允许嵌套定义");
						break;
					}
					i++;
					type=tokens[i];
					next_type=type.Value;
					if(!Is_in_s(Basic_type,&type.Value,10)){
						error_list.push_back("错误："+Position(type.line,type.byte)+" 类型'"+type.Value+"'无效");
						break;
					}
					i++;
					next=tokens[i];
					if(Is_in_s(KeyWords,&next.Value,17)){
						error_list.push_back("错误："+Position(next.line,next.byte)+" 接口函数名不能为关键字");
						break;
					}
                    if(functions.find(next.Value)!=functions.end()){
						error_list.push_back("错误："+Position(next.line,next.byte)+" 接口函数'"+next.Value+"'不允许重复定义");
						break;
                    }
                    token=next;
                    m=token.sub.size();
                    if(m>0){
						for(int f=0;f<m;f++){
							if(token.sub[f].sub.size()!=2){
								error_list.push_back("错误："+Position(next.line,next.byte)+" 接口函数'"+next.Value+"'参数定义缺少项");
								break;
							}
							type=token.sub[f].sub[0];
							if(!Is_in_s(Basic_type,&type.Value,10)){
								error_list.push_back("错误："+Position(type.line,type.byte)+"类型'"+type.Value+"'无效");
								break;
							}
							o=token.sub[f].sub[1];
							if(o.Type!="var"){
								error_list.push_back("错误："+Position(o.line,o.byte)+"变量名'"+o.Value+"'无效");
								break;
							}
							if(g.find(o.Value)!=g.end()){
								error_list.push_back("错误："+Position(o.line,o.byte)+"变量名'"+o.Value+"'不能重复");
								break;
							}
							g.insert(pair<string,var>(o.Value,var(f/2,type.Value)));
							farg.push_back(var(f/2,type.Value));
						}
						if(!error_list.empty()){
							break;
						}
                    }
                    iter=domains[tab].find(token.Value);
                    if(iter!=domains[tab].end()){
						if(!tab){
							functions.insert(pair<string,function>(token.Value,function(iter->second.id,true,next_type,farg)));
						}
						else{
							functions.insert(pair<string,function>(token.Value,function(iter->second.id,true,next_type,farg,true)));
						}
                    }
                    else{
                    	iter=domain.find(token.Value);
						if(iter!=domain.end()){
							if(!tab){
								functions.insert(pair<string,function>(token.Value,function(iter->second.id,true,next_type,farg)));
							}
							else{
								functions.insert(pair<string,function>(token.Value,function(iter->second.id,true,next_type,farg)));
							}
						}
						else{
							if(is_std){
								functions.insert(pair<string,function>(token.Value,function(0,true,next_type,farg,false,true)));
							}
							else{
								error_list.push_back("错误："+Position(line,byte)+" 接口'"+token.Value+"'必须与变量绑定");
								break;
							}
						}
                    }
			}
			else if(token.Value=="call"){
                i++;
                exp=vector<Token>();
                exp.push_back(tokens[i]);
				Parse_exp(exp,false,domains[tab],tab==0,true);
				out.push_back(Ins("pop"));
                    if(!error_list.empty()){
						break;
                    }
			}
			else if(token.Value=="continue"){
				if(!inner_loop){
					error_list.push_back("错误："+Position(line,byte)+" 'continue'语句必须出现在循环体中");
					return i;
				}
				out.push_back(Ins("jump",Tostring(jumpto-1)));
			}
			else if(token.Value=="delete"){
				if(inner_loop){
					error_list.push_back("错误："+Position(line,byte)+" 'delete'语句不能在循环体中出现");
					return i;
				}
				i++;
				token=tokens[i];
				if(token.Type!="var"){
					error_list.push_back("错误："+Position(line,byte)+" 被删除的必须为变量");
					return i;
				}
				iter=domains[0].find(token.Value);
				if(iter==domains[0].end()){
					iter=give.find(token.Value);
					if(iter==give.end()){
						error_list.push_back("错误："+Position(line,byte)+" 变量'"+token.Value+"'未在子作用域中定义");
						return i;
					}
				}
				else if(tab){
					error_list.push_back("错误："+Position(line,byte)+" 全局变量'"+token.Value+"'不能在子作用域中被删除");
					return i;
				}
				out.push_back(Ins("delete",Tostring(iter->second.id)));
			}
			else if(token.Value=="return"){
				if(!tab){
					error_list.push_back("错误："+Position(line,byte)+" 'return'语句必须出现在函数中");
					break;
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
						return i;
					}
					exp.push_back(token);
				}
				if(err){
					return i;
				}
				if(exp.empty()&&ftype!="void"){
					error_list.push_back("错误："+Position(line,byte)+" 表达式不能为空");
					return i;
				}
				if(exp.size()!=0){
					if(ftype=="void"){
						error_list.push_back("错误："+Position(line,byte)+" 表达式必须为空");
						return i;
					}
					exp=Trans_exp(exp);
					exp.push_back(Token());
					Parse_exp(exp,false,domains[tab],tab==0);
					if(!error_list.empty()){
						return i;
					}
				}
				else{
					out.push_back(Ins("push_null"));
				}
				is_return=true;
				out.push_back(Ins("return"));
			}
			else if(token.Value=="if"){
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
                rt_type=Parse_exp(exp,false,domains[tab],tab==0);
                if(!error_list.empty()){
					return max;
                }
				if(rt_type!="bool"&&rt_type!="int"&&rt_type!="float"){
					error_list.push_back("错误："+Position(line,byte)+" 表达式类型必须为'bool'或'int'或'float' 当前类型："+exp[0].Type);
					break;
				}
                last_pos=out.size();
                out.push_back(Ins("false_jump"));
				i=Grammar_check(tokens,innerFX,domains[tab],i,tab,jumpto,inner_loop)-1;
                    if(!error_list.empty()){
						break;
                    }
					if(bk){
					bk=0;
					i--;
					last_p.push_back(out.size());
					out.push_back(Ins("jump"));
				}
				for(int f=i;f<max;f++){
					token=tokens[f];
					if(token.Type=="keyword"){
						break;
					}
				}
				if(token.Value=="else"){
					if_stack.push_back(out.size());
					out.push_back(Ins("jump"));
				}
				out[last_pos].arg1=Tostring(out.size()+1);
				last_pos=tab;
			}
			else if(token.Value=="else"){
				if(last_word!="if"||tab!=last_pos){
					error_list.push_back("错误："+Position(line,byte)+" 没有对应的if语句");
					break;
				}
				i++;
				i=Grammar_check(tokens,innerFX,domains[tab],i,tab,jumpto,inner_loop)-1;
                    if(!error_list.empty()){
						break;
                    }
				if(bk){
					bk=0;
					i--;
				}
				out[if_stack.back()].arg1=Tostring(out.size()+1);
				if_stack.pop_back();
			}
			else if(token.Value=="#path"){
				i++;
				token=tokens[i];
				if(token.Type!="string"){
					error_list.push_back("错误："+Position(token.line,token.byte)+" 路径必须为字符串");
					return max;
				}
				c_path=token.Value;
			}
			else if(token.Value=="when"||token.Value=="while"){
				int cha;
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
				cha=out.size();
                rt_type=Parse_exp(exp,false,domains[tab],tab==0);
                cha=out.size()-cha;
				if(rt_type!="bool"&&rt_type!="int"&&rt_type!="float"){
					error_list.push_back("错误："+Position(line,byte)+" 表达式类型必须为'bool'或'int'或'float' 当前类型："+exp[0].Type);
					break;
				}
                last_pos=out.size();
                out.push_back(Ins("false_jump"));
				i=Grammar_check(tokens,innerFX,domains[tab],i,tab,out.size()+1-cha,true)-1;
                    if(!error_list.empty()){
						break;
                    }
				Parse_exp(exp,false,domains[tab],tab==0);
				out.push_back(Ins("true_jump",Tostring(last_pos+2)));
				out[last_pos].arg1=Tostring(out.size()+1);
				if(bk){
					bk=0;
					i--;
				}
				while(last_p.size()){
					out[last_p.back()].arg1=Tostring(out.size()+1);
					last_p.pop_back();
				}
			}
			else if(token.Value=="break"){
				if(!inner_loop){
					error_list.push_back("错误："+Position(line,byte)+" 'break'语句必须出现在循环体中");
					return i;
				}
				bk=true;
			}
			else if(token.Value=="import"){
				if(ssize>1){
					error_list.push_back("错误："+Position(token.line,token.byte)+" 'import'语句必须位于全局作用域");
					return i;
				}
				i++;
				token=tokens[i];
				if(token.Type!="string"){
					error_list.push_back("错误："+Position(line,byte)+" 'import'语句必须使用字符串参数");
					return i;
				}
				ssize--;
				if(depends.find(token.Value)==depends.end()){
					Import(token.Value,"");
				}
				else{
					warn_list.push_back("警告："+Position(line,byte)+" 库'"+token.Value+"'已被引用");
				}
				ssize++;
				if(!error_list.empty()){
					return i;
				}
			}
			else if(!inner_loop){
				i++;
				type=tokens[i];
				if(!Is_in_s(Basic_type,&type.Value,10)){
					error_list.push_back("错误："+Position(type.line,type.byte)+" 类型'"+type.Value+"'无效");
					break;
				}
				i++;
				next=tokens[i];
				if(Is_in_s(KeyWords,&next.Value,17)){
					error_list.push_back("错误："+Position(next.line,next.byte)+" 变量名不能为关键字");
					break;
				}
				if(token.Value=="local"){
					iter=domains[tab].find(next.Value);
					posi=var_num[tab]++;
					domains[tab].insert(pair<string,var>(next.Value,var(posi,type.Value)));
					result=iter==domains[tab].end();
				}
				else if(!innerFX){
					iter=domains[0].find(next.Value);
					posi=var_num[0]++;
					domains[tab].insert(pair<string,var>(next.Value,var(posi,type.Value)));
					result=iter==domains[0].end();
				}
				else{
					error_list.push_back("错误："+Position(line,byte)+" 全局变量'"+next.Value+"'不能在函数中定义");
					break;
				}
				if(!result){
					error_list.push_back("错误："+Position(line,byte)+" 变量'"+next.Value+"'已在此作用域中定义");
					break;
				}
				i++;
				exp=vector<Token>();
				type.Type=token.Value;
				if(type.Value=="void"){
					error_list.push_back("错误："+Position(line,byte)+" 类型'"+type.Value+"'无效");
					break;
				}
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
				string o_t=Parse_exp(exp,false,domains[tab],tab==0);
				if(!error_list.empty()){
					break;
				}
				if(type.Value!="Object"&&type.Value!=o_t&&o_t!="Object"){
					error_list.push_back("错误："+Position(line,byte)+" 变量'"+next.Value+"'类型与表达式的类型不匹配 "+type.Value+"/"+o_t);
					break;
				}
				if(o_t=="NativeFunction"&&next.Value!=last_face&&last_face!=""){
					functions[next.Value]=functions[last_face];
					last_face="";
				}
				out.push_back(Ins("store_var_"+type.Type,Tostring(posi)));
			}
			else{
				error_list.push_back("错误："+Position(token.line,token.byte)+" 循环体中不能定义变量");
				break;
			}
			last_word=l;
		}
		else if(token.Type=="callbox"){
			exp=vector<Token>();
			exp.push_back(token);
			Parse_exp(exp,false,domains[tab],tab==0,true);
			if(!error_list.empty()){
				break;
			}
			out.push_back(Ins("pop"));
		}
		else{
			if(token.Value!="local"&&token.Value!="global"){
				error_list.push_back("错误："+Position(token.line,token.byte)+" 指令'"+token.Value+"'无效");
				break;
			}
		}
		last_tab=tab;
	}
	ssize--;
	if(innerFX&&tab==1&&!is_return){
		error_list.push_back("错误："+Position(token.line,token.byte)+" 函数必须拥有'return'语句");
	}
	return i;
}

void Compile_file(string File_name,char* temp_name,bool is_import=false,bool is_builtin=false){
	File_name=c_path+File_name;
	FILE *fp=fopen(File_name.c_str(),"r");
	if(fp==NULL){
		cout<<"错误：文件"<<File_name<<"不存在\n";
		return;
	}
	vector<Token> token_list;
	vector<string> temp;
	token_list.push_back(Token("break","\n",0,0));
	Parse_Token(fp,&token_list);
	fclose(fp);
	int m=token_list.size();
	Token token;
	for(int i=0;i<m;i++){
		token=token_list[i];
		if(token.Value[0]=='"'){
			token_list[i].Type="string";
			token.Type="string";
			token_list[i].Value=token.Value.substr(1,token.Value.length()-2);
			token.Value=token_list[i].Value;
		}
		else if(token.Value=="EOF"){
			token_list[i].Type="int";
			token.Type="int";
			token_list[i].Value=Tostring(EOF);
			token.Value=token_list[i].Value;
		}
		else if(token.Value=="pi"){
			token_list[i].Type="float";
			token.Type="float";
			token_list[i].Value="3.141592";
			token.Value="3.141592";
		}
		else{
			bool tmp=true;
			bool is_float=false;
			string onstr;
			for(int x=0;x<token.Value.length();x++){
				onstr=token.Value.substr(x,1);
				if(token.Value[x]=='.'){
					is_float=true;
				}
				else if(!Is_in_s(Num,&onstr,10)){
					tmp=false;
					break;
				}
			}
			if(tmp){
				if(is_float){
					token_list[i].Type="float";
					token.Type="float";
				}
				else{
					token_list[i].Type="int";
					token.Type="int";
				}
			}
		}
		if(Is_in_s(ct,&token.Type,4)){
			int x=0,max=consts.size();
			bool on=true;
			for(;x<max;x++){
				if(consts[x]==token){
					on=false;
					break;
				}
			}
			if(on){
				if(token.Value.length()>0&&token_list[i-1].Value!="import"&&token_list[i-1].Value!="#path"){
					if(token.Type=="float"&&token.Value=="."){
						continue;
					}
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
	}
	if(error_list.empty()){
		token_list=Fold(token_list);
	}
	token=token_list.back();
	token_list.push_back(Token("break","\n",token.line+1,0));
	if(error_list.empty()){
		last_func=false;
		Grammar_check(token_list,false,map<string,var>(),0,-1,-1,false,is_builtin);
		//语法解析
	}
	if(error_list.empty()&&!is_import){
		fp=fopen(temp_name,"w+");
		for(int i=0;i<out.size();i++){
			fputs(out[i].encode().c_str(),fp);
		}
		fclose(fp);
	}
}

void Import(string path,bool builtin=false){
	depends.insert(path);
	path+=".lap";
	Compile_file(path,NULL,true,builtin);
	if(warn_list.size()>0){
		printf("在%s：\n",path.c_str());
		for(int i=0;i<warn_list.size();i++){
			printf("%s\n",warn_list[i].c_str());
			warn_list.pop_back();
		}
	}
	if(error_list.size()>0){
		printf("在%s：\n",path.c_str());
		for(int i=0;i<error_list.size();i++){
			printf("%s\n",error_list[i].c_str());
			error_list.pop_back();
		}
	}
}

//

int main(int argc,char* argv[]){
	if(argc>1){
		string on=string(argv[1]);
		if(on=="-h"){
			printf("Lapc by hacklinjiuyue v1.4a\n--------------------\n  lapc -h for help\n  lapc x.lap x.lapm to compile the file\n--------------------\n");
		}
		else{
			//string t,int c=-1,vector<var> arg=vector<var>(),vector<Token> i=vector<Token>(),vector<int> len=vector<int>(),int ID=0
			domains.push_back(map<string,var>());
			var_num.push_back(0);
			Import("./builtin",true);
			is_std=false;
			/*
			if(error_list.size()>0){
				printf("在%s：\n","./builtin.laph");
				for(int i=0;i<error_list.size();i++){
					printf("%s\n",error_list[i].c_str());
				}
				return -1;
			}*/
			TypeMap["int"]="0";
			TypeMap["float"]="1";
			TypeMap["string"]="2";
			TypeMap["bool"]="3";
			TypeMap["Array"]="4";
			TypeMap["File"]="5";
			TypeMap["DLLHandle"]="6";
			TypeMap["NativeFunction"]="7";
			Compile_file(argv[1],argv[2]);
			if(!warn_list.empty()){
				printf("在%s：\n",argv[1]);
				for(int i=0;i<warn_list.size();i++){
					printf("%s\n",warn_list[i].c_str());
				}
			}
			if(error_list.size()>0){
				printf("在%s：\n",argv[1]);
				for(int i=0;i<error_list.size();i++){
					printf("%s\n",error_list[i].c_str());
				}
			}
			else if(is_reflect){
				map<string,function>::iterator iter;
				string p=string(argv[2]);
				p+=".ref";
				FILE *fp=fopen(p.c_str(),"w+");
				for(iter=functions.begin();iter!=functions.end();iter++){
					if(iter->second.is_builtin){
						continue;
					}
					fputs(iter->first.c_str(),fp);
					fputs(" ",fp);
					if(iter->second.is_interface){
						fputs(Tostring(iter->second.line).c_str(),fp);
					}
					else{
						fputs(Tostring(iter->second.line-2).c_str(),fp);
					}
					fputs(" ",fp);
					fputs(Tostring(iter->second.args.size()).c_str(),fp);
					fputs(" ",fp);
					fputs(Tostring(iter->second.is_interface).c_str(),fp);
					fputs("\n",fp);
				}
				fclose(fp);
			}
			system("pause");
		}
	}
}
