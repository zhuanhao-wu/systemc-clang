"""Include tests that is from ex_* folders"""
import os
import warnings
from shutil import copy
import re
import pytest
from util.sexpdiff import sexpdiff
from util.vparser import VerilogParser


def detect_module(conf, ex_id):
    """detects module in the example folder"""
    __tracebackhide__ = True
    root_folder = conf.root_folder
    module_list = [
        fn for fn in os.listdir(root_folder)
        if fn.endswith('.cpp') or fn.endswith('.hpp')
    ]
    if not module_list:
        pytest.fail('no module found for ex_{}'.format(ex_id))
    elif len(module_list) > 1:
        pytest.fail('more than 1 c++ file found for ex_{}'.format(ex_id))
    return module_list[0]


def test_ex_sexp(tmpdir, exdriver, tool_output):
    """tests the conversion from cpp to sexp"""
    conf = exdriver.conf
    ex_id = conf.ex_id
    modname = detect_module(conf, ex_id)
    output_folder = tmpdir
    # first we need to copy them as cpp file so that xlat works
    filename = str(output_folder) + '/' + re.sub('.hpp$', '.cpp', modname)
    copy(conf.get_module_name(modname), filename)

    # idempotent because we only match the end of the string
    sexp_name = re.sub('.hpp$', '_hdl.txt', modname)
    sexp_name = re.sub('.cpp$', '_hdl.txt', sexp_name)

    if not os.path.isfile(conf.get_golden_sexp_name(sexp_name)):
        warnings.warn(conf.get_golden_sexp_name(sexp_name))
        warnings.warn('golden file for ex_{} not found'.format(ex_id))
        pytest.skip('no golden standard to match, skipped (update required)')

    res, filename = exdriver.generate_sexp(
        path=filename,
        output_folder=output_folder,
        verbose=tool_output,
        keep_sexp=True
    )
    assert res, 'should convert to sexpression, possibly something about the '\
                'systemc-clang binary'

    diff_res, diff_str = sexpdiff(
        filename,
        conf.get_golden_sexp_name(sexp_name)
    )
    if diff_res:
        print(diff_str)

    assert not diff_res, 'should match golden standard, or update is required'


def test_ex_verilog(tmpdir, exdriver, tool_output):
    conf = exdriver.conf
    ex_id = conf.ex_id
    modname = detect_module(conf, ex_id)
    output_folder = tmpdir
    filename = str(output_folder) + '/' + re.sub('.hpp$', '.cpp', modname)
    v_name = re.sub('.hpp$', '.cpp', modname)
    v_name = re.sub('.cpp$', '_hdl.txt.v', v_name)
    copy(conf.get_module_name(modname), filename)
    res, filename = exdriver.generate_verilog(
        path=filename,
        output_folder=output_folder,
        verbose=tool_output
    )
    assert res, "should convert to Verilog from cpp"
    diff_info = VerilogParser.diff(
        filename,
        conf.get_golden_verilog_name(v_name)
    )
    print(str(diff_info))
    assert diff_info is None, 'should be no diff in Verilog'


def test_ex_sexp_to_verilog(tmpdir, exdriver, tool_output):
    conf = exdriver.conf
    ex_id = conf.ex_id
    modname = detect_module(conf, ex_id)
    output_folder = tmpdir
    sexp_name = re.sub('.hpp$', '_hdl.txt', modname)
    sexp_name = re.sub('.cpp$', '_hdl.txt', sexp_name)
    filename = conf.get_golden_sexp_name(sexp_name)

    if not os.path.isfile(filename):
        warnings.warn('golden file for ex_{} not found'.format(ex_id))
        pytest.skip('no golden standard to match, skipped (update required)')

    copy(filename, str(output_folder) + '/')
    sexp_path = str(output_folder) + '/' + sexp_name
    v_name = sexp_name + '.v'

    res, filename = exdriver.generate_verilog_from_sexp(
        path=sexp_path,
        output_folder=output_folder,
        keep_v=True,
        verbose=tool_output
    )
    assert res, "should convert to Verilog from sexp"
    print('filename: ', filename)
    print('golden: ', conf.get_golden_verilog_name(v_name))
    diff_info = VerilogParser.diff(
        sexp_path + '.v',
        conf.get_golden_verilog_name(v_name)
    )
    print(str(diff_info))
    assert diff_info is None, 'should be no diff in Verilog'

