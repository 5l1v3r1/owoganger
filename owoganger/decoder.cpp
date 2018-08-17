#include "stdafx.h"
#include "decoder.h"





decoder::decoder(DWORD flag, std::vector<BYTE> * encodedBuf, std::string xorKey)
{
	this->encodedBuf = encodedBuf;
	this->flag = flag;
	this->xor_key = xorKey;
	//this->decodedBuf = new std::vector<BYTE>;
}


decoder::~decoder()
{
	
}

void decoder::initDecode()
{
	if (this->flag == SOURCE_XOR) {
		if (!this->xor_key.empty()) {
			this->decxor();
			std::cout << "[+] Dexored the file" << std::endl;
		}
		else {
			std::cout << "[-] No xor key supplied" << std::endl;
			exit(-1);
		}
	}
	return;
}


void decoder::decxor() {
	if (this->encodedBuf->size() == 0) {
		std::cout << "[-] Empty encoded buffer" << std::endl;
		exit(-2);
	}
	int i = 0;
	std::cout << "[x] Dexoring the file" << std::endl;
	if (this->xor_key.length() == 1) {	//one char xor
		while (i < this->encodedBuf->size())  
		{
			this->encodedBuf->at(i) = (BYTE)(this->encodedBuf->at(i) ^ (BYTE)this->xor_key[0]);
			i++;
		}
	
	}
	else {
		while (i < this->encodedBuf->size()) {
			this->encodedBuf->at(i) = (BYTE)(this->encodedBuf->at(i) ^ (BYTE)this->xor_key[i % this->xor_key.length()]);
			i++;
		}
	}
}