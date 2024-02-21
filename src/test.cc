#include "opv.h"
#include "ss_shuffle.h"

#include <iostream>
#include <vector>

// #ifdef USE_EMP
using namespace emp;
// #endif

// #ifdef USE_EMP
void TestIKNPOT(char **argv) {
  int port, party;
  int nOT = 128;

  block *m0 = new block[nOT];
  block *m1 = new block[nOT];
  block *r = new block[nOT];
  bool *b = new bool[nOT];
  
  PRG prg = new PRG(fix_key);

  emp::parse_party_and_port(argv, &party, &port);

  std::cout << "port: " << port << ", party: " << party << std::endl;

  // establish connection
  HighSpeedNetIO *io = new HighSpeedNetIO(party == ALICE ? nullptr : "127.0.0.1", 
                                          port,
                                          port + 1,
                                          false);
  
  // initial iknp ot
  IKNP<HighSpeedNetIO> *iknp = new IKNP<HighSpeedNetIO>(io);

  // generate random data
  if (party == ALICE) {
    prg.random_block(m0, nOT);
    prg.random_block(m1, nOT);
  } else {
    prg.random_bool(b, nOT);
  }

  // start ot and timing
  auto start = emp::clock_start();
  if (party == ALICE) {
    iknp->send(m0, m1, nOT);
  } else {
    iknp->recv(r, b, nOT);
  }
  io->flush();
  long long dura = emp::time_from(start);

  std::cout << double(dura) / 1000 << "ms" << std::endl;

  delete[] m0;
  delete[] m1;
  delete[] r;
  delete[] b;
  delete io;
}
// #endif

void TestPermutation() {
  uint64_t index[16];
  std::iota(index, index + 16, 0);
  std::random_device rd;
  std::mt19937 g(rd());
 
  std::shuffle(index, index + 16, g);
  
  for (int i = 0; i < 16; i++) {
    std::cout << index[i] << " ";
  }
  std::cout << std::endl;
}

namespace OPVTest {

void TestOblivSetup(char **argv) {
  int port, party;
  block *seeds;

  emp::parse_party_and_port(argv, &party, &port);

  // establish connection
  HighSpeedNetIO *io = new HighSpeedNetIO(party == ALICE ? nullptr : "127.0.0.1", 
                                          port,
                                          port + 1,
                                          false);

  OblivSetup(8, 3, party, io, &seeds);

  delete io;
}

void TestExpand(char **argv) {
  int port, party;
  int length = 8;
  int x = 3;
  block v[length];
  block *seeds;

  emp::parse_party_and_port(argv, &party, &port);

  // establish connection
  HighSpeedNetIO *io = new HighSpeedNetIO(party == ALICE ? nullptr : "127.0.0.1", 
                                          port,
                                          port + 1,
                                          false);

  OblivSetup(length, x, party, io, &seeds);

  Expand(length, x, seeds, party, v);

  std::cout << "============= " << (party == ALICE ? "Alice" : "Bob") << std::endl;

  for (int i = 0; i < length; i++) {
    std::cout << v[i] << std::endl;
  }

  std::cout << "==============================" << std::endl;

  delete io;
}

}

