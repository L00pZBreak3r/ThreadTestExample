#include "pch.h"
#include "RequestClass.h"


#include <iostream>

void Request::DoWork()
{
	std::wcout << L"Thread(" << ThreadId << L"):Request(" << Id << L")...";
	Sleep(WaitMillis);
	std::wcout << L"done." << std::endl;
}

Request::~Request()
{
	std::wcout << L"Thread(" << ThreadId << L"):Request(" << Id << L")...deleted." << std::endl;
}

// возвращает nullptr если нужно завершить процесс, либо указатель на память,
// которую в дальнейшем требуется удалить
Request* RequestManager::GetRequest() throw()
{
	if (mMaxCount <= 0)
		return nullptr;
	return new Request(mMaxCount--);
}

// обрабатывает запрос, но память не удаляет
void RequestManager::ProcessRequest(Request* request) throw()
{
	if (request)
		request->DoWork();
}

