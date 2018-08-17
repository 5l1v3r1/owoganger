#pragma once
#include <vector>
#include "stdafx.h"
#include "typedefs.h"
class decoder
{
public:
	decoder(DWORD flag, std::vector<BYTE> * encodedBuf, std::string xor_key);
	~decoder();
	void initDecode();
	
private:
	void decxor();
	std::vector<BYTE>  *encodedBuf;
	std::string xor_key;
	DWORD flag;
};

