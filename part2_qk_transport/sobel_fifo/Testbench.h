#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include <string>
using namespace std;

#include <systemc>
using namespace sc_core;
using namespace sc_dt; /// system c data type
#include "tlm_utils/tlm_quantumkeeper.h"

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include <cstdint>
#include "Initiator.h"
#include "filter_def.h"

const int WHITE = 255;
const int BLACK = 0;
const int THRESHOLD = 90;

class Testbench : public sc_module {
public:
/*
  sc_in_clk i_clk;
  sc_out<bool> o_rst;
  sc_fifo_out<unsigned char> o_r;
  sc_fifo_out<unsigned char> o_g;
  sc_fifo_out<unsigned char> o_b;
  sc_fifo_in<int> i_result;
*/
  /// 實例化(instantiate) Initiator(type) 的物件 initiator
  Initiator initiator;
  ///printf("i_result%d\n",i_result);

  SC_HAS_PROCESS(Testbench);

  Testbench(sc_module_name n);
  ~Testbench() = default;

  int read_bmp(string infile_name);
  int write_bmp(string outfile_name);

  unsigned int get_width() { return width; }

  unsigned int get_height() { return height; }

  unsigned int get_width_bytes() { return width_bytes; }

  unsigned int get_bytes_per_pixel() { return bytes_per_pixel; }

  unsigned char *get_source_image() { return source_bitmap; }
  unsigned char *get_target_image() { return target_bitmap; }

private:
  unsigned int input_rgb_raw_data_offset;
  const unsigned int output_rgb_raw_data_offset;
  int width;
  int height;
  unsigned int width_bytes;
  unsigned char bits_per_pixel;
  unsigned short bytes_per_pixel;
  unsigned char *source_bitmap;
  unsigned char *target_bitmap;

  void do_sobel();
  tlm_utils::tlm_quantumkeeper m_qk; // Quantum keeper for temporal decoupling
};
#endif
