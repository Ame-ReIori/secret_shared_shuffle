#pragma once
#ifndef __DLEN_PRG_H
#define __DLEN_PRG_H

#include "defines.h"

#ifdef USE_EMP
#include <emp-tool/emp-tool.h>
using namespace emp;
#endif

#ifdef USE_LIBOTE
#include <cryptoTools/Common/block.h>
#include <cryptoTools/Crypto/AES.h>
using namespace osuCrypto;
#endif

void DLenPRG(block seed, block *out);
#endif