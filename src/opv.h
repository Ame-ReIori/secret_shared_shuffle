#pragma once
#ifndef __OPV_H
#define __OPV_H

#include "defines.h"
#include "dlen_prg.h"

#include <deque>
#include <vector>
// #ifdef USE_EMP
#include <emp-tool/emp-tool.h>
#include <emp-ot/emp-ot.h>

using namespace emp;

/**
 * @param length: vector length
 * @param i: punctured value
 * @param party: party
 * @param seeds: outputed seeds
 */
void OblivSetup(uint64_t length, uint64_t x, int party, HighSpeedNetIO *io, block **seeds);

void Expand(uint64_t length, uint64_t x, block *seeds, int party, block *v);

// #endif

#endif