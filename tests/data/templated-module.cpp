#include "systemc.h"

typedef float MY_FLOAT;
typedef MY_FLOAT MY_FLOAT2;

struct my_data {
  public:
    int ya_data;
};

SC_MODULE( non_template ) {
  int x;
  int y[10];

  sc_in<bool> blah;
	sc_in< sc_bv<8> > filter_input, filter_second_input;

  my_data mdt;
  sc_signal<int> array_signal[10];

  void ef() {}
  SC_CTOR(non_template) {
    SC_METHOD(ef) {}
  };
};

template <typename S, typename T>
SC_MODULE( test ){
    sc_in_clk clk;

    sc_in<S> inS;
    sc_in<T> inT;
    sc_in<T> outT;

    sc_in<S> inArrayS[10];

    sc_out<S> outS;
    sc_out<S> outArrayS[5];

    sc_signal<S> test_signal;
    sc_signal<S> data_array[10];
    sc_signal<T> not_data_array[10];


    // These are all others
    static const int ORDER = 5;
    static const double c[ORDER+1];
    sc_buffer<double> i[ORDER];

    // FieldDecl of an sc_module
    //non_template simple_module;

    void entry_function_1() {
        while(true) {
            // do nothing
        }
    }

    SC_CTOR( test ) {
        SC_METHOD(entry_function_1);
        sensitive << clk.pos();
    }
};

SC_MODULE(DUT) {
    sc_clock clk;
    sc_signal<int> sig1;

    test<int,double> test_instance;

    non_template non_template;

    test<MY_FLOAT2, double> test_float_double;

    SC_CTOR(DUT) : test_instance("testing"), non_template("non-templated-module-instance"), test_float_double("testing_float_double") {
    test_instance.clk(clk);
    test_instance.inS(sig1);
    test_instance.outS(sig1);

    }
 
};

int sc_main(int argc, char *argv[]) {
  DUT d("d");
   return 0;
}
