#pragma once
#ifndef __DLEN_PRG_H
#define __DLEN_PRG_H

#include "defines.h"

// #ifdef USE_EMP
#include <emp-tool/emp-tool.h>
using namespace emp;

void DLenPRG(block seed, block *out);
// #endif

#endif