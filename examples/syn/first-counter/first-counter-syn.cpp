#include "systemc.h"
#include "first-counter.h"

SC_MODULE(DUT) {

  sc_signal<bool>   clock_sig;
  sc_signal<bool>   reset;
  sc_signal<bool>   enable;
  sc_signal<sc_uint<4> > counter_out;
  int i = 0;
 
  // Connect the DUT
  first_counter counter;

  SC_CTOR(DUT) : counter{"COUNTER"} {
    counter.clock(clock_sig);
    counter.reset(reset);
    counter.enable(enable);
    counter.counter_out(counter_out);
  }

};

int sc_main (int argc, char* argv[]) {
  DUT dut("dut_syn");

  return 0;

 }
