#pragma once
#ifndef __OPV_H
#define __OPV_H

#include "defines.h"
#include "dlen_prg.h"

#include <deque>
#include <vector>

#ifdef USE_EMP
#include <emp-tool/emp-tool.h>
#include <emp-ot/emp-ot.h>

using namespace emp;
#endif

#ifdef USE_LIBOTE
#include <cryptoTools/Common/block.h>
#include <libOTe/Base/BaseOT.h>
#include <libOTe/TwoChooseOne/Iknp/IknpOtExtReceiver.h>
#include <libOTe/TwoChooseOne/Iknp/IknpOtExtSender.h>
#include <coproto/Socket/AsioSocket.h>

using namespace osuCrypto;
#endif

/**
 * @param length: vector length
 * @param i: punctured value
 * @param party: party
 * @param seeds: outputed seeds
 */
#ifdef USE_EMP
void OblivSetup(uint64_t length, uint64_t x, int party, HighSpeedNetIO *io, block **seeds);
#endif

#ifdef USE_LIBOTE
void OblivSetup(uint64_t length, uint64_t x, int party, cp::Socket & chl, block **seeds);
#endif

void Expand(uint64_t length, uint64_t x, block *seeds, int party, block *v);

#endif