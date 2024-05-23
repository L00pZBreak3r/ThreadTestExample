// Main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "RequestWorker.h"

int _tmain(int argc, _TCHAR* argv[])
{
    (void)_setmode(_fileno(stdout), _O_U16TEXT);


    ExecuteRequestWorker(20000, argc > 1);
    return 0;
}
