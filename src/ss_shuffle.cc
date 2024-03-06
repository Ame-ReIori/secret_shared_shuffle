#include "ss_shuffle.h"

/**
 * @param perm: permutation information
 * @param length: length of permutation
 * @param party: party
 * @param io: io
 * @param out: the pointer of output value.
 */
void ShareTranslation(uint64_t *perm, int length, int party, HighSpeedNetIO *io,
                      block *a, block *b, block *delta) {
  // alice has two outputs, a and b, while bob has only one, delta.
  block v[length][length];
  block **seeds = (block **)malloc(length * sizeof(block *));

  if (party == ALICE) {
    // ALICE gets the whole matrix and generate vector a and b
    memset(a, 0, length * sizeof(block));
    memset(b, 0, length * sizeof(block));

    for (int i = 0; i < length; i++) {
      OblivSetup(length, -1, party, io, seeds + i);
      Expand(length, -1, *(seeds + i), party, v[i]);
    }

    for (int i = 0; i < length; i++) {
      for (int j = 0; j < length; j++) {
        a[i] += v[j][i];
        b[i] += v[i][j];
      }
    }

  } else {
    memset(delta, 0, length * sizeof(block));

    for (int i = 0; i < length; i++) {
      OblivSetup(length, perm[i], party, io, seeds + i);
      Expand(length, perm[i], *(seeds + i), party, v[i]);
    }

    for (int i = 0; i < length; i++) {
      for (int j = 0; j < length; j++) {
        delta[i] += (v[i][j] - v[j][perm[i]]);
      }
    }
  }
}

template<typename T>
T *LocateVector(T *vecs, int x, int y, int z, int i, int j) {
  return vecs + i * y * z + j * z;
}

void SubPerm(uint64_t T, int t,
             int prefix_len, uint64_t prefix, 
             int suffix_len, uint64_t suffix, 
             uint64_t *perm, uint64_t *v) {

  uint64_t tmp[T];
  uint64_t index;
  for (int i = 0; i < T; i++) {
    index = (prefix << (t + suffix_len)) | (perm[i] << suffix_len) | suffix;
    tmp[i] = v[index];
  }

  for (int i = 0; i < T; i++) {
    index = (prefix << (t + suffix_len)) | (i << suffix_len) | suffix;
    v[index] = tmp[i];
  }
}

template<typename T>
void SimplePerm(uint64_t *perm, uint64_t length, T *v) {
  T tmp[length];
  
  for (int i = 0; i < length; i++) {
    tmp[i] = v[perm[i]];
  }

  memcpy(v, tmp, length * sizeof(T));
}

void PermReconstruct(int d, uint64_t N, uint64_t T, int n, int t, uint64_t *perms, uint64_t *perm) {
  int prefix_len = 0;
  int suffix_len = n - t;
  int subperm_num = N / T;
  uint64_t *subperm;
  uint64_t index, prefix, suffix, counter;

  // initial perm
  for (int i = 0; i < N; i++) {
    perm[i] = i;
  }
  // process first d / 2 layers
  for (int i = 0; i < d / 2; i++) { 
    counter = 0;
    for (int j = 0; j < subperm_num; j++) {
      prefix = counter >> suffix_len;
      suffix = counter & ((1 << suffix_len) - 1);
      subperm = LocateVector<uint64_t>(perms, d, subperm_num, T, i, j);
      SubPerm(T, t, prefix_len, prefix, suffix_len, suffix, subperm, perm);
      counter++;
    }
    prefix_len += t;
    suffix_len -= t;
  }
  
  // process middle layer
  counter = 0;
  for (int j = 0; j < subperm_num; j++) {
    prefix = counter >> suffix_len;
    suffix = counter & ((1 << suffix_len) - 1);
    subperm = LocateVector<uint64_t>(perms, d, subperm_num, T, d / 2, j);
    SubPerm(T, t, prefix_len, prefix, suffix_len, suffix, subperm, perm);
    counter++;
  }

  prefix_len -= t;
  suffix_len += t;

  // process the last d / 2 layers
  for (int i = d / 2 + 1; i < d; i++) {
    counter = 0;
    for (int j = 0; j < subperm_num; j++) {
      prefix = counter >> suffix_len;
      suffix = counter & ((1 << suffix_len) - 1);
      subperm = LocateVector<uint64_t>(perms, d, subperm_num, T, i, j);
      SubPerm(T, t, prefix_len, prefix, suffix_len, suffix, subperm, perm);
      counter++;
    }
    prefix_len -= t;
    suffix_len += t;
  }
}

