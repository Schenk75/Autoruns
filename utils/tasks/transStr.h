#include <string>
#include <string.h>
#include <tchar.h>
#include <windows.h>
using namespace std;

// 字符串类型TCHAR转为char*
char* TCHAR2char(const TCHAR* STR)
{
    //返回字符串的长度
    int size = WideCharToMultiByte(CP_ACP, 0, STR, -1, NULL, 0, NULL, FALSE);
    //申请一个多字节的字符串变量
    char* str = new char[sizeof(char) * size];
    //将TCHAR转成str
    WideCharToMultiByte(CP_ACP, 0, STR, -1, str, size, NULL, FALSE);
    return str;
}

// 字符串类型wchar转成string
string wchar2string(wchar_t* pWCStrKey)
{
    //第一次调用确认转换后单字节字符串的长度，用于开辟空间
    int pSize = WideCharToMultiByte(CP_OEMCP, 0, pWCStrKey, wcslen(pWCStrKey), NULL, 0, NULL, NULL);
    char* pCStrKey = new char[pSize + 1];
    //第二次调用将双字节字符串转换成单字节字符串
    WideCharToMultiByte(CP_OEMCP, 0, pWCStrKey, wcslen(pWCStrKey), pCStrKey, pSize, NULL, NULL);
    pCStrKey[pSize] = '\0';
    string pKey = pCStrKey;
    delete[] pCStrKey;
    return pKey;
}


// 字符串类型string转换成wstring  
wstring string2wstring(string str)
{
    wstring result;
    //获取缓冲区大小，并申请空间，缓冲区大小按字符计算  
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
    TCHAR* buffer = new TCHAR[len + 1];
    //多字节编码转换成宽字节编码  
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
    buffer[len] = '\0';             //添加字符串结尾  
    //删除缓冲区并返回值  
    result.append(buffer);
    delete[] buffer;
    return result;
}

// 标准化可执行文件路径
string format_image_path(string image_path) 
{
    int n;
    if ((n = image_path.find(" -")) != string::npos)    // 含有命令选项
        image_path = image_path.substr(0, n);
    if ((n = image_path.find(" /")) != string::npos)    // 含有命令选项
        image_path = image_path.substr(0, n);
    if ((n = image_path.find("\"")) != string::npos){    // 被双引号包裹的路径
        if (n == 0){
            image_path = image_path.substr(1);
            if ((n = image_path.find("\"")) != string::npos)
                image_path = image_path.substr(0, n);
        }
    }
    if ((n = image_path.find("system32")) != string::npos)   // 包含系统环境变量
        image_path = image_path.substr(n).insert(0, "C:\\Windows\\");
    if ((n = image_path.find("System32")) != string::npos)
        image_path = image_path.substr(n).insert(0, "C:\\Windows\\");
    // if (image_path.contains("syswow64", Qt::CaseInsensitive))
    //     image_path = QString("C:\\Windows\\SysWOW64") + image_path.split("syswow64", QString::KeepEmptyParts, Qt::CaseInsensitive)[1];
    // if (image_path.contains("\\??\\"))
    //     image_path = image_path.remove(image_path.indexOf("\\??\\"), 4);
    if ((n = image_path.find("%ProgramFiles%")) != string::npos)
        image_path = image_path.substr(14).insert(0, "C:\\Program Files");
    // if (image_path.contains("%programdata%", Qt::CaseInsensitive))
    //     image_path = image_path.replace(0, 13, "C:\\ProgramData");
    // if ((n = image_path.find("\%SystemRoot%")) != string::npos)
    //     image_path = image_path.replace(0, 12, "C:\\Windows");
    if ((n = image_path.find("\\")) == string::npos)
        image_path = image_path.insert(0, "C:\\Windows\\System32\\");
    return image_path;
}