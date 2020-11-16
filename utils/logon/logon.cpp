// QueryKey - Enumerates the subkeys of key and its associated values.
//     hKey - Key whose subkeys and values are to be enumerated.

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <typeinfo>
#include "fileDescription.h"
#include "transStr.h"

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

void QueryKey(HKEY hKey)
{
    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys = 0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 

    DWORD i, retCode;

    TCHAR  achValue[MAX_VALUE_NAME];
    BYTE  achData[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;
    DWORD cchData = MAX_VALUE_NAME;
    DWORD type;

    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 

    // Enumerate the key values. 
    if (cValues)
    {
        //printf("\nNumber of values: %d\n", cValues);

        for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
        {
            cchValue = MAX_VALUE_NAME;
            cchData = MAX_VALUE_NAME;
            achValue[0] = '\0';
            achData[0] = '\0';
            retCode = RegEnumValue(hKey, i,
                achValue,
                &cchValue,
                NULL,
                &type,
                achData,
                &cchData);

            if (retCode == ERROR_SUCCESS)
            {
                unsigned long j;

                // 获取注册表项的名称
                string name = wstring2string(achValue);

                // 获取键值，即可执行文件路径
                string image_path = "";
                // BYTE转换为string
                for (j = 0; j < cchData; j=j+2) {
                    image_path = image_path + (char)achData[j];
                }
                image_path = format_image_path(image_path);

                // 获取可执行文件描述 
                TCHAR* fileDescription = new TCHAR[1024];
                wstring str = string2wstring(image_path);
                BOOL bRet = GetFileVersionString(str.c_str(), _T("FileDescription"), fileDescription, 1024);
                string file_description = TCHAR2char(fileDescription);
                int n;
                if ((n = file_description.find("?")) != string::npos)
                    file_description = "";
                if (name == "" && file_description != "")
                    name = file_description;
                
                // 格式化输出
                string info = name;
                string split_sign("$#&");
                info.append(split_sign);
                info.append(file_description);
                info.append(split_sign);
                info.append(image_path);
                printf("%s\n", info.c_str());

                //printf("%s\n", file_description.c_str());
                //printf("%d %s\n", i + 1, val.c_str());
            }
        }
    }
}

int __cdecl _tmain(int argc, TCHAR* argv[])
{
    HKEY hTestKey;
    // 访问控制权限设置
    DWORD dwAccess = KEY_READ | KEY_WOW64_64KEY;

    if (argc != 3) {
        printf("Usage: logon.exe <root key>(HKLM/HKCU) <registry path>\n");
        return 0;
    }

    //printf("%s\n", typeid(argv[2]).name());
    string root = TCHAR2char(argv[1]);
    string reg_path = TCHAR2char(argv[2]);

    //printf("root key: %s\nregistry path: %s\n", root.c_str(), reg_path.c_str());

    // 输入的路径变量类型为LPCSTR
    wstring str = string2wstring(reg_path);

    if (root == "HKLM") {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, str.c_str(), 0, dwAccess, &hTestKey) == ERROR_SUCCESS)
            QueryKey(hTestKey);
    }
    else {
        if (RegOpenKeyEx(HKEY_CURRENT_USER, str.c_str(), 0, dwAccess, &hTestKey) == ERROR_SUCCESS)
            QueryKey(hTestKey);
    }
    

    RegCloseKey(hTestKey);
    return 1;
}