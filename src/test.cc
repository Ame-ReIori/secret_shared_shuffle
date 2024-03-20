#include "opv.h"
#include "ss_shuffle.h"

#include <iostream>
#include <vector>
#include <chrono>

#ifdef USE_EMP
using namespace emp;
#endif

#ifdef USE_LIBOTE
using namespace osuCrypto;
#endif


#include <cryptoTools/Crypto/PRNG.h>
#include <libOTe/Base/BaseOT.h>
#include <libOTe/TwoChooseOne/Iknp/IknpOtExtReceiver.h>
#include <libOTe/TwoChooseOne/Iknp/IknpOtExtSender.h>
#include <coproto/Socket/AsioSocket.h>

void TestLibOTeIKNP(char **argv) {
  auto ip = std::string("127.0.0.1");
  auto port = 8913;
  int party = atoi(argv[1]);
  std::string socket = ip + ":" + std::to_string(port);
  std::string hint = "libOTe_chl";

  uint64_t nOT = 1 << 10;
  osuCrypto::PRNG prng(osuCrypto::sysRandomSeed());
  if (party == 0) {
    auto chl = osuCrypto::cp::asioConnect(socket, true);
    osuCrypto::IknpOtExtSender sender;

    // prepare data
    std::vector<std::array<osuCrypto::block, 2>> send_msg(nOT);

    prng.get(send_msg.data(), send_msg.size());
    
    osuCrypto::DefaultBaseOT base;
    osuCrypto::BitVector bv(sender.baseOtCount());
    std::vector<osuCrypto::block> base_msg(sender.baseOtCount());
    bv.randomize(prng);

    osuCrypto::cp::sync_wait(base.receive(bv, base_msg, prng, chl));
    sender.setBaseOts(base_msg, bv);

    osuCrypto::cp::sync_wait(sender.sendChosen(send_msg, prng, chl));

    // for (int i = 0; i < nOT; i++) {
    //   std::cout << send_msg[i][0] << "\t" << send_msg[i][1] << std::endl;
    // }

    std::cout << "===== communication statistics =====" << std::endl;
    std::cout << "Received: " << chl.bytesReceived() << " B" << std::endl;
    std::cout << "Sent: " << chl.bytesSent() << " B" << std::endl;

  } else {
    auto chl = osuCrypto::cp::asioConnect(socket, false);
    osuCrypto::IknpOtExtReceiver receiver;
    
    std::vector<osuCrypto::block> recv_msg(nOT);
    osuCrypto::BitVector b(nOT);
    
    b.randomize(prng);

    osuCrypto::DefaultBaseOT base;
    std::vector<std::array<osuCrypto::block, 2>> base_msg(receiver.baseOtCount());

    osuCrypto::cp::sync_wait(base.send(base_msg, prng, chl));
    receiver.setBaseOts(base_msg);

    osuCrypto::cp::sync_wait(receiver.receiveChosen(b, recv_msg, prng, chl));

    // for (int i = 0; i < nOT; i++) {
    //   std::cout << (int)b[i] << "\t" << recv_msg[i] << std::endl;
    // }

  }
}

#ifdef USE_EMP
void TestIKNPOT(char **argv) {
  int port, party;
  int nOT = 1 << 10;

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
  delete iknp;
  delete io;
}
#endif

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
#ifdef USE_EMP
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
#elif defined USE_LIBOTE
  int party = atoi(argv[1]);
  std::string ip = "127.0.0.1";
  int port = 8442;
  std::string socket = ip + ":" + std::to_string(port);

  block *seeds;

  auto chl = cp::asioConnect(socket, party ^ 1);

  OblivSetup(8, 3, party, chl, &seeds);
  
#endif
}

void TestExpand(char **argv) {
#ifdef USE_EMP
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
#elif defined USE_LIBOTE
  int party = atoi(argv[1]);
  std::string ip = "127.0.0.1";
  int port = 8442;
  std::string socket = ip + ":" + std::to_string(port);

  auto chl = cp::asioConnect(socket, party ^ 1);

  int length = 8;
  int x = 3;
  block v[length];
  block *seeds;

  OblivSetup(length, x, party, chl, &seeds);
  Expand(length, x, seeds, party, v);

  std::cout << "============= " << (party == Role::Alice ? "Alice" : "Bob") << std::endl;

  for (int i = 0; i < length; i++) {
    std::cout << v[i] << std::endl;
  }

  std::cout << "==============================" << std::endl;
#endif
}

}

