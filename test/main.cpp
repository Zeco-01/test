#include<iostream>
#include <fstream>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <openssl/md5.h>
#include <string>
#include <vector>
#include<boost\regex.hpp>
#pragma comment(lib,"libcurl_a.lib")
#pragma comment(lib,"libeay32.lib")
#pragma comment(lib,"ssleay32.lib")
#pragma comment(lib,"libboost_regex-vc140-mt-gd-1_60.lib")
using namespace std;
static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)//curl写数据回调
{
	std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
	if (NULL == str || NULL == buffer)
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}
void Wchar_tToString(std::string& szDst, wchar_t *wchar)//unicode编码转换
{
	wchar_t * wText = wchar;
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);// WideCharToMultiByte的运用
	char *psText;  // psText为char*的临时数组，作为赋值给std::string的中间变量
	psText = new char[dwNum];
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);// WideCharToMultiByte的再次运用
	szDst = psText;// std::string赋值
	delete[]psText;// psText的清除
}
std::string string_To_UTF8(const std::string & str)
{
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
	ZeroMemory(pwBuf, nwLen * 2 + 2);

	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr(pBuf);

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;

	return retStr;
}
string& replace_all_distinct(string&   str, const   string&   old_value, const   string&   new_value)
{
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
		if ((pos = str.find(old_value, pos)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}
string simpleTranslate(string src,string f,string t) {
	CURL* curl;
	curl = curl_easy_init(); 
	CURLcode res;
	string last;
	if (curl) {
		char myurl[2000] = "http://api.fanyi.baidu.com/api/trans/vip/translate?";
		char *appid = "20160324000016450";    //replace myAppid with your own appid
		string q = string_To_UTF8(src);
		//const char *q = src.c_str();          //replace apple with your own text to be translate, ensure that the input text is encoded with UTF-8!
		const char *from = f.c_str();          //replace en with your own language type of input text
		const char *to = t.c_str();            //replace zh with your own language type of output text
		char salt[60];
		int a = rand();
		sprintf(salt, "%d", a);
		char *secret_key = "g8FcbxfFXoAJ8ZaNsWtQ";   //replace mySecretKey with your own mySecretKey
		char sign[800] = "";
		strcat(sign, appid);
		strcat(sign, q.c_str());
		strcat(sign, salt);
		strcat(sign, secret_key);
		string response;
		
		unsigned char md[16];
		int i;
		char tmp[3] = { '\0' }, buf[33] = { '\0' };
		//MD5((unsigned char *)sign, strlen((unsigned char *)sign), md);
		MD5((unsigned char *)sign, strlen(sign), md);
		for (i = 0; i < 16; i++) {
			sprintf(tmp, "%2.2x", md[i]);
			strcat(buf, tmp);
		}
		
		strcat(myurl, "appid=");
		strcat(myurl, appid);
		strcat(myurl, "&q=");
		strcat(myurl, q.c_str());
		strcat(myurl, "&from=");
		strcat(myurl, from);
		strcat(myurl, "&to=");
		strcat(myurl, to);
		strcat(myurl, "&salt=");
		strcat(myurl, salt);
		strcat(myurl, "&sign=");
		strcat(myurl, buf);
		string url(myurl);
		url = replace_all_distinct(url, " ", "%20");
		//设置访问的地址
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
		
		//cout << url << endl;
		res = curl_easy_perform(curl);

	//	cout << response << endl;


		/* Check for errors */
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		/* always cleanup */
		curl_easy_cleanup(curl);
		Json::Reader reader;
		Json::Value root;
		if (reader.parse(response, root)) {
			Json::Value r = root["trans_result"];
			Json::Value rr = r[0];
			string result = rr["dst"].asString();
			
			int len = strlen(result.c_str()) + 1;
			char outch[MAX_PATH];
			WCHAR * wChar = new WCHAR[len];
			wmemset(wChar, 0, len);
			MultiByteToWideChar(CP_UTF8, 0, result.c_str(), len, wChar, len);
			
			Wchar_tToString(last, wChar);
			
			delete[] wChar;

		}
		
	}
	return last;
}
std::string& trim(std::string &s)
{
	if (s.empty())
	{
		return s;
	}

	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}
unsigned short charCodeAt(string s, int index) {
	char temp = s.at(index);
	wchar_t wtemp = temp;
	unsigned short g = wtemp;
	return g;
}
string tk(string s) {
	string a = trim(s);
	int b = 403081;
	string Vb = "+-a^+6";
	string Ub = "+-3^+b+-f";
	string dd = ".";
	vector<unsigned short> d;
	int e = 0;
	for (int f = 0; f < a.length(); f++) {
		unsigned short g = charCodeAt(a, f);

	}
	return a;
}//未完成 计算tk 根据js代码翻译
void split(std::string& s, std::string& delim, std::vector< std::string >* ret)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != std::string::npos)
	{
		ret->push_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last>0)
	{
		ret->push_back(s.substr(last, index - last));
	}
}
string& trim2(string &s)
{
	if (s.empty())
	{
		return s;
	}
	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}
