#pragma once
#include <vector>
#include "stdafx.h"
#include "typedefs.h"
class decoder
{
public:
	decoder(DWORD flag, std::vector<BYTE> * encodedBuf);
	decoder(DWORD flag, std::vector<BYTE> * encodedBuf, std::string xor_key);
	~decoder();
	std::vector<BYTE> * initDecode();
	
private:
	void decb64();
	void decxor();
	std::vector<BYTE>decodedBuf;
	std::vector<BYTE>encodedBuf;
	std::string xor_key;
	DWORD flag;
};

