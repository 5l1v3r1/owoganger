#include "stdafx.h"
#include "decoder.h"





decoder::decoder(DWORD flag, std::vector<BYTE> * encodedBuf)
{
	this->encodedBuf = *encodedBuf;
	this->flag = flag;
}

decoder::decoder(DWORD flag, std::vector<BYTE> * encodedBuf, std::string xor_key)
{
	this->encodedBuf = *encodedBuf;
	this->flag = flag;
	this->xor_key == xor_key;
}


decoder::~decoder()
{
}

std::vector<BYTE>* decoder::initDecode()
{
	if (this->flag == SOURCE_B64) {
		this->decb64();
	}
	if (this->flag == SOURCE_XOR) {
		if (!this->xor_key.empty()) {
			this->decxor();
		}
		else {
			std::cout << "[-] No xor key supplied" << std::endl;
			exit(-1);
		}
	}
	return &this->decodedBuf;
}

void decoder::decb64()
{
	if (this->encodedBuf.size() == 0) {
		std::cout << "[-] Empty encoded buffer" << std::endl;
		exit(-2);
	}
	BYTE * tmpBuf = new BYTE[this->encodedBuf.size()];// assuming it's not so big lol
	std::copy(this->encodedBuf.begin(), this->encodedBuf.end(), tmpBuf);
	//DO MAGIC

}

void decoder::decxor() {
	if (this->encodedBuf.size() == 0) {
		std::cout << "[-] Empty encoded buffer" << std::endl;
		exit(-2);
	}
	BYTE * tmpBuf = new BYTE[this->encodedBuf.size()];
	std::copy(this->encodedBuf.begin(), this->encodedBuf.end(), tmpBuf);
	//DO MAGIC
}