vector<string> multiTrans(string source, string from, string to) {
	vector<string> results;
	CURL* curl;
	curl = curl_easy_init();
	CURLcode res;
	std::cout << "multiTrans: " + source << endl;
	if (curl) {
		char myurl[2000] = "http://fanyi.baidu.com/multitransapi";
		string response;
		strcat(myurl, "?from=");
		strcat(myurl, from.c_str());
		strcat(myurl, "&to=");
		strcat(myurl, to.c_str());
		strcat(myurl, "&query=");
		string us = string_To_UTF8(source);
		strcat(myurl, us.c_str());
		string temp = simpleTranslate(source,from,to);		
		string u = string_To_UTF8(temp);
		strcat(myurl, "&raw_trans=");
		strcat(myurl, u.c_str());
		strcat(myurl, "&count=5");
		string url(myurl);
		//url = trim2(url);
		url = replace_all_distinct(url, " ", "%20");
		//设置访问的地址
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
		//cout << myurl << endl;
		res = curl_easy_perform(curl);
	//	cout << response << endl;


		/* Check for errors */
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		/* always cleanup */
		curl_easy_cleanup(curl);
		Json::Reader reader;
		Json::Value root;
		
		if (reader.parse(response, root)) {
			Json::Value r = root["data"];
			Json::Value rr = r["cands"];
			for (int i = 0; i < rr.size(); i++) {
				string last;
				Json::Value temp = rr[i];
				string result = temp.asString();
				int len = strlen(result.c_str()) + 1;
				char outch[MAX_PATH];
				WCHAR * wChar = new WCHAR[len];
				wmemset(wChar, 0, len);
				MultiByteToWideChar(CP_UTF8, 0, result.c_str(), len, wChar, len);

				Wchar_tToString(last, wChar);

				delete[] wChar;
				results.push_back(last);
			}

			
		}

	}
	return results;
}
int blockWordNum(string block) {
	int num = 0;
	for (int i = 0; i < block.length(); i++) {
		if (block[i] == '/')num++;
	}
	return num;
}
int main(void)
{
	ifstream ifs("source.txt");
	ofstream ofs("results.txt"); 
	char* buffer = new char[1024];
	boost::regex r1("\\s([a-zA-Z0-9$]|\\s|/)+/");
	boost::regex r2("[a-zA-Z0-9]+?/");
	boost::smatch mat;
	int i = 0;
	while (!ifs.eof()) {
		ifs.getline(buffer, 1024);
		string s(buffer);
		vector<string> blocks;
		string sentence = "";
			boost::sregex_iterator it(s.begin(), s.end(), r1);
			boost::sregex_iterator end;		
			for (; it != end; ++it) {
				string temp = (*it).str();
				if (blockWordNum(temp) > 1) {
					string block_new = "";
					boost::sregex_iterator it2(temp.begin(), temp.end(), r2);
					boost::sregex_iterator end2;
					for (; it2 != end2; ++it2) {
						block_new += (*it2).str().substr(0, (*it2).str().length()-1);
						block_new += " ";
					}
					blocks.push_back(block_new);
				}
				else {
					blocks.push_back(temp.substr(1,temp.length()-2));
				}
			}
			for (int i = 0; i < blocks.size(); i++) {
				//cout << blocks[i] << endl;
				sentence += blocks[i];
			
				sentence += " ";
			}
			std::cout << sentence << endl;
			ofs << sentence << endl;
			ofs << simpleTranslate(sentence, "en", "zh") << endl;
			for (int i = 0; i < blocks.size(); i++) {
				ofs << blocks[i] << ": ";
				vector<string> temp = multiTrans(blocks[i],"en","zh");
				for (int j = 0; j < temp.size(); j++) {
					ofs << temp[j] << " ";
				}
				ofs << endl;
			}
			i++;
		if (i == 100)break;
		std::cout << i << endl;
	}
	ifs.close();
	ofs.close();
	delete buffer;
	/*vector<string> s = multiTrans("from 1 to 5 HP ", "en", "zh");
	for (int i = 0; i < s.size(); i++) {
		cout << s[i] << endl;
	}*/
	return 0;
}
