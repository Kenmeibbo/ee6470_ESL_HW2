#ifndef INITIATOR_H_
#define INITIATOR_H_
#include <systemc>
using namespace sc_core;

#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"

class Initiator : public sc_module {
public:
  tlm_utils::simple_initiator_socket<Initiator> i_skt;  /// 定義initiator socket as i_skt

  SC_HAS_PROCESS(Initiator);
  Initiator(sc_module_name n);


/// read from target socket(sobel_filter t_skt)  
  int read_from_socket(unsigned long int addr, unsigned char mask[],  /// write to socket 寫在 Testbench.cpp
                       unsigned char rdata[], int dataLen);
/// write to target socket(sobel_filter t_skt)
  int write_to_socket(unsigned long int addr, unsigned char mask[],
                      unsigned char wdata[], int dataLen);

  void do_trans(tlm::tlm_generic_payload &trans);
  tlm::tlm_generic_payload trans;
};
#endif
