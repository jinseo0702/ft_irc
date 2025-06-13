#include "../include/SHA256.hpp"

const UINT SHA256::SHA256_K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
	0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
	0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
	0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
	0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
	0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

SHA256::SHA256(){
	SHA256_Init();
};

SHA256::~SHA256(){
	std::memset(&Info, 0, sizeof(Info));
};

void SHA256::SHA256_Init(){
	this->Info.uChainVar[0] = 0x6a09e667;
	this->Info.uChainVar[1] = 0xbb67ae85;
	this->Info.uChainVar[2] = 0x3c6ef372;
	this->Info.uChainVar[3] = 0xa54ff53a;
	this->Info.uChainVar[4] = 0x510e527f;
	this->Info.uChainVar[5] = 0x9b05688c;
	this->Info.uChainVar[6] = 0x1f83d9ab;
	this->Info.uChainVar[7] = 0x5be0cd19;

	this->Info.uHighLength = this->Info.uLowLength = this->Info.remain_num = 0;
};

void SHA256::SHA256_Process(const BYTE *pszMessage, UINT uDataLen){

	UINT remain_buffer = this->Info.remain_num;

	if ((this->Info.uLowLength += (uDataLen << 3)) < 0)
		this->Info.uHighLength++;

	this->Info.uHighLength += (uDataLen >> 29);

	while ((uDataLen + remain_buffer) >= SHA256_DIGEST_BLOCKLEN)
	{
		std::memcpy((UCHAR_PTR)(this->Info.szBuffer + remain_buffer), pszMessage, (SINT)SHA256_DIGEST_BLOCKLEN);
		SHA256_Transform((ULONG_PTR)this->Info.szBuffer, this->Info.uChainVar);
		pszMessage += (SHA256_DIGEST_BLOCKLEN - remain_buffer);
		uDataLen -= (SHA256_DIGEST_BLOCKLEN - remain_buffer);
		remain_buffer = 0;
	}

	std::memcpy((UCHAR_PTR)(this->Info.szBuffer + remain_buffer), pszMessage, uDataLen);
	this->Info.remain_num = remain_buffer + uDataLen;
};

void SHA256::SHA256_Close(BYTE* pszDigest) {
	ULONG i, Index;

	Index = (this->Info.uLowLength >> 3) % SHA256_DIGEST_BLOCKLEN;
	this->Info.szBuffer[Index++] = 0x80;

	if (Index > SHA256_DIGEST_BLOCKLEN - 8){
		std::memset((UCHAR_PTR)this->Info.szBuffer + Index, 0, (SINT)(SHA256_DIGEST_BLOCKLEN - Index));
		SHA256_Transform((ULONG_PTR)this->Info.szBuffer, this->Info.uChainVar);
		std::memset((UCHAR_PTR)this->Info.szBuffer, 0, (SINT)SHA256_DIGEST_BLOCKLEN - 8);
	}
	else
		std::memset((UCHAR_PTR)this->Info.szBuffer + Index, 0, (SINT)(SHA256_DIGEST_BLOCKLEN - Index - 8));
	if (definedLittleEndian()){
		this->Info.uLowLength = ENDIAN_REVERSE_ULONG(this->Info.uLowLength);
		this->Info.uHighLength = ENDIAN_REVERSE_ULONG(this->Info.uHighLength);
	}
	((ULONG_PTR)this->Info.szBuffer)[SHA256_DIGEST_BLOCKLEN / 4 - 2] = this->Info.uHighLength;
	((ULONG_PTR)this->Info.szBuffer)[SHA256_DIGEST_BLOCKLEN / 4 - 1] = this->Info.uLowLength;

	SHA256_Transform((ULONG_PTR)this->Info.szBuffer, this->Info.uChainVar);

	for (i = 0; i < SHA256_DIGEST_VALUELEN; i += 4)
        BIG_D2B(&(this->Info.uChainVar)[i / 4], &pszDigest[i]);
	
}

void SHA256::SHA256_Encrpyt(const BYTE* pszMessage, UINT uPlainTextLen, BYTE* pszDigest) {
    SHA256_Init();
    SHA256_Process(pszMessage, uPlainTextLen);
    SHA256_Close(pszDigest);
}


void SHA256::SHA256_Transform(ULONG_PTR Message, ULONG_PTR ChainVar){
	
    ULONG a, b, c, d, e, f, g, h, X[64];
	ULONG j;

	for (j = 0; j < 16; j++){
		X[j] = GetData(Message[j]);
	}

	for (j = 16; j < 64; j++){
		X[j] = RHO1(X[j - 2]) + X[j - 7] + RHO0(X[j - 15]) + X[j - 16];
	}

	a = ChainVar[0];
	b = ChainVar[1];
	c = ChainVar[2];
	d = ChainVar[3];
	e = ChainVar[4];
	f = ChainVar[5];
	g = ChainVar[6];
	h = ChainVar[7];

	for (j = 0; j < 64; j += 8){
		FF(a, b, c, d, e, f, g, h, j + 0, X);
		FF(h, a, b, c, d, e, f, g, j + 1, X);
		FF(g, h, a, b, c, d, e, f, j + 2, X);
		FF(f, g, h, a, b, c, d, e, j + 3, X);
		FF(e, f, g, h, a, b, c, d, j + 4, X);
		FF(d, e, f, g, h, a, b, c, j + 5, X);
		FF(c, d, e, f, g, h, a, b, j + 6, X);
		FF(b, c, d, e, f, g, h, a, j + 7, X);
	}

	ChainVar[0] += a;
	ChainVar[1] += b;
	ChainVar[2] += c;
	ChainVar[3] += d;
	ChainVar[4] += e;
	ChainVar[5] += f;
	ChainVar[6] += g;
	ChainVar[7] += h;
};