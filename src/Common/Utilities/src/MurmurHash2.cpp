//
// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//
#include <sal.h>


namespace BitFunnel
{
    //*************************************************************************
    // MurmurHash2, 64-bit versions, by Austin Appleby
    // The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
    // and endian-ness issues if used across multiple platforms.
    //*************************************************************************

    // 64-bit hash for 64-bit platforms
    unsigned __int64 MurmurHash64A(const void *key, size_t len, unsigned seed)
    {
        const unsigned __int64 m = 0xc6a4a7935bd1e995;
        const int r = 47;
        unsigned __int64 h = seed ^ (len * m);
        const unsigned __int64 *data = (const unsigned __int64 *)key;
        const unsigned __int64 *end = data + (len / 8);
        while (data != end)
        {
            unsigned __int64 k = *data++;
            k *= m;
            k ^= k >> r;
            k *= m;
            h ^= k;
            h *= m;
        }
        const unsigned char *data2 = (const unsigned char *)data;

        switch(len &7)
        {
        case 7: h ^= unsigned __int64(data2[6]) << 48;
            __fallthrough;
        case 6: h ^= unsigned __int64(data2[5]) << 40;
            __fallthrough;
        case 5: h ^= unsigned __int64(data2[4]) << 32;
            __fallthrough;
        case 4: h ^= unsigned __int64(data2[3]) << 24;
            __fallthrough;
        case 3: h ^= unsigned __int64(data2[2]) << 16;
            __fallthrough;
        case 2: h ^= unsigned __int64(data2[1]) << 8;
            __fallthrough;
        case 1: h ^= unsigned __int64(data2[0]);
            h *= m;
        }
        ;
        h ^= h >> r;
        h *= m;
        h ^= h >> r;
        return h;
    }


    //*************************************************************************
    // MurmurHash2, 32-bit versions, by Austin Appleby
    // The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
    // and endian-ness issues if used across multiple platforms.
    //*************************************************************************
    #define mmix(h,k) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }

    unsigned __int32 MurmurHash32A(const void * key, size_t len, unsigned seed)
    {
	    const unsigned __int32 m = 0x5bd1e995;
	    const int r = 24;
	    unsigned __int32 l = static_cast<unsigned __int32>(len);

	    const unsigned char * data = (const unsigned char *)key;

	    unsigned __int32 h = seed;

	    while(len >= 4)
	    {
		    unsigned __int32 k = *(unsigned int*)data;

		    mmix(h,k);

		    data += 4;
		    len -= 4;
	    }

	    unsigned __int32 t = 0;

	    switch(len)
	    {
	    case 3: t ^= data[2] << 16;
	    case 2: t ^= data[1] << 8;
	    case 1: t ^= data[0];
	    };

	    mmix(h,t);
	    mmix(h,l);

	    h ^= h >> 13;
	    h *= m;
	    h ^= h >> 15;

	    return h;
    }

    #undef mmix
}
