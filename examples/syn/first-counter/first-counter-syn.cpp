#include "systemc.h"
#include "first-counter.h"

SC_MODULE(DUT) {

  sc_in<bool>   clock;
  sc_in<bool>   reset;
  sc_in<bool>   enable;
  sc_out<sc_uint<4> > counter_out;
  int i = 0;
 
  // Connect the DUT
  first_counter counter;

  SC_CTOR(DUT) : counter{"COUNTER"} {
    counter.clock(clock);
    counter.reset(reset);
    counter.enable(enable);
    counter.counter_out(counter_out);
  }

};

int sc_main (int argc, char* argv[]) {
  DUT dut("dut_syn");

  return 0;

 }
