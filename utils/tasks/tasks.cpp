/********************************************************************
 This sample enumerates through the tasks on the local computer and
 displays their name and state.
********************************************************************/

#define _WIN32_DCOM

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <comdef.h>
#include <typeinfo>
#include <string>
#include <taskschd.h>
#include "transStr.h"
#include "fileDescription.h"

#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")

using namespace std;

void walkFolders(ITaskFolder* rootFolder, HRESULT hr);
void getTasks(ITaskFolder* rootFolder, HRESULT hr);
void walkFoldersFormat(ITaskFolder* rootFolder, HRESULT hr);
void getTasksFormat(ITaskFolder* rootFolder, HRESULT hr);

int __cdecl wmain()
{
    //  ------------------------------------------------------
    //  Initialize COM.
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        printf("\nCoInitializeEx failed: %x", hr);
        return 1;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL);

    if (FAILED(hr))
    {
        printf("\nCoInitializeSecurity failed: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Create an instance of the Task Service. 
    ITaskService* pService = NULL;
    hr = CoCreateInstance(CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void**)&pService);
    if (FAILED(hr))
    {
        printf("\nFailed to CoCreate an instance of the TaskService class: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(),
        _variant_t(), _variant_t());
    if (FAILED(hr))
    {
        printf("\nITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Get the pointer to the root task folder.
    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr))
    {
        printf("\nCannot get Root Folder pointer: %x", hr);
        return 1;
    }
    //walkFoldersFormat(pRootFolder, hr);
    //getTasksFormat(pRootFolder, hr);

    walkFolders(pRootFolder, hr);
    //getTasks(pRootFolder, hr);
    pRootFolder->Release();
    pService->Release();
    CoUninitialize();
    //system("pause");
    return 0;
}

// Recursive search sub-folders - print format
void walkFoldersFormat(ITaskFolder* rootFolder, HRESULT hr) 
{
    ITaskFolderCollection* pFolders = NULL;
    hr = rootFolder->GetFolders(0, &pFolders);
    getTasksFormat(rootFolder, hr);
    if (FAILED(hr))
    {
        printf("\nCannot get Folders: %x", hr);
        return;
    }

    LONG numFolders = 0;
    hr = pFolders->get_Count(&numFolders);
    //printf("Number of Folders:%d", numFolders);

    if (numFolders != 0) {
        for (LONG i = 0; i < numFolders; i++) {
            ITaskFolder* pRootFolder = NULL;
            hr = pFolders->get_Item(_variant_t(i + 1), &pRootFolder);
            if (SUCCEEDED(hr)) {
                BSTR name = NULL;
                hr = pRootFolder->get_Name(&name);
                if (FAILED(hr))
                {
                    printf("\nCannot get Folder name: %x", hr);
                    return;
                }
                printf("\n\nFolder Name: %S", name);
                SysFreeString(name);
                walkFoldersFormat(pRootFolder, hr);
            }
            else
                printf("\n\tCannot get the folder name: %x", hr);
        }
        pFolders->Release();
    }
}

// Get the registered tasks in the folder - print format
void getTasksFormat(ITaskFolder* rootFolder, HRESULT hr) 
{
    IRegisteredTaskCollection* pTaskCollection = NULL;
    hr = rootFolder->GetTasks(NULL, &pTaskCollection);

    if (FAILED(hr))
    {
        printf("\n\tCannot get the registered tasks.: %x", hr);
        return;
    }

    LONG numTasks = 0;
    hr = pTaskCollection->get_Count(&numTasks);

    if (numTasks == 0)
    {
        printf("\n\tNo Tasks are currently running");
        pTaskCollection->Release();
        return;
    }

    //printf("\nNumber of Tasks : %d", numTasks);

    TASK_STATE taskState;

    for (LONG i = 0; i < numTasks; i++)
    {
        IRegisteredTask* pRegisteredTask = NULL;
        hr = pTaskCollection->get_Item(_variant_t(i + 1), &pRegisteredTask);

        if (SUCCEEDED(hr))
        {
            BSTR taskName = NULL;
            hr = pRegisteredTask->get_Name(&taskName);
            if (SUCCEEDED(hr))
            {
                printf("\n\tTaskName: %S", taskName);
                SysFreeString(taskName);

                hr = pRegisteredTask->get_State(&taskState);
                if (SUCCEEDED(hr))
                    printf("\n\tState: %d", taskState);
                else
                    printf("\n\tCannot get the registered task state: %x", hr);

                ITaskDefinition* taskDefination = NULL;
                hr = pRegisteredTask->get_Definition(&taskDefination);
                if (FAILED(hr))
                {
                    printf("\n\tCannot get the task defination: %x", hr);
                    return;
                }

                IActionCollection* taskActions = NULL;
                hr = taskDefination->get_Actions(&taskActions);
                if (FAILED(hr))
                {
                    printf("\n\tCannot get the task actions: %x", hr);
                    return;
                }
                taskDefination->Release();

                /*LONG numActions = 0;
                hr = taskActions->get_Count(&numActions);
                if (SUCCEEDED(hr))
                    printf("\n\tCount of Actions: %d", numActions);
                else
                    printf("\n\tCannot get the number of actions: %x", hr);*/

                IAction* action = NULL;
                hr = taskActions->get_Item(1, &action);
                if (FAILED(hr))
                {
                    printf("\n\tCannot get the action: %x", hr);
                    return;
                }
                taskActions->Release();

                IExecAction* execAction = NULL;
                hr = action->QueryInterface(IID_IExecAction, (void**)&execAction);
                if (FAILED(hr))
                {
                    printf("\n\tQueryInterface call failed for IExecAction: %x", hr);
                    return;
                }
                action->Release();

                BSTR imagePath = NULL;
                hr = execAction->get_Path(&imagePath);
                if (SUCCEEDED(hr))
                    printf("\n\tImage Path: %S", imagePath);
                else
                    printf("\n\tCannot get the image path: %x", hr);
                execAction->Release();
            }
            else
            {
                printf("\n\tCannot get the registered task name: %x", hr);
            }
            pRegisteredTask->Release();
        }
        else
        {
            printf("\n\tCannot get the registered task item at index=%d: %x", i + 1, hr);
        }
    }
    pTaskCollection->Release();
}

// Recursive search sub-folders
void walkFolders(ITaskFolder* rootFolder, HRESULT hr) 
{
    ITaskFolderCollection* pFolders = NULL;
    hr = rootFolder->GetFolders(0, &pFolders);
    getTasks(rootFolder, hr);
    if (FAILED(hr))
    {
        return;
    }

    LONG numFolders = 0;
    hr = pFolders->get_Count(&numFolders);
    //printf("Number of Folders:%d", numFolders);

    if (numFolders != 0) {
        for (LONG i = 0; i < numFolders; i++) {
            ITaskFolder* pRootFolder = NULL;
            hr = pFolders->get_Item(_variant_t(i + 1), &pRootFolder);
            if (SUCCEEDED(hr)) {
                //BSTR name = NULL;
                //hr = pRootFolder->get_Name(&name);
                //if (FAILED(hr))
                //{
                //    return;
                //}
                //printf("\n\nFolder Name: %S", name);
                //SysFreeString(name);
                walkFolders(pRootFolder, hr);
            }
        }
        pFolders->Release();
    }
}

// Get the registered tasks in the folder
void getTasks(ITaskFolder* rootFolder, HRESULT hr) 
{
    IRegisteredTaskCollection* pTaskCollection = NULL;
    hr = rootFolder->GetTasks(NULL, &pTaskCollection);

    if (FAILED(hr))
        return;

    LONG numTasks = 0;
    hr = pTaskCollection->get_Count(&numTasks);

    if (numTasks == 0) {
        //printf("\n\tNo Tasks are currently running");
        pTaskCollection->Release();
        return;
    }

    //printf("\nNumber of Tasks : %d", numTasks);

    // TASK_STATE taskState;

    for (LONG i = 0; i < numTasks; i++) {
        IRegisteredTask* pRegisteredTask = NULL;
        hr = pTaskCollection->get_Item(_variant_t(i + 1), &pRegisteredTask);

        if (SUCCEEDED(hr))
        {
            BSTR taskName = NULL;
            hr = pRegisteredTask->get_Name(&taskName);
            if (SUCCEEDED(hr))
            {
                // hr = pRegisteredTask->get_State(&taskState);
                // if (SUCCEEDED(hr))
                //     printf("%d$#^", taskState);
                // else
                //     printf("$#^");

                ITaskDefinition* taskDefination = NULL;
                hr = pRegisteredTask->get_Definition(&taskDefination);
                if (FAILED(hr))
                    continue;

                IActionCollection* taskActions = NULL;
                hr = taskDefination->get_Actions(&taskActions);
                if (FAILED(hr))
                    continue;
                taskDefination->Release();

                // LONG numActions = 0;
                // hr = taskActions->get_Count(&numActions);
                // if (SUCCEEDED(hr))
                //     printf("\n\tCount of Actions: %d", numActions);
                // else
                //     printf("\n\tCannot get the number of actions: %x", hr);

                IAction* action = NULL;
                hr = taskActions->get_Item(1, &action);
                if (FAILED(hr))
                    continue;
                taskActions->Release();

                IExecAction* execAction = NULL;
                hr = action->QueryInterface(IID_IExecAction, (void**)&execAction);
                if (FAILED(hr))
                    continue;
                action->Release();

                BSTR imagePath = NULL;
                hr = execAction->get_Path(&imagePath);
                if (SUCCEEDED(hr)){
                    // printf("\n%s", typeid(imagePath).name());  // 查看变量类型
                    string image_path = wchar2string(imagePath);
                    SysFreeString(imagePath);
                    string task_name = wchar2string(taskName);
                    SysFreeString(taskName);

                    // 获取格式化的可执行文件地址
                    image_path = format_image_path(image_path);
                    // printf("\n%s", image_path.c_str());

                    // 获取可执行文件描述
                    TCHAR* fileDescription = new TCHAR[1024];
                    wstring str = string2wstring(image_path);
                    BOOL bRet = GetFileVersionString(str.c_str(), _T("FileDescription"), fileDescription, 1024);
                    string file_description = TCHAR2char(fileDescription);
                    int n;
                    if ((n = file_description.find("?")) != string::npos)
                        file_description = "";

                    // 格式化输出
                    string info = task_name;
                    string split_sign("$#&");
                    info.append(split_sign);
                    info.append(file_description);
                    info.append(split_sign);
                    info.append(image_path);
                    printf("%s\n", info.c_str());
                    // printf("\n%s", file_description.c_str());
                }
                execAction->Release();
            }
            pRegisteredTask->Release();
        }
    }
    pTaskCollection->Release();
}
