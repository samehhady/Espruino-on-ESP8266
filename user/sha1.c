#include <string.h>
#include "sha1.h"
/*
#include "espmissingincludes.h"
#include "c_types.h"
#include "osapi.h"
*/
//#include "espmissingincludes.h"
#include "platform_config.h"
//#include "mem.h"
//#include "osapi.h"


#define HASH_LENGTH 20
#define BLOCK_LENGTH 64

static union _buffer {
	uint8_t b[BLOCK_LENGTH];
	uint32_t w[BLOCK_LENGTH/4];
} buffer;
uint8_t bufferOffset;

static union _state {
	uint8_t b[HASH_LENGTH];
	uint32_t w[HASH_LENGTH/4];
} state;
uint32_t byteCount;
uint8_t keyBuffer[BLOCK_LENGTH];
uint8_t innerHash[HASH_LENGTH];

#define SHA1_K0 0x5a827999
#define SHA1_K20 0x6ed9eba1
#define SHA1_K40 0x8f1bbcdc
#define SHA1_K60 0xca62c1d6

static const uint8_t sha1InitState[] = {
  0x01,0x23,0x45,0x67, // H0
  0x89,0xab,0xcd,0xef, // H1
  0xfe,0xdc,0xba,0x98, // H2
  0x76,0x54,0x32,0x10, // H3
  0xf0,0xe1,0xd2,0xc3  // H4
};

void sha1Init(void) {
  memcpy(state.b,sha1InitState, HASH_LENGTH);
  byteCount = 0;
  bufferOffset = 0;
}

static uint32_t sha1Rol32(uint32_t number, uint8_t bits) {
  return ((number << bits) | (number >> (32-bits)));
}

static void sha1HashBlock() {
  uint8_t i;
  uint32_t a,b,c,d,e,t;

  a=state.w[0];
  b=state.w[1];
  c=state.w[2];
  d=state.w[3];
  e=state.w[4];
  for (i=0; i<80; i++) {
    if (i>=16) {
      t = buffer.w[(i+13)&15] ^ buffer.w[(i+8)&15] ^ buffer.w[(i+2)&15] ^ buffer.w[i&15];
      buffer.w[i&15] = sha1Rol32(t,1);
    }
    if (i<20) {
      t = (d ^ (b & (c ^ d))) + SHA1_K0;
    } else if (i<40) {
      t = (b ^ c ^ d) + SHA1_K20;
    } else if (i<60) {
      t = ((b & c) | (d & (b | c))) + SHA1_K40;
    } else {
      t = (b ^ c ^ d) + SHA1_K60;
    }
    t+=sha1Rol32(a,5) + e + buffer.w[i&15];
    e=d;
    d=c;
    c=sha1Rol32(b,30);
    b=a;
    a=t;
  }
  state.w[0] += a;
  state.w[1] += b;
  state.w[2] += c;
  state.w[3] += d;
  state.w[4] += e;
}

static void sha1AddUncounted(uint8_t data) {
  buffer.b[bufferOffset ^ 3] = data;
  bufferOffset++;
  if (bufferOffset == BLOCK_LENGTH) {
    sha1HashBlock();
    bufferOffset = 0;
  }
}

static size_t sha1Write(uint8_t data) {
  ++byteCount;
  sha1AddUncounted(data);
  return 1;
}
size_t sha1Print(char *str) {
	size_t size = 0;
	while (*str) size += sha1Write((uint8_t)*(str++));
	return size;
}

static void sha1Pad() {
  // Implement SHA-1 padding (fips180-2 §5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  sha1AddUncounted(0x80);
  while (bufferOffset != 56) sha1AddUncounted(0x00);

  // Append length in the last 8 bytes
  sha1AddUncounted(0); // We're only using 32 bit lengths
  sha1AddUncounted(0); // But SHA-1 supports 64 bit lengths
  sha1AddUncounted(0); // So zero pad the top bits
  sha1AddUncounted(byteCount >> 29); // Shifting to multiply by 8
  sha1AddUncounted(byteCount >> 21); // as SHA-1 supports bitstreams as well as
  sha1AddUncounted(byteCount >> 13); // byte.
  sha1AddUncounted(byteCount >> 5);
  sha1AddUncounted(byteCount << 3);
}


uint8_t *sha1Result(void) {
  // Pad to complete the last block
  sha1Pad();
  
  // Swap byte order back
  for (int i=0; i<5; i++) {
    uint32_t a,b;
    a=state.w[i];
    b=a<<24;
    b|=(a<<8) & 0x00ff0000;
    b|=(a>>8) & 0x0000ff00;
    b|=a>>24;
    state.w[i]=b;
  }
  
  // Return pointer to hash (20 characters)
  return state.b;
}

/*
#define HMAC_IPAD 0x36
#define HMAC_OPAD 0x5c

void sha1InitHmac(const uint8_t* key, int keyLength) {
  uint8_t i;
  memset(keyBuffer,0,BLOCK_LENGTH);
  if (keyLength > BLOCK_LENGTH) {
    // Hash long keys
    sha1Init();
    for (;keyLength--;) sha1Write(*key++);
    memcpy(keyBuffer, sha1Result(), HASH_LENGTH);
  } else {
    // Block length keys are used as is
    memcpy(keyBuffer,key,keyLength);
  }
  // Start inner hash
  sha1Init();
  for (i=0; i<BLOCK_LENGTH; i++) {
    sha1Write(keyBuffer[i] ^ HMAC_IPAD);
  }
}

uint8_t* sha1ResultHmac(void) {
  uint8_t i;
  // Complete inner hash
  memcpy(innerHash, sha1Result(),HASH_LENGTH);
  // Calculate outer hash
  sha1Init();
  for (i=0; i<BLOCK_LENGTH; i++) sha1Write(keyBuffer[i] ^ HMAC_OPAD);
  for (i=0; i<HASH_LENGTH; i++) sha1Write(innerHash[i]);
  return sha1Result();
}
*/