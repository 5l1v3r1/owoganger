#pragma once
#include "stdafx.h"
#include <vector>
class owoganger
{
public:
	owoganger(std::string,std::string);
	~owoganger();

private:
	void transact(); // step 1
	void load(HANDLE, HANDLE); // step 2
	void rollback(HANDLE); // step 3
	void animate(HANDLE); // step 4
	BOOL checkNtStatus(NTSTATUS *);
	void readFile();
	LPCSTR source; //source executable path
	LPCSTR destination; //destination executable path
	HMODULE ntdll;
	std::vector<BYTE> exeBuf;



};