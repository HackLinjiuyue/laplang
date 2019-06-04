#include<iostream>
#include<vector>
#include<map>
#include<sstream>

using namespace std;

//定义部分

const string Symbols[25]={")","(","+","-","*","/","%",">","<","=","!=",">=","<=","^","<<",">>","&&","||","[","]","{","}","==",",","!"};

const string KeyWords[8]={"func","return","if","break","continue","while","for","class"};

const string Basic_type[5]={"int","float","double","bool","string"};

const string Num[10]={"0","1","2","3","4","5","6","7","8","9"};

const string Func_lack_msg[3]={"类型","变量名","'('"};

vector<string> Class_type,error_list;

stringstream stream;

map<string,string> global,local;

class Token{
public:
	string Type,Value;
	int line,byte;
	vector<Token> sub;
	Token(string type,string value,int Line,int b){
		Type=type;
		Value=value;
		line=Line;
		byte=b;
	}
	Token(){
		Type="";
		Value="";
	}
	string Tostring(){
		return Type+" "+Value;
	}
};

//

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

string Position(int line,int byte){
	return Tostring(line)+"."+Tostring(byte);
}

void Parse_Token(FILE *fp,vector<Token> *token_list){
	string onstr(""),onget(" "),linshi,last_type;
	char last_get;
	int line=1,byte=0;
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
		if(Is_in_s(KeyWords,&onstr,5)){
			last_type="keyword";
		}
		else if(Is_in_s(Basic_type,&onstr,5)){
			last_type="type";
		}
		if(onget[0]=='\n'){
			line++;
			byte=0;
			if(onstr.length()>0){
				token_list->push_back(Token(last_type,onstr,line,byte));
				onstr="";
			}
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
			if(Is_in_s(Symbols,&linshi,25)){
				token_list->push_back(Token("op",linshi,line,byte));
			}
			else{
				token_list->push_back(Token("op",onget,line,byte));
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
					onget[0]=fgetc(fp);
				}
				token_list->push_back(Token("num",onstr,line,byte));
				onstr="";
				if(onget[0]!=EOF){
					token_list->push_back(Token("op",onget,line,byte));
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
			token_list->push_back(Token("str",onstr,line,byte));
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

void Gnerate_exp(vector<Token> *on_list,vector<string> *temp){

}

int Check_format(vector<Token> &token_list,int index){//无错返回目标index,否则返回-1
	Token token=token_list[index];
	bool is_comma=false,is_right=false;
	int len,var_len=0,type_len=0;
	if(token.Value=="func"){
		len=token_list.size()-index-1;
		if(token_list.size()-index-1<3){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义缺少"+Func_lack_msg[len]);
			return -1;
		}
		index++;
		token=token_list[index];
		if(!Is_in_s(Basic_type,&token.Value,5)){
			error_list.push_back("错误："+Position(token.line,token.byte)+" '"+token.Value+"'不是一个有效的类型");
			return -1;
		}
		index++;
		token=token_list[index];
		if(global.find(token.Value)!=global.end()){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 变量'"+token.Value+"'被重复定义");
			return -1;
		}
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
				if(token.Type!="var"){
					error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义缺少变量名");
					break;
				}
			}
			else if(token.Value==","){
				is_comma=true;
			}
			else if(token.Type!="var"){
				error_list.push_back("错误："+Position(token.line,token.byte)+" 非变量不能出现在参数名集中");
				break;
			}
			else{
				var_len++;
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
		if(index==token_list.size()){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义的参数类型集缺少'<'");
			return -1;
		}
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
				if(token.Type!="type"){
					error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义的参数类型集缺少类型");
					break;
				}
			}
			else if(token.Value==","){
				is_comma=true;
			}
			else if(token.Type!="type"){
				error_list.push_back("错误："+Position(token.line,token.byte)+" 非类型不能出现在参数类型集中");
				break;
			}
			else{
				type_len++;
				if(!Is_in_s(Basic_type,&token.Value,5)){
					error_list.push_back("错误："+Position(token.line,token.byte)+" '"+token.Value+"'不是一个有效的类型");
				}
			}
		}
		if(error_list.size()>0){
			return -1;
		}
		if(!is_right){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义的参数类型集缺少'>'");
			return -1;
		}
		if(var_len!=type_len){
			error_list.push_back("错误："+Position(token.line,token.byte)+" 函数定义的参数个数与参数类型的个数不符 "+Position(var_len,type_len));
			return -1;
		}
	}
}

void Grammar_check(vector<Token> &token_list,vector<string> *temp){
	string onstr;
	Token token;
	vector<Token> on_list;
	for(int i=0;i<token_list.size();i++){
		token=token_list[i];
		onstr=token.Type;
		if(onstr=="keyword"){
			Check_format(token_list,i);
		}
		else{

		}
		if(error_list.size()>0){
			break;
		}
	}
}

void Compile_file(string File_name){
	FILE *fp=fopen(File_name.c_str(),"r");
	if(fp==NULL){
		cout<<"错误：文件"<<File_name<<"不存在\n";
		return;
	}
	vector<Token> token_list;
	vector<string> temp;
	Parse_Token(fp,&token_list);
	if(error_list.size()==0){
		Grammar_check(token_list,&temp);
	}
	fclose(fp);
}

//

int main(int argc,char* argv[]){
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
