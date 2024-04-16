// #include "Testbench.h"  // 已經不是跟test_bench 對接了 -> 改成使用blocking transport
/// for buffer
#include <iostream>
const int SIZE = 25;
double buffer[SIZE];

#include <cmath>
#include <iomanip>   /// get_xx 會用到？
#include "SobelFilter.h"
                                        ///sc_module(n) 定義一個module，名稱為:n，sc_module是定義module的語法
SobelFilter::SobelFilter(sc_module_name n) : sc_module(n), t_skt("t_skt"), base_offset(0)  {
  SC_THREAD(do_filter);                                  /// n 這個module的參數還有這些參數的初始值
  //sensitive << i_clk.pos();   ///Generate even when i_clk at positive trigger
  //dont_initialize();
  //reset_signal_is(i_rst, false);
  
  /// 將SobelFilter中的blocking_transport註冊到t_skt端，告訴系統當t_skt有request時，要使用SobelFolter內的blocking_transport處理
  t_skt.register_b_transport(this, &SobelFilter::blocking_transport);
}
/*
// sobel mask
const int mask[MASK_N][MASK_X][MASK_Y] = {{{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}},

                                          {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}}};
*/
// Gaussian blur                              x0  x1 x2 x3 x4
const double mask[MASK_N][MASK_X][MASK_Y] = {{{1, 4, 7, 4, 1},
                                              {4, 16, 26, 16, 4},
                                              {7, 26, 41, 26, 7},
                                              {4, 16, 26, 16, 4},
                                              {1, 4, 7, 4, 1}}};

void SobelFilter::do_filter() {
unsigned char r = 0, g = 0, b = 0;  /// for observed
int x = 0, y = 0;                   /// for buffer count

double grey = 0;
  while (true) {
    for (unsigned int i = 0; i < MASK_N; ++i) {  /// 初始化(initialization)
      val[i] = 0;
    }
    for (unsigned int v = 0; v < MASK_Y; ++v) {  /// v = 0~4
      for (unsigned int u = 0; u < MASK_X; ++u) {   ///[u,v] mask 掃 Gaussian blur 的內容(5*5) u代表哪一直行(column)，v代表哪一橫列(row)
        
        if(x == 0 || u == MASK_X-1 ) {  /// 初始讀取 以及 接下來 只讀最右邊的 MASK_X - 1 = 4
          r = i_r.read();   /// 從blocing transport read data (blocking transport 定義在SobelFilter.cpp)
          g = i_g.read();
          b = i_b.read();
          grey = (r + g + b) / 3;  ///initial or 新值放入buffer 接需要read
        }

        if (x != 0 && y != 0){   // buffer滿了，除了一開始，其他時間都是滿的
        /// u+v*5 是因為buffer是1*25的array，使用u+v*5可以將array視為5*5
        buffer[u+v*5]=buffer[u+v*5+1];       // 將x+1 column的值給 x
        }

        if (x == 0 && y == 0) buffer[u+5*v] = grey;  /// 0~24  一開始全放
        /// data reuse
        else if (u != MASK_X - 1 && x != 0) { /// 不在(pixel)第一行，且也還沒讀取到最右邊的值
          grey = buffer[u+5*v];   ///從buffer給值
        }
        else if (u == MASK_X - 1){
          buffer[u+5*v] = grey;   // 將新值放入buffer
        }
        for (unsigned int i = 0; i != MASK_N; ++i) {
          val[i] += grey * (mask[i][u][v]) / 273;
        }
      }
    }
    // counter for pixels 256 * 256
    x++;
    if(x == 256) {  // if the pixels is 512 need to change to 512;
      y++;
      x = 0;
      }
    double total = 0;
    for (unsigned int i = 0; i != MASK_N; ++i) {
      total += val[i] * val[i];
    }
    int result = (int)(std::sqrt(total)); ///將值轉為int
    o_result.write(result);
    //wait(10); //emulate module delay
  }
}


/// build blocking_transport
void SobelFilter::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay) {
  sc_dt::uint64 addr = payload.get_address();
  addr = addr - base_offset;
  unsigned char *mask_ptr = payload.get_byte_enable_ptr();
  unsigned char *data_ptr = payload.get_data_ptr();
  word buffer;
  delay = sc_time(1, SC_NS);
  switch (payload.get_command()) {
  case tlm::TLM_READ_COMMAND:
    switch (addr) {
    case SOBEL_FILTER_RESULT_ADDR:
      buffer.uint = o_result.read();
      break;
    case SOBEL_FILTER_CHECK_ADDR:
      buffer.uint = o_result.num_available();
      break;
    default:
      std::cerr << "Error! SobelFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
      break;
    }
    data_ptr[0] = buffer.uc[0];
    data_ptr[1] = buffer.uc[1];
    data_ptr[2] = buffer.uc[2];
    data_ptr[3] = buffer.uc[3];
    break;

  case tlm::TLM_WRITE_COMMAND:  ///將 test_bench送來的r, g, b透過blocking transport寫入do_filter(Gaussian_blur)
    switch (addr) {
    case SOBEL_FILTER_R_ADDR:
      if (mask_ptr[0] == 0xff) {
        i_r.write(data_ptr[0]);  /// data write from Testbench.cpp and transport by initiator
      }
      if (mask_ptr[1] == 0xff) {
        i_g.write(data_ptr[1]);
      }
      if (mask_ptr[2] == 0xff) {
        i_b.write(data_ptr[2]);
      }
      break;
    default:
      std::cerr << "Error! SobelFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
      break;
    }
    break;

  case tlm::TLM_IGNORE_COMMAND:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  default:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  }
  payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
}
