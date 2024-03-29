#pragma once
#ifndef __SS_SHUFFLE_H
#define __SS_SHUFFLE_H

#include "defines.h"
#include "opv.h"

#ifdef USE_EMP
#include <emp-tool/emp-tool.h>

void ShareTranslation(uint64_t *perm, int length, int party, HighSpeedNetIO *io, 
                      block *a, block *b, block *delta);

void Offline(uint64_t N, uint64_t T, uint64_t *perms, uint64_t party, HighSpeedNetIO *io,
             uint64_t *perm, block *a, block *b, block *delta);

void PermuteShare(uint64_t N, uint64_t T, 
                  uint64_t *perm, block *delta,
                  block *x, block *a, block *b,
                  uint64_t party, HighSpeedNetIO *io, 
                  block *out);

void SecretSharedShuffle(uint64_t N, uint64_t T, uint64_t party, HighSpeedNetIO *io, 
                         block *x, uint64_t *perm, block *delta, block *a, block *b,
                         block *out);
#endif


#ifdef USE_LIBOTE
#include <cryptoTools/Crypto/PRNG.h>
#include <cryptoTools/Common/block.h>
#include <coproto/Socket/AsioSocket.h>

void ShareTranslation(uint64_t *perm, int length, int party, cp::Socket & chl, 
                      block *a, block *b, block *delta);

void Offline(uint64_t N, uint64_t T, uint64_t *perms, uint64_t party, cp::Socket & chl,
             uint64_t *perm, block *a, block *b, block *delta);

void PermuteShare(uint64_t N, uint64_t T,
                  uint64_t *perm, block *delta,
                  block *x, block *a, block *b,
                  uint64_t party, cp::Socket & chl,
                  block *out);

void SecretSharedShuffle(uint64_t N, uint64_t T, uint64_t party, cp::Socket & chl,
                         block *x, uint64_t *perm, block *delta, block *a, block *b,
                         block *out);
#endif

void PermReconstruct(int d, uint64_t N, uint64_t T, int n, int t, uint64_t *perms, uint64_t *perm);

#endif