namespace ShuffleTest {

void TestShareTranslation(char **argv) {
  int port, party;
  int length = 4;

  emp::parse_party_and_port(argv, &party, &port);

  // establish connection
  HighSpeedNetIO *io = new HighSpeedNetIO(party == ALICE ? nullptr : "127.0.0.1", 
                                          port,
                                          port + 1,
                                          false);

  uint64_t perm[4];
  perm[0] = 3;
  perm[1] = 1;
  perm[2] = 0;
  perm[3] = 2;

  block a[length], b[length], delta[length];

  ShareTranslation(perm, length, party, io, a, b, delta);

  delete io;
}

void TestPermReconstruct() {
  uint64_t N = 16;
  uint64_t T = 4;
  int n = (int)(log2(N));
  int t = (int)(log2(T));
  int d = 2 * (int)ceil(n / t) - 1;
  int subperm_num = N / T;

  uint64_t perms[d][subperm_num][T] = 
    {{{3, 2, 0, 1}, {1, 3, 0, 2}, {1, 0, 3, 2}, {2, 0, 1, 3}}, 
     {{1, 3, 0, 2}, {1, 0, 3, 2}, {2, 0, 1, 3}, {3, 2, 0, 1}}, 
     {{1, 2, 0, 3}, {0, 1, 3, 2}, {0, 1, 2, 3}, {0, 2, 3, 1}}};

  // uint64_t ***perms_ptr = (uint64_t ***)malloc(d * sizeof(uint64_t **));

  // for (int i = 0; i < d; i++) {
  //   perms_ptr[i] = (uint64_t **)malloc(subperm_num * sizeof(uint64_t *));
  //   for (int j = 0; j < subperm_num; j++) {
  //     perms_ptr[i][j] = (uint64_t *)malloc(T * sizeof(uint64_t));
  //     for (int k = 0; k < T; k++) {
  //       perms_ptr[i][j][k] = perms[i][j][k];
  //     }
  //   }
  // }

  uint64_t perm[N];
  PermReconstruct(d, N, T, n, t, (uint64_t *)perms, perm);
  
  for (int i = 0; i < N; i++) {
    std::cout << perm[i] << " ";
  }
  std::cout << std::endl;
}

void TestPermuteShare(char **argv) {
  int port, party;
  int length = 4;

  emp::parse_party_and_port(argv, &party, &port);

  // establish connection
  HighSpeedNetIO *io = new HighSpeedNetIO(party == ALICE ? nullptr : "127.0.0.1", 
                                          port,
                                          port + 1,
                                          false);


  uint64_t N = 16;
  uint64_t T = 4;
  int n = (int)(log2(N));
  int t = (int)(log2(T));
  int d = 2 * (int)ceil(n / t) - 1;
  int subperm_num = N / T;

  uint64_t perms[d][subperm_num][T] = 
    {{{3, 2, 0, 1}, {1, 3, 0, 2}, {1, 0, 3, 2}, {2, 0, 1, 3}}, 
     {{1, 3, 0, 2}, {1, 0, 3, 2}, {2, 0, 1, 3}, {3, 2, 0, 1}}, 
     {{1, 2, 0, 3}, {0, 1, 3, 2}, {0, 1, 2, 3}, {0, 2, 3, 1}}};
  uint64_t perm[N];

  PRG prg;
  block x[N];
  block out[N];
  prg.random_block(x, N);

  for (int i = 0; i < N; i++) {
    std::cout << x[i] << std::endl;
  }

  PermReconstruct(d, N, T, n, t, (uint64_t *)perms, perm);
  // PermuteShare(N, T, (uint64_t *)perms, perm, x, party, io, out);

  std::cout << "================================ [out]" << std::endl;
  for (int i = 0; i < N; i++) {
    std::cout << out[i] << std::endl;
  }

  delete io;
}

void TestSSShuffle(char **argv) {
  int port, party;
  int length = 4;

  emp::parse_party_and_port(argv, &party, &port);

  // establish connection
  HighSpeedNetIO *io = new HighSpeedNetIO(party == ALICE ? nullptr : "127.0.0.1", 
                                          port,
                                          port + 1,
                                          false);


  uint64_t N = 128;
  uint64_t T = 16;
  int n = (int)(log2(N));
  int t = (int)(log2(T));
  int d = 2 * (int)ceil(n / t) - 1;
  int subperm_num = N / T;

  block out[N];
  block x[N];
  for (int i = 0; i < N; i++) {
    x[i] = makeBlock(0, i);
  }

  block a[d * N];
  block b[d * N];
  block delta[d * N];
  block offset[(d - 1) * N];
  uint64_t perms[d * N];
  for (int i = 0; i < d * N; i++) {
    perms[i] = i % T;
  }

  std::random_device rd;
  std::mt19937 g(rd());

  for (int i = 0; i < d * subperm_num; i++) {
    std::shuffle(perms + i * T, perms + (i + 1) * T, g);
  }

  uint64_t subperms[d * N];
  uint64_t perm[N];

  Offline(N, T, perms, party, io, perm, subperms, a, b, offset, delta);
  std::cout << io->schannel->counter << std::endl;
  SecretSharedShuffle(N, T, party, io, x, a, b, offset, subperms, perm, delta, out);

  for (int i = 0; i < N; i++) {
    std::cout << out[i] << std::endl;
  }

  delete io;
}

}

int main(int argc, char **argv) {
  // OPVTest::TestOblivSetup(argv);
  // OPVTest::TestExpand(argv);
  // ShuffleTest::TestShareTranslation(argv);
  // ShuffleTest::TestPermReconstruct();
  // ShuffleTest::TestPermuteShare(argv);
  ShuffleTest::TestSSShuffle(argv);
  // TestIKNPOT(argv);
  // TestPermutation();
}