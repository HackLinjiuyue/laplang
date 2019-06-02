#include<iostream>
#include<vector>
#include<set>
#include<sstream>

using namespace std;

//定义部分

const string Symbols[24]={")","(","+","-","*","/","%",">","<","=","!=",">=","<=","^","<<",">>","&&","||","[","]","{","}","==",","};

const string KeyWords[5]={"func","return","if","break","continue"};

const string Basic_type[5]={"int","float","double","bool","string"};

const string Num[10]={"0","1","2","3","4","5","6","7","8","9"};

vector<string> Class_type,error_list;

stringstream stream;

class Token{
	string Type,Value;
	public:
		Token(string type,string value){
			Type=type;
			Value=value;
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

void Parse_Token(FILE *fp,vector<Token> *token_list){
	string onstr(""),onget(" "),linshi,last_type;
	char last_get;
	int line=1,byte=0;
	while(onget[0]!=EOF){
		onget[0]=fgetc(fp);
		byte++;
		if(onget[0]==EOF){
			if(onstr.length()>0){
				token_list->push_back(Token(last_type,onstr));
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
				token_list->push_back(Token(last_type,onstr));
				onstr="";
			}
		}
		else if(onget[0]==' '){
			if(onstr.length()>0){
				token_list->push_back(Token(last_type,onstr));
				onstr="";
			}
		}
		else if(Is_in_s(Symbols,&onget,24)){
			if(last_type!="op"&&onstr.length()>0){
				token_list->push_back(Token(last_type,onstr));
				onstr="";
			}
			last_type="op";
			linshi=onstr+onget[0];
			if(Is_in_s(Symbols,&linshi,24)){
				token_list->push_back(Token("op",linshi));
			}
			else{
				token_list->push_back(Token("op",onget));
			}
		}
		else if(Is_in_s(Num,&onget,10)){
			if(onstr.length()>0){
				token_list->push_back(Token(last_type,onstr));
				onstr="";
			}
			onstr=onget;
			onget[0]=fgetc(fp);
			while(Is_in_s(Num,&onget,10)||onget[0]=='.'){
				onstr+=onget[0];
				onget[0]=fgetc(fp);
			}
			token_list->push_back(Token("num",onstr));
			onstr="";
			if(onget[0]!=EOF){
				token_list->push_back(Token("op",onget));
			}
		}
		else if(onget[0]=='"'){
			onstr="";
			onget[0]=fgetc(fp);
			while(onget[0]!='"'){
				if(onget[0]==';'||onget[0]=='\n'||onget[0]==EOF){
					error_list.push_back("错误："+Tostring(line)+"."+Tostring(byte)+" 字符串末尾缺少<\">");
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
			token_list->push_back(Token("str",onstr));
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

void Compile_file(string File_name){
	FILE *fp=fopen(File_name.c_str(),"r");
	if(fp==NULL){
		cout<<"错误：文件"<<File_name<<"不存在\n";
		return;
	}
	vector<Token> token_list;
	Parse_Token(fp,&token_list);
	if(error_list.size()==0){
		for(int i=0;i<token_list.size();i++){
			cout<<token_list[i].Tostring()<<"\n";
		}
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
