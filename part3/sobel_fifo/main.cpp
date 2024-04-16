///為了觀察值
#include <iostream>
#include <string>
using namespace std;

// for simulation time
#include <time.h>

// Wall Clock Time Measurement
#include <sys/time.h>
// for bus
#include "SimpleBus.h"
#include "SobelFilter.h"
#include "Testbench.h"

// for counter value in initiator.cpp
// #include "Initiator.cpp"
extern unsigned R_counter;
extern unsigned W_counter;

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
  SimpleBus<1, 1> bus("bus");
  bus.set_clock_period(sc_time(CLOCK_PERIOD, SC_NS));
  SobelFilter sobel_filter("sobel_filter");
  tb.initiator.i_skt(bus.t_skt[0]); /// initiator 與 bus bind 起來
  bus.setDecode(0, SOBEL_MM_BASE, SOBEL_MM_BASE + SOBEL_MM_SIZE - 1);
  bus.i_skt[0](sobel_filter.t_skt); /// bus 與 sobel_filter bind 起來



  tb.read_bmp(argv[1]);
  double START,END;
  START = clock();
  sc_start();
  END = clock();
	cout << endl << "simulation time == " << (END - START) / CLOCKS_PER_SEC << " S" << endl;
  
  std::cout << "Simulated time == " << sc_core::sc_time_stamp() << std::endl;
  
  std::cout << "NO. of Read ==" << R_counter << std::endl;
  std::cout << "NO. of Write ==" << W_counter << std::endl;

  tb.write_bmp(argv[2]);
 
  return 0;
}
