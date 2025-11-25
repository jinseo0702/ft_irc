#ifndef SHA256_HPP
#define SHA256_HPP

#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

typedef unsigned int ULONG;
typedef ULONG* ULONG_PTR;

typedef unsigned int UINT;
typedef UINT* UINT_PTR;

typedef signed int SINT;
typedef SINT* SINT_PTR;

typedef unsigned char UCHAR;
typedef UCHAR* UCHAR_PTR;

typedef unsigned char BYTE;

static const int SHA256_DIGEST_BLOCKLEN	= 64;
static const int SHA256_DIGEST_VALUELEN	= 32;

struct SHA256_INFO{
    UINT uChainVar[SHA256_DIGEST_VALUELEN / 4];
	UINT uHighLength;
	UINT uLowLength;
	UINT remain_num;
	BYTE szBuffer[SHA256_DIGEST_BLOCKLEN];
};


class SHA256
{
    private:
    static const UINT SHA256_K[64];
    SHA256_INFO Info;
    
    inline ULONG ROTL_ULONG(ULONG x, int n) const {
        return ((x << n) | (x >> (32 - n)));
    }
    inline ULONG ROTR_ULONG(ULONG x, int n) const {
        return ((x >> n) | (x << (32 - n)));
    }
    
    inline ULONG ENDIAN_REVERSE_ULONG(ULONG dws) const {
        return ( (ROTL_ULONG((dws),  8) & 0x00ff00ff)	\
        | (ROTL_ULONG((dws), 24) & 0xff00ff00) );
    }
    
    inline void BIG_B2D(const BYTE* B, ULONG& D) const{
        D = ENDIAN_REVERSE_ULONG(*reinterpret_cast<const ULONG*>(B));
    }
    
    inline void BIG_D2B(const ULONG* D, BYTE* B) const{
        *reinterpret_cast<ULONG*>(B) = ENDIAN_REVERSE_ULONG(*D);
    }
    
    inline void LITTLE_B2D(const BYTE* B, ULONG& D) const{
        D = *reinterpret_cast<const ULONG*>(B);
    }
    
    inline void LITTLE_D2B(const ULONG* D, BYTE* B) const{
        *reinterpret_cast<ULONG*>(B) = *D;
    }
    
    inline ULONG RR(ULONG x, int n) const {
        return (ROTR_ULONG(x, n));
    }
    
    inline ULONG SS(ULONG x, int n) const {
        return (x >> n);
    }
    
    inline ULONG Ch(ULONG x, ULONG y, ULONG z) const {
        return (((x & y) ^ ((~x) & z)));
    }
    
    inline ULONG Maj(ULONG x, ULONG y, ULONG z) const {
        return (((x & y) ^ (x & z) ^ (y & z)));
    }
    
    inline ULONG Sigma0(ULONG x) const {
        return (RR(x,  2) ^ RR(x, 13) ^ RR(x, 22));
    }
    
    inline ULONG Sigma1(ULONG x) const {
        return (RR(x,  6) ^ RR(x, 11) ^ RR(x, 25));
    }
    
    inline ULONG RHO0(ULONG x) const {
        return (RR(x,  7) ^ RR(x, 18) ^ SS(x,  3));
    }
    
    inline ULONG RHO1(ULONG x) const {
        return (RR(x, 17) ^ RR(x, 19) ^ SS(x, 10));
    }
    
    inline void FF(ULONG& a, ULONG& b, ULONG& c, ULONG& d, 
        ULONG& e, ULONG& f, ULONG& g, ULONG& h, 
        ULONG j, ULONG* X) {
            ULONG T1 = h + Sigma1(e) + Ch(e, f, g) + SHA256_K[j] + X[j];
            d += T1;
            h = T1 + Sigma0(a) + Maj(a, b, c);
        }
        
        inline ULONG GetData(ULONG x) const {
            #if defined(BIG_ENDIAN)
            return x;
            #else
            return ENDIAN_REVERSE_ULONG(x);
            #endif
        }
        
        inline bool definedLittleEndian(){
            #if defined(LITTLE_ENDIAN)
            return (true);
            #else
            return (false);
            #endif
        }
        
        SHA256();
        ~SHA256();
        void SHA256_Transform(ULONG_PTR Message, ULONG_PTR ChainVar);
        void SHA256_Init();
        void SHA256_Process( const BYTE *pszMessage, UINT uDataLen );
        void SHA256_Close( BYTE *pszDigest );
        static void SHA256_Encrpyt( const BYTE *pszMessage, UINT uPlainTextLen, BYTE *pszDigest );
        
    public:
        static std::string SaltMaker();
        static std::string SHA256Maker(const std::string& str);
    };
    
    #endif