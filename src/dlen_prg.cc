#include "dlen_prg.h"

void DLenPRG(block seed, block *out) {
  AES_KEY aes;
  emp::AES_set_encrypt_key(all_one_block, &aes);
  out[0] = makeBlock(0, 1) ^ seed;
  out[1] = makeBlock(0, 2) ^ seed;
  
  AES_ecb_encrypt_blks(out, 2, &aes);
}