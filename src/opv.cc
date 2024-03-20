#include "opv.h"

// #ifdef USE_EMP

void PrepareCorrelation(int cur_depth, int depth, block seed, block *str0, block *str1) {
  if (cur_depth == depth) {
    return;
  }

  block next_seeds[2];
  DLenPRG(seed, next_seeds);
  str0[cur_depth] ^= next_seeds[0];
  str1[cur_depth] ^= next_seeds[1];
  PrepareCorrelation(cur_depth + 1, depth, next_seeds[0], str0, str1);
  PrepareCorrelation(cur_depth + 1, depth, next_seeds[1], str0, str1);
}

#ifdef USE_EMP
void OblivSetup(uint64_t length, uint64_t x, int party, HighSpeedNetIO *io, block **seeds) {
  int depth = int(log2(length));
  IKNP<HighSpeedNetIO> *ote = new IKNP<HighSpeedNetIO>(io);
  if (party == ALICE) {
    *seeds = (block *)malloc(1 * sizeof(block));

    // generate root
    PRG prg;
    prg.random_block(*seeds, 1);
    // preparing data for the following ote
    block str0[depth], str1[depth];
    memset(str0, 0x00, depth * 16);
    memset(str1, 0x00, depth * 16);

    PrepareCorrelation(0, depth, **seeds, str0, str1);

    // start ote
    ote->send(str0, str1, depth);
  } else {
    *seeds = (block *)malloc(depth * sizeof(block));

    // parse input i to bool vector
    bool chosen_bit[depth];
    for (int i = depth - 1; i >= 0 ; i--) {
      chosen_bit[i] = (x & 1) ^ 1;
      x >>= 1;
    }

    // start ote
    // receive aggregated seeds
    // real seeds will be generated in Expand phase,
    // which is a little different from the paper
    ote->recv(*seeds, chosen_bit, depth);
  }
  io->flush();
  delete ote;
}
#elif defined USE_LIBOTE
void OblivSetup(uint64_t length, uint64_t x, int party, cp::Socket & chl, block **seeds) {
  int depth = int(log2(length));
  PRNG prg(sysRandomSeed());

  if (party == Role::Alice) {
    *seeds = (block *)malloc(1 * sizeof(block));

    IknpOtExtSender sender;

    // process base ot
    DefaultBaseOT base;
    BitVector bv(sender.baseOtCount());
    std::vector<block> base_msg(sender.baseOtCount());

    bv.randomize(prg);
    cp::sync_wait(base.receive(bv, base_msg, prg, chl));
    sender.setBaseOts(base_msg, bv);

    // process standard ot ext
    std::vector<std::array<block, 2>> send_msg(depth);
    prg.get(*seeds, 1);

    std::vector<block> str0(depth, ZeroBlock);
    std::vector<block> str1(depth, ZeroBlock);

    PrepareCorrelation(0, depth, **seeds, str0.data(), str1.data());

    for (int i = 0; i < depth; i++) {
      send_msg[i][0] = str0[i];
      send_msg[i][1] = str1[i];
    }
    
    cp::sync_wait(sender.sendChosen(send_msg, prg, chl));

  } else {
    *seeds = (block *)malloc(depth * sizeof(block));
    std::vector<block> recv_msg(depth);

    BitVector b(depth);
    IknpOtExtReceiver receiver;

    DefaultBaseOT base;
    std::vector<std::array<block, 2>> base_msg(receiver.baseOtCount());
    cp::sync_wait(base.send(base_msg, prg, chl));

    receiver.setBaseOts(base_msg);

    for (int i = depth - 1; i >= 0; i--) {
      b[i] = (x & 1) ^ 1;
      x >>= 1;
    }

    cp::sync_wait(receiver.receiveChosen(b, recv_msg, prg, chl));

    memcpy(*seeds, recv_msg.data(), depth * sizeof(block));
  }
}
#endif


