typedef struct packed {
  logic[31:0] op1;
  logic[31:0] op2;
} op_t;
typedef enum logic[31:0] {
  ADD, SUB
} opcode_t;

interface MyInterface;
  op_t        operands;
  opcode_t    opcode;
  logic[31:0] res;
  modport producer(
    output operands, opcode,
    input res
  );

  modport consumer(
    input operands, opcode,
    output res
  );
endinterface

module producer(
  MyInterface.producer interf,
  input opcode_t opcode
);
  always @(*) begin
    interf.operands.op1 = 32'h1234;
    interf.operands.op2 = 32'h4321;
    interf.opcode = opcode;
  end
endmodule

module consumer(
  MyInterface.consumer interf
);
  always @(*) begin
    if(interf.opcode == ADD) begin
      interf.res = interf.operands.op1 + interf.operands.op2;
    end else if(interf.opcode == SUB) begin
      interf.res = interf.operands.op1 - interf.operands.op2;
    end
  end
endmodule

module top(
  input opcode_t opcode,
  output [31:0] res
);
  MyInterface interf();
  producer p(
    .interf(interf),
    .opcode(opcode)
  );
  consumer c(
    .interf(interf)
  );
  assign res = interf.res;
endmodule

