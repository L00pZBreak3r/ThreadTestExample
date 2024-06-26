#include "pch.h"
#include "RequestClass.h"
#include "TSQueueClass.h"
#include "RequestWorker.h"

#include <iostream>

/*
Основной поток должен:
1)	Запустить несколько рабочих потоков (NumberOfThreads).
2)	Класть в одну очередь заданий задачи до тех пор, пока GetRequest() не вернёт nullptr.
3)	Корректно остановить рабочие потоки. Они должны доделать текущий ProcessRequest, если он имеется, и остановиться. Если имеются необработанные задания, не обращать на них внимания.
4)	Завершить программу.

Рабочий поток должен:

1)	Обрабатывать поступающие через очередь запросы с помощью ProcessRequest.
2)	Завершиться, как только основной поток ему это скомандует.

Вызовы GetRequest() и ProcessRequest() могут работать долго.
*/

using RequestQueue = TSQueue<Request*>;

#define THREAD_WAIT_RELAX_MILLIS 20

struct SWorkerInfo
{
public:
    int ThreadId;
    RequestManager* Manager;
    RequestQueue* Queue;
    bool* StopPtr;
    std::mutex* ConsoleMutex;

    SWorkerInfo(int aThreadId, RequestManager* aManager, RequestQueue* aQueue, bool* aStopPtr, std::mutex* aConsoleMutex = nullptr)
        : ThreadId(aThreadId), Manager(aManager), Queue(aQueue), StopPtr(aStopPtr), ConsoleMutex(aConsoleMutex) {}
    SWorkerInfo(const SWorkerInfo&) = delete;
    SWorkerInfo(SWorkerInfo&&) = delete;
    SWorkerInfo& operator=(const SWorkerInfo& other) = delete;
};

static void DoProcessRequest(RequestManager* aManager, Request* aRequest)
{
    try
    {
        aManager->ProcessRequest(aRequest);
    }
    catch (...) {}
    delete aRequest;
}

static UINT WINAPI WorkerThreadFunc(LPVOID pArguments)
{
    const SWorkerInfo* aWorkerInfo = (const SWorkerInfo*)pArguments;
    RequestManager* aManager = aWorkerInfo->Manager;
    RequestQueue* aQueue = aWorkerInfo->Queue;
    std::mutex* aConsoleMutex = aWorkerInfo->ConsoleMutex;

    while (!*aWorkerInfo->StopPtr)
    {
        Request* aRequest = nullptr;
        if (aQueue->pop(&aRequest) && (aRequest != nullptr))
        {
            aRequest->ThreadId = aWorkerInfo->ThreadId;
            if (aConsoleMutex)
            {
                std::unique_lock<std::mutex> lock(*aConsoleMutex);
                DoProcessRequest(aManager, aRequest);
            }
            else
                DoProcessRequest(aManager, aRequest);
        }
        Sleep(THREAD_WAIT_RELAX_MILLIS);
    }

    _endthreadex(0);
    return 0;
}

static Request* ManagerGetRequest(RequestManager* aManager)
{
    Request* aResult = nullptr;
    try
    {
        aResult = aManager->GetRequest();
    }
    catch (...) {}
    return aResult;
}

void ExecuteRequestWorker(int aWaitMillis, bool aUseDebugOutput)
{
    std::wcout << L"Starting Manager thread." << std::endl;

    std::mutex aConsoleMutex;
    bool aStop = false;
    RequestManager* aManager = new RequestManager(32);
    RequestQueue* aQueue = new RequestQueue(false);

    SWorkerInfo* aWorkerInfos[RequestManager::NumberOfThreads];
    HANDLE aWorkerThreads[RequestManager::NumberOfThreads];

    for (int i = 0; i < RequestManager::NumberOfThreads; i++)
    {
        aWorkerInfos[i] = new SWorkerInfo(i + 1, aManager, aQueue, &aStop, (aUseDebugOutput) ? &aConsoleMutex : nullptr);

        unsigned threadID;
        aWorkerThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, &WorkerThreadFunc, aWorkerInfos[i], 0, &threadID);
    }

    Request* aRequest;
    while ((aRequest = ManagerGetRequest(aManager)))
    {
        aQueue->push(aRequest);
        Sleep(THREAD_WAIT_RELAX_MILLIS);
    }

    std::wcout << L"No more work." << std::endl;
    Sleep(aWaitMillis);
    aStop = true;
    std::wcout << L"Stopping all worker threads." << std::endl;

    WaitForMultipleObjects(RequestManager::NumberOfThreads, aWorkerThreads, TRUE, INFINITE);

    while (aQueue->pop(&aRequest))
    {
        delete aRequest;
    }

    for (int i = 0; i < RequestManager::NumberOfThreads; i++)
    {
        CloseHandle(aWorkerThreads[i]);
        delete aWorkerInfos[i];
    }

    delete aQueue;
    delete aManager;
}