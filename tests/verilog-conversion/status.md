## Instructions to generate the report
In systemc-clang build folder: `python -B run-verilog-tests.py -m verilog`
and this will generate a `log.csv` file

Then use the `report-from-csv.py` in this folder: `python -B report-from-csv.py --input-csv path/to/log.csv --output-md status.md`
| Test Name | systemc-clang/xlat | convert.py | hdl.txt syntax | transformation |
|:---------:|:------------------:|:----------:|:--------------:|:--------------:|
|test_ex_verilog[1]|✔︎ OK|✔︎ OK|✔︎ OK|✔︎ OK|
|test_ex_verilog[2]|✘ Failed|✘ Failed|✘ Failed|✘ Failed|
|test_ex_verilog[3]|✔︎ OK|✔︎ OK|✔︎ OK|✔︎ OK|
|test_ex_verilog[4]|✔︎ OK|✔︎ OK|✔︎ OK|✔︎ OK|
|test_ex_verilog[5]|✔︎ OK|✔︎ OK|✔︎ OK|✘ Failed|
|test_ex_verilog[6]|✔︎ OK|✔︎ OK|✔︎ OK|✘ Failed|
|test_ex_verilog[7]|✘ Failed|✘ Failed|✘ Failed|✘ Failed|
|test_ex_verilog[8]|✘ Failed|✘ Failed|✘ Failed|✘ Failed|
|test_ex_verilog[9]|✘ Failed|✘ Failed|✘ Failed|✘ Failed|
|test_ex_verilog[10]|✘ Failed|✘ Failed|✘ Failed|✘ Failed|
|test_ex_verilog[12]|✘ Failed|✘ Failed|✘ Failed|✘ Failed|
|test_ex_verilog[13]|✔︎ OK|✔︎ OK|✔︎ OK|✔︎ OK|
|test_ex_verilog[14]|✘ Failed|✘ Failed|✘ Failed|✘ Failed|
|test_ex_verilog[15]|✘ Failed|✘ Failed|✘ Failed|✘ Failed|
|test_zfp1_verilog|✔︎ OK|✔︎ OK|✔︎ OK|✔︎ OK|
|test_zfp2_verilog|✔︎ OK|✔︎ OK|✔︎ OK|✘ Failed|
