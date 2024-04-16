///為了觀察值
#include <iostream>
#include <string>
using namespace std;

// for simulation time
#include <time.h>

// Wall Clock Time Measurement
#include <sys/time.h>

#include "SobelFilter.h"
#include "Testbench.h"

// TIMEVAL STRUCT IS Defined ctime
// use start_time and end_time variables to capture
// start of simulation and end of simulation
struct timeval start_time, end_time;

// int main(int argc, char *argv[])
int sc_main(int argc, char **argv) {
  if ((argc < 3) || (argc > 4)) {
    cout << "No arguments for the executable : " << argv[0] << endl;
    cout << "Usage : >" << argv[0] << " in_image_file_name out_image_file_name"
         << endl;
    return 0;
  }

  //Create modules and signals
  Testbench tb("tb");           /// 建立一個名為"tb"的Testbanch module  就是定義名稱
  SobelFilter sobel_filter("sobel_filter");
  tb.initiator.i_skt(sobel_filter.t_skt); /// 將initiator sockert 與 sobel_filter socket bind 起來
  //sc_clock clk("clk", CLOCK_PERIOD, SC_NS);
  //sc_signal<bool> rst("rst");   /// doesn't have time concept and just like wire

/*  /// 將原本channel的概念換掉，改用tlm (socket的概念連接) 這些接線改寫在sobel_filter.cpp 的blocking transport
  //Create FIFO channels
  sc_fifo<unsigned char> r;
  sc_fifo<unsigned char> g;
  sc_fifo<unsigned char> b;
  sc_fifo<int> result;

  //Connect FIFO channels with modules
  tb.i_clk(clk);
  tb.o_rst(rst);
  sobel_filter.i_clk(clk);
  sobel_filter.i_rst(rst);
  tb.o_r(r);
  tb.o_g(g);
  tb.o_b(b);
  tb.i_result(result); // 跟sobel_filter.o_result(result) 對接
  sobel_filter.i_r(r);
  sobel_filter.i_g(g);
  sobel_filter.i_b(b);
  sobel_filter.o_result(result);
*/
  tb.read_bmp(argv[1]);
  double START,END;
  START = clock();
  sc_start();
  END = clock();
	cout << endl << "simulation time == " << (END - START) / CLOCKS_PER_SEC << " S" << endl;
  
  std::cout << "Simulated time == " << sc_core::sc_time_stamp() << std::endl;
  
  tb.write_bmp(argv[2]);
 
  return 0;
}
