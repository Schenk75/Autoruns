// QueryKey - Enumerates the subkeys of key and its associated values.
//     hKey - Key whose subkeys and values are to be enumerated.

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
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
    DWORD Type;

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
                &Type,
                achData,
                &cchData);

            if (retCode == ERROR_SUCCESS)
            {
                unsigned long j;
                
                // 获取注册表项的名称
                string name = wstring2string(achValue);

                // 获取动态链接库名
                string exec_name = "";
                for (j = 0; j < cchData; j = j + 2)
                    exec_name = exec_name + (char)achData[j];
                
                // 生成可执行路径
                string image_path1 = "C:\\Windows\\System32\\" + exec_name;
                string image_path2 = "C:\\Windows\\SysWOW64\\" + exec_name;

                // 获取描述
                TCHAR* fileDescription1 = new TCHAR[1024];
                TCHAR* fileDescription2 = new TCHAR[1024];
                wstring str1 = string2wstring(image_path1);
                wstring str2 = string2wstring(image_path2);
                BOOL bRet = GetFileVersionString(str1.c_str(), _T("FileDescription"), fileDescription1, 1024);
                bRet = GetFileVersionString(str2.c_str(), _T("FileDescription"), fileDescription2, 1024);
                string file_description1 = TCHAR2char(fileDescription1);
                string file_description2 = TCHAR2char(fileDescription2);
                int n;
                if ((n = file_description1.find("?")) != string::npos)
                    file_description1 = "";
                if ((n = file_description2.find("?")) != string::npos)
                    file_description2 = "";

                // 格式化输出
                string info1 = name;
                string split_sign("$#&");
                info1.append(split_sign);
                info1.append(file_description1);
                info1.append(split_sign);
                info1.append(image_path1);
                printf("%s\n", info1.c_str());

                string info2 = name;
                info2.append(split_sign);
                info2.append(file_description2);
                info2.append(split_sign);
                info2.append(image_path2);
                printf("%s\n", info2.c_str());
            }
        }
    }
}

int __cdecl _tmain()
{
    HKEY hTestKey;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs"),
        0,
        KEY_READ,
        &hTestKey) == ERROR_SUCCESS
        )
    {
        QueryKey(hTestKey);
    }

    RegCloseKey(hTestKey);
}