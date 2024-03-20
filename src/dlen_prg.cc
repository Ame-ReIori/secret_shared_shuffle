#include "dlen_prg.h"

#ifdef USE_EMP
using namespace emp;
void DLenPRG(block seed, block *out) {
  AES_KEY aes;
  AES_set_encrypt_key(all_one_block, &aes);
  out[0] = makeBlock(0, 1) ^ seed;
  out[1] = makeBlock(0, 2) ^ seed;
  
  AES_ecb_encrypt_blks(out, 2, &aes);
}
#endif

#ifdef USE_LIBOTE
using namespace osuCrypto;
void DLenPRG(block seed, block *out) {
  AES aes(AllOneBlock);
  block pt[2] = { toBlock(1) ^ seed, toBlock(2) ^ seed };
  aes.ecbEncBlocksInline<2>(pt, out);
}
#endif