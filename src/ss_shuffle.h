#pragma once
#ifndef __SS_SHUFFLE_H
#define __SS_SHUFFLE_H

#include "defines.h"
#include "opv.h"

#include <emp-tool/emp-tool.h>

void ShareTranslation(uint64_t *perm, int length, int party, HighSpeedNetIO *io, 
                      block *a, block *b, block *delta);

void PermReconstruct(int d, uint64_t N, uint64_t T, int n, int t, uint64_t *perms, uint64_t *perm);

void Offline(uint64_t N, uint64_t T, uint64_t *perms, uint64_t party, HighSpeedNetIO *io,
             uint64_t *perm, uint64_t *subperms,
             block *a, block *b, block *offset, block *delta);

void PermuteShare(uint64_t N, uint64_t T, 
                  uint64_t *perm, uint64_t *subperms, block *delta,
                  block *x, block *a, block *b, block *offset,
                  uint64_t party, HighSpeedNetIO *io, 
                  block *out);

void SecretSharedShuffle(uint64_t N, uint64_t T, uint64_t party, HighSpeedNetIO *io, 
                         block *x, block *a, block *b, block *offset,
                         uint64_t *subperms, uint64_t *perm, block *delta,
                         block *out);

#endif