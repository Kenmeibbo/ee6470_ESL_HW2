#ifndef FILTER_DEF_H_
#define FILTER_DEF_H_

const int MASK_N = 1; //sobel filter:2
const int MASK_X = 5; //sobel filter:3
const int MASK_Y = 5; //sobel filter:3

const int SOBEL_FILTER_R_ADDR = 0x00000000;
const int SOBEL_FILTER_RESULT_ADDR = 0x00000004;
const int SOBEL_FILTER_CHECK_ADDR = 0x00000008;

union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};

// Gaussian blue mask
const double mask[MASK_N][MASK_X][MASK_Y] = {{{1, 4, 7, 4, 1},
                                              {4, 16, 26, 16, 4},
                                              {7, 26, 41, 26, 7},
                                              {4, 16, 26, 16, 4},
                                              {1, 4, 7, 4, 1}}};
#endif