void SubPermReconstruct(int layer, int d, uint64_t N, uint64_t T, int n, int t, uint64_t *perms, uint64_t *perm) {
  int prefix_len, suffix_len, prefix, suffix, index;
  int counter = 0;
  int subperm_num = N / T;
  if (layer < d / 2) {
    prefix_len = layer * t;
    suffix_len = n - (layer + 1) * t;
  } else if (layer == d / 2) {
    prefix_len = n - t;
    suffix_len = 0;
  } else {
    prefix_len = n - (layer - d / 2 + 1) * t;
    suffix_len = (layer - d / 2) * t;
  }

  // initial perm
  for (int i = 0; i < N; i++) {
    perm[i] = i;
  }

  counter = 0;
  for (int i = 0; i < subperm_num; i++) {
    prefix = counter >> suffix_len;
    suffix = counter & ((1 << suffix_len) - 1);
    SubPerm(T, t, prefix_len, prefix, suffix_len, suffix, perms + i * T, perm);
    counter++;
  }
}

void Reallocate(block *v, int layer, int d, uint64_t N, uint64_t T, int n, int t) {
  int prefix_len, suffix_len, prefix, suffix, index;
  int counter = 0;
  if (layer < d / 2) {
    prefix_len = layer * t;
    suffix_len = n - (layer + 1) * t;
  } else if (layer == d / 2) {
    prefix_len = n - t;
    suffix_len = 0;
  } else {
    prefix_len = n - (layer - d / 2 + 1) * t;
    suffix_len = (layer - d / 2) * t;
  }

  block tmp[N];
  for (int i = 0; i < N; i++) {
    prefix = counter >> suffix_len;
    suffix = counter & ((1 << suffix_len) - 1);
    index = (prefix << (suffix_len + t)) 
          | ((i % T) << suffix_len)
          | suffix;
    tmp[index] = v[i];
    if ((i + 1) % T == 0) counter++; 
  }

  memcpy(v, tmp, N * sizeof(block));
}

