#include <systemc.h>
#include "avg.cpp"

SC_MODULE(DUT) {

	sc_in<bool> clock;
  sc_in<bool> reset;
  sc_in<sc_bv<8> > input_;
  sc_out<sc_bv<8> > output_;  // intput/output are keywords in verilog

	avg avg_unit;

  SC_CTOR(DUT) : avg_unit{"avger"} {

    avg_unit.clock(clock); 
    avg_unit.i_data(input_); 
    avg_unit.o_data(output_);
  }

};


int sc_main(int argc, char* argv[])	{
  DUT dut("syn_dut");

	return 0;
}
