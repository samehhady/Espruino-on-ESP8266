#ifndef sha1_h
#define sha1_h

//#include <inttypes.h>
#include <c_types.h>

void sha1Init(void);
//void sha1InitHmac(const uint8_t* secret, int secretLength);
uint8_t *sha1Result(void);
//uint8_t *sha1ResultHmac(void);
//size_t sha1Write(uint8_t);
size_t sha1Print(char *str);

#endif