namespace ShuffleTest {

void TestShareTranslation(char **argv) {
#ifdef USE_EMP
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

  if (party == ALICE) {
    std::cout << "========================== output a" << std::endl;
    for (int i = 0; i < length; i++) {
      std::cout << a[i] << std::endl;
    }

    std::cout << "========================== output b" << std::endl;
    for (int i = 0; i < length; i++) {
      std::cout << b[i] << std::endl;
    }
  } else {
    std::cout << "========================== output delta" << std::endl;
    for (int i = 0; i < length; i++) {
      std::cout << delta[i] << std::endl;
    }
  }

  delete io;
#elif defined USE_LIBOTE
  int party = atoi(argv[1]);
  std::string ip = "127.0.0.1";
  int port = 8442;
  std::string socket = ip + ":" + std::to_string(port);

  auto chl = cp::asioConnect(socket, party ^ 1);

  int length = 4;
  uint64_t perm[4];
  perm[0] = 3;
  perm[1] = 1;
  perm[2] = 0;
  perm[3] = 2;

  block a[length], b[length], delta[length];

  ShareTranslation(perm, length, party, chl, a, b, delta);

  if (party == Role::Alice) {
    std::cout << "========================== output a" << std::endl;
    for (int i = 0; i < length; i++) {
      std::cout << a[i] << std::endl;
    }

    std::cout << "========================== output b" << std::endl;
    for (int i = 0; i < length; i++) {
      std::cout << b[i] << std::endl;
    }
  } else {
    std::cout << "========================== output delta" << std::endl;
    for (int i = 0; i < length; i++) {
      std::cout << delta[i] << std::endl;
    }
  }
#endif
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

  uint64_t perm[N];
  PermReconstruct(d, N, T, n, t, (uint64_t *)perms, perm);
  
  for (int i = 0; i < N; i++) {
    std::cout << perm[i] << " ";
  }
  std::cout << std::endl;
}

void TestPermuteShare(char **argv) {
#ifdef USE_EMP
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

  PRG prg;
  block x[N];
  block out[N];

  for (int i = 0; i < N; i++) {
    x[i] = makeBlock(0, i);
  }

  block a[N];
  block b[N];
  block delta[N];
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

  Offline(N, T, perms, party, io, perm, a, b, delta);

  PermuteShare(N, T, perm, delta, x, a, b, party, io, out);

  if (party == ALICE) {
    io->send_block(out, N);
  } else {
    std::cout << "============================= [perm]" << std::endl;
    for (int i = 0; i < N; i++) {
      std::cout << perm[i] << " ";
    }
    
    std::cout << "\n============================= [out]" << std::endl;
    block out_[N];
    io->recv_block(out_, N);
    for (int i = 0; i < N; i++) {
      std::cout << out[i] + out_[i] << std::endl;;
    }
  }

  delete io;
#elif defined USE_LIBOTE
  int party = atoi(argv[1]);
  std::string ip = "127.0.0.1";
  int port = 8442;
  std::string socket = ip + ":" + std::to_string(port);

  auto chl = cp::asioConnect(socket, party ^ 1);

  uint64_t N = 16;
  uint64_t T = 4;
  int n = (int)(log2(N));
  int t = (int)(log2(T));
  int d = 2 * (int)ceil(n / t) - 1;
  int subperm_num = N / T;

  block x[N];
  block out[N];

  for (int i = 0; i < N; i++) {
    x[i] = toBlock(0, i);
  }

  block a[N];
  block b[N];
  block delta[N];
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

  Offline(N, T, perms, party, chl, perm, a, b, delta);

  PermuteShare(N, T, perm, delta, x, a, b, party, chl, out);

  if (party == Role::Alice) {
    std::vector<block> out_view(out, out + N);
    cp::sync_wait(chl.send(out_view));
  } else {
    std::cout << "============================= [perm]" << std::endl;
    for (int i = 0; i < N; i++) {
      std::cout << perm[i] << " ";
    }
    
    std::cout << "\n============================= [out]" << std::endl;
    std::vector<block> out_view(N);
    block out_[N];
    cp::sync_wait(chl.recv(out_view));
    memcpy(out_, out_view.data(), N * sizeof(block));
    for (int i = 0; i < N; i++) {
      std::cout << out[i] + out_[i] << std::endl;;
    }
  }
#endif
}

void TestSSShuffle(char **argv) {
#ifdef USE_EMP
  int port, party;
  int length = 4;

  emp::parse_party_and_port(argv, &party, &port);

  // establish connection
  HighSpeedNetIO *io = new HighSpeedNetIO(party == ALICE ? nullptr : "127.0.0.1", 
                                          port,
                                          port + 1,
                                          false);


  uint64_t N = 1 << 12;
  uint64_t T = 128;
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

  // std::cout << "==================== subperms" << std::endl;
  // for (int i = 0; i < d * N; i++) {
  //   std::cout << perms[i] << " ";
  // }
  // std::cout << std::endl;

  auto start = std::chrono::system_clock::now();
  Offline(N, T, perms, party, io, perm, a, b, delta);
  auto end = std::chrono::system_clock::now();
  auto dura = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "time: " << double(dura.count()) * std::chrono::milliseconds::period::num / std::chrono::milliseconds::period::den << "s" << std::endl;

  start = std::chrono::system_clock::now();
  SecretSharedShuffle(N, T, party, io, x, perm, delta, a, b, out);
  end = std::chrono::system_clock::now();
  dura = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "time: " << double(dura.count()) * std::chrono::milliseconds::period::num / std::chrono::milliseconds::period::den << "s" << std::endl;

  // if (party == ALICE) {
  //   for (int i = 0; i < N; i++) {
  //     std::cout << perm[i] << " ";
  //   }
  //   std::cout << std::endl;
  //   io->send_block(out, N);
  // } else {

  //   for (int i = 0; i < N; i++) {
  //     std::cout << perm[i] << " ";
  //   }
  //   std::cout << std::endl;
    
  //   std::cout << "\n============================= [out]" << std::endl;
  //   block out_[N];
  //   io->recv_block(out_, N);
  //   for (int i = 0; i < N; i++) {
  //     std::cout << out[i] + out_[i] << std::endl;;
  //   }
  // }

  delete io;
#elif defined USE_LIBOTE
  int party = atoi(argv[1]);
  std::string ip = "127.0.0.1";
  int port = 8442;
  std::string socket = ip + ":" + std::to_string(port);

  auto chl = cp::asioConnect(socket, party ^ 1);

  uint64_t N = 16;
  uint64_t T = 4;
  int n = (int)(log2(N));
  int t = (int)(log2(T));
  int d = 2 * (int)ceil(n / t) - 1;
  int subperm_num = N / T;

  block out[N];
  block x[N];
  for (int i = 0; i < N; i++) {
    x[i] = toBlock(0, i);
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

  auto start = std::chrono::system_clock::now();
  Offline(N, T, perms, party, chl, perm, a, b, delta);
  auto end = std::chrono::system_clock::now();
  auto dura = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "time: " << double(dura.count()) * std::chrono::milliseconds::period::num / std::chrono::milliseconds::period::den << "s" << std::endl;

  start = std::chrono::system_clock::now();
  SecretSharedShuffle(N, T, party, chl, x, perm, delta, a, b, out);
  end = std::chrono::system_clock::now();
  dura = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "time: " << double(dura.count()) * std::chrono::milliseconds::period::num / std::chrono::milliseconds::period::den << "s" << std::endl;

  if (party == Role::Alice) {
    for (int i = 0; i < N; i++) {
      std::cout << perm[i] << " ";
    }
    std::cout << std::endl;
    std::vector<block> out_view(out, out + N);
    cp::sync_wait(chl.send(out_view));
  } else {

    for (int i = 0; i < N; i++) {
      std::cout << perm[i] << " ";
    }
    std::cout << std::endl;
    
    std::cout << "\n============================= [out]" << std::endl;
    std::vector<block> out_view(N);
    block out_[N];
    cp::sync_wait(chl.recv(out_view));
    memcpy(out_, out_view.data(), N * sizeof(block));
    for (int i = 0; i < N; i++) {
      std::cout << out[i] + out_[i] << std::endl;;
    }
  }
#endif
}

}

int main(int argc, char **argv) {
  // OPVTest::TestOblivSetup(argv);
  // OPVTest::TestExpand(argv);
  // ShuffleTest::TestShareTranslation(argv);
  // ShuffleTest::TestPermReconstruct();
  // ShuffleTest::TestPermuteShare(argv);
  ShuffleTest::TestSSShuffle(argv);
  // TestLibOTeIKNP(argv);
  // TestIKNPOT(argv);
  // TestPermutation();
}