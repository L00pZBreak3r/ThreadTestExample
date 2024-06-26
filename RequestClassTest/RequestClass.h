#pragma once

class Request
{
    static const int WaitMillis = 2000;
public:
    Request(int aId) : Id(aId) {}
    Request(const Request&) = delete;
    Request(Request&&) = delete;
    Request& operator=(const Request& other) = delete;
    ~Request();

    void DoWork();

    int Id;
    int ThreadId = 0;
};

class RequestManager
{
    int mMaxCount;
public:
    RequestManager(int aMaxCount) : mMaxCount(aMaxCount) {}
    RequestManager(const RequestManager&) = delete;
    RequestManager(RequestManager&&) = delete;
    RequestManager& operator=(const RequestManager& other) = delete;

    // возвращает nullptr если нужно завершить процесс, либо указатель на память,
    // которую в дальнейшем требуется удалить
    Request* GetRequest() throw();

    // обрабатывает запрос, но память не удаляет
    void ProcessRequest(Request* request) throw();

    static const int NumberOfThreads = 16;

};