void SimpleExpand(int cur_depth, int depth, int index, block seed, block *v) {
  block next_seeds[2];
  DLenPRG(seed, next_seeds);

  if (cur_depth == depth - 1) {  
    v[index] = next_seeds[0];
    v[index + 1] = next_seeds[1];
    return;
  }

  int offset = 1 << (depth - cur_depth - 1);
  SimpleExpand(cur_depth + 1, depth, index, next_seeds[0], v);
  SimpleExpand(cur_depth + 1, depth, index + offset, next_seeds[1], v);
}

int CheckOnPath(block x, block y) {
  uint64_t *x_ = (uint64_t *)&x;
  uint64_t *y_ = (uint64_t *)&y;
  return ((x_[0] == y_[0]) & (x_[1] == y_[1]));
}

void PrintQueue(std::deque<block> x) {
  std::cout << "======= [Queue]" << std::endl;
  for (int i = 0; i < x.size(); i++) {
    std::cout << x[i] << std::endl;
  }
  std::cout << "======= [Queue End]" << std::endl;
}

void PunctureExpand(int depth, uint64_t x, block *seeds, block *v) {
  int size = 0;
  int cur_depth = 1;

  bool bits[depth];

  block seed, agg_mask;
  block next_seeds[2];

  std::deque<block> q;

  // convert x to bit vector
  for (int i = depth - 1; i >= 0 ; i--) {
    bits[i] = (x & 1) ^ 1;
    x >>= 1;
  }

  int path = bits[0];
  
  // push the first layer
  if (bits[0]) {
#ifdef USE_EMP
    q.push_back(zero_block);
#elif defined USE_LIBOTE
    q.push_back(ZeroBlock);
#endif
    q.push_back(*seeds);
  } else {
    q.push_back(*seeds);
#ifdef USE_EMP
    q.push_back(zero_block);
#elif defined USE_LIBOTE
    q.push_back(ZeroBlock);
#endif
  }

  while (cur_depth < depth) {
    size = q.size();
#ifdef USE_EMP
    agg_mask = zero_block;
#elif defined USE_LIBOTE
    agg_mask = ZeroBlock;
#endif
    // process each layer
    for (int i = 0; i < size; i++) {
      seed = q.front();
      q.pop_front();
#ifdef USE_EMP
      if (!CheckOnPath(seed, zero_block)) { // not on-path node
#elif defined USE_LIBOTE
      if (!CheckOnPath(seed, ZeroBlock)) {
#endif
        // evaluate prg and push directly
        DLenPRG(seed, next_seeds);
        q.push_back(next_seeds[0]);
        q.push_back(next_seeds[1]);
        
        // in addition, we need aggregate all seeds according to each layer's bit
        agg_mask ^= next_seeds[bits[cur_depth]];
      } else {
        // zero_block plays a placeholder of punctured value
        if (bits[cur_depth]) {
#ifdef USE_EMP
          q.push_back(zero_block);
#elif defined USE_LIBOTE
          q.push_back(ZeroBlock);
#endif
          q.push_back(seeds[cur_depth]);
        } else {
          q.push_back(seeds[cur_depth]);
#ifdef USE_EMP
          q.push_back(zero_block);
#elif defined USE_LIBOTE
          q.push_back(ZeroBlock);
#endif
        }
      }
    }
    // correct the seed 
    path = ((path ^ 1) << 1) ^ bits[cur_depth];
    q[path] ^= agg_mask;
    cur_depth++;
  }

  for (int i = 0; i < q.size(); i++) {
    v[i] = q[i];
  }
}

void Expand(uint64_t length, uint64_t x, block *seeds, int party, block *v) {
  int depth = int(log2(length));
#ifdef USE_EMP
  if (party == ALICE) {
#elif defined USE_LIBOTE
  if (party == Role::Alice) {
#endif
    SimpleExpand(0, depth, 0, *seeds, v);
  } else {
    PunctureExpand(depth, x, seeds, v);
  }
}

// #endif