void Offline(uint64_t N, uint64_t T, uint64_t *perms, uint64_t party, HighSpeedNetIO *io,
             uint64_t *perm, block *a, block * b, block *delta) {
  int n = (int)(log2(N));
  int t = (int)(log2(T));
  int d = 2 * (int)ceil(n / t) - 1;
  int subperm_num = N / T;
  uint64_t *subperm;
  uint64_t subperms[d * N];

  block as[d * N];
  block bs[d * N];
  block deltas[d * N];
  block offset[(d - 1) * N];
  block offset_[(d - 1) * N];

  PermReconstruct(d, N, T, n, t, perms, perm);
  if (party == ALICE) {
    for (int i = 0; i < d; i++) {
      for (int j = 0; j < subperm_num; j++) {
        ShareTranslation(nullptr, T, ALICE, io,
                        LocateVector<block>(as, d, subperm_num, T, i, j), 
                        LocateVector<block>(bs, d, subperm_num, T, i, j), 
                        nullptr);
              
        subperm = LocateVector<uint64_t>(perms, d, subperm_num, T, i, j);
        ShareTranslation(subperm, T, BOB, io,
                        nullptr, nullptr, 
                        LocateVector<block>(deltas, d, subperm_num, T, i, j));
      }
    }
  } else {
    for (int i = 0; i < d; i++) {
      for (int j = 0; j < subperm_num; j++) {
        subperm = LocateVector<uint64_t>(perms, d, subperm_num, T, i, j);
        ShareTranslation(subperm, T, BOB, io,
                        nullptr, nullptr, 
                        LocateVector<block>(deltas, d, subperm_num, T, i, j));
        ShareTranslation(nullptr, T, ALICE, io,
                        LocateVector<block>(as, d, subperm_num, T, i, j), 
                        LocateVector<block>(bs, d, subperm_num, T, i, j), 
                        nullptr);
      }
    }
  }
  
  // reallocate a and b
  for (int i = 0; i < d; i++) {
    Reallocate(as + i * N, i, d, N, T, n, t);
    Reallocate(bs + i * N, i, d, N, T, n, t);
    Reallocate(deltas + i * N, i, d, N, T, n, t);
    SubPermReconstruct(i, d, N, T, n, t,
                       perms + i * N, 
                       subperms + i * N);
  }

  // calculate delta = a ^ {i + 1} - b^i
  for (int i = 0; i < (d - 1) * N; i++) {
    offset[i] = as[N + i] - bs[i];
  }

  // obtain the final delta
  if (party == ALICE) {
    io->send_block(offset, (d - 1) * N);
    io->recv_block(offset_, (d - 1) * N);
  } else {
    io->recv_block(offset_, (d - 1) * N);
    io->send_block(offset, (d - 1) * N);
  }
  
  for (int i = 0; i < N; i++) {
    delta[i] = deltas[i] + offset_[i];
  }

  for (int i = N; i < (d - 1) * N; i += N) {
    SimplePerm<block>(subperms + i, N, delta);

    for (int j = 0; j < N; j++) {
      delta[j] += (deltas[i + j] + offset_[i + j]);
    }
  }

  SimplePerm<block>(subperms + (d - 1) * N, N, delta);
  for (int i = 0; i < N; i++) {
    a[i] = as[i];
    b[i] = bs[(d - 1) * N + i];
    delta[i] += deltas[(d - 1) * N + i];
  }
}

void PermuteShare(uint64_t N, uint64_t T, 
                  uint64_t *perm, block *delta,
                  block *x, block *a, block *b,
                  uint64_t party, HighSpeedNetIO *io, 
                  block *out) {
  // implementation of single round permute+share
  int n = (int)(log2(N));
  int t = (int)(log2(T));
  int d = 2 * (int)ceil(n / t) - 1;

  // suppose T|N

  block m[N];
  block w[N];
  
  if (party == ALICE) {
    // calculate x + a^1
    for (int i = 0; i < N; i++) {
      m[i] = x[i] + a[i];
    }

    io->send_block(m, N);
    io->recv_block(w, N);

    for (int i = 0; i < N; i++) {
      out[i] = w[i] - b[i];
    }
  } else {
    // already computes the subpermutation, stored in perms
    PRG prg;
    prg.random_block(w, N);
    io->recv_block(m, N);
    io->send_block(w, N);

    SimplePerm(perm, N, m);
    for (int i = 0; i < N; i++) {
      out[i] = m[i] + delta[i] - w[i];
    }
  }
}

void SecretSharedShuffle(uint64_t N, uint64_t T, uint64_t party, HighSpeedNetIO *io, 
                         block *x, uint64_t *perm, block *delta, block *a, block *b,
                         block *out) {
  block out0[N];

  int n = (int)(log2(N));
  int t = (int)(log2(T));
  int d = 2 * (int)ceil(n / t) - 1;
  int subperm_num = N / T;


  if (party == ALICE) {
    PermuteShare(N, T, nullptr, nullptr, x, a, b, ALICE, io, out0);
    PermuteShare(N, T, perm, delta, nullptr, nullptr, nullptr, BOB, io, out);

    SimplePerm(perm, N, out0);
    for (int i = 0; i < N; i++) {
      out[i] += out0[i];
    }
  } else { // bob
    PermuteShare(N, T, perm, delta, nullptr, nullptr, nullptr, BOB, io, out0);
    SimplePerm<block>(perm, N, x);
    for (int i = 0; i < N; i++) {
      x[i] += out0[i];
    }
    PermuteShare(N, T, nullptr, nullptr, x, a, b, ALICE, io, out);
  }
}