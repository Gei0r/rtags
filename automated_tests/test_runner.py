#!/usr/bin/env python
# coding=utf-8
#
# Assuming that the bins are in build/bin, run with
#
#     PATH=$(pwd)/build/bin:$PATH nosetests
#
# into the project folder.
#
import os
import sys
import platform
import json
import subprocess as sp
import re
from hamcrest import assert_that, has_length, has_item

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"
socket_file = "/var/tmp/rdm_dev"
clang = "clang++"
rdm_temp_file = "~/.rtags_dev"

if platform.system() == 'Windows':
    socket_file = os.path.expanduser("~") + "/rdm"
    clang = "clang++.exe"
    rdm_temp_file = os.path.expanduser("~") + "/.rtags_dev"


def create_compile_commands(test_dir, test_files):
    ret = [dict(directory=os.path.abspath(test_dir), file=test_file,
                command=(clang + " -std=c++11 -I. -c %s") % os.path.join(test_dir, test_file))
           for test_file in (src_file for src_file in test_files
                             if src_file.endswith('.cpp'))]
    print "Compile commands: [%s]" % ', '.join(map(str, ret))
    return ret


def read_locations(project_dir, lines):
    # print "proj_dir=" + project_dir + " lines=" + lines
    lines = [line.split(":") for line in lines.split("\n") if len(line) > 0]
    if platform.system() == 'Windows' and re.match('^[a-zA-Z]:', line):
        return [Location(line[0]+':'+line[1], line[2], line[3]) for line in lines]
    else:
        return [Location(os.path.join(project_dir, line[0]), line[1], line[2]) for line in lines]


class Location:
    def __init__(self, file, line, col):
        print "init: " + "file=" + file + " line=" + line + " col=" + col
        self.file, self.line, self.col = str(file), int(line), int(col)

        if platform.system() == 'Windows':
            self.file = self.file.replace("\\", "/")

    @classmethod
    def from_str(cls, s):
        if platform.system() == 'Windows' and re.match('^[a-zA-Z]:', s):
            elems = s.split(':')
            return cls(elems[0]+':'+elems[1], elems[2], elems[3])
        else:
            return cls(*s.split(":")[0:3])

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.__dict__ == other.__dict__
        else:
            raise ValueError("Type error")

    def __ne__(self, other):
        return not self.__eq__(other)

    def __repr__(self):
        return "%s:%d:%d" % (self.file, self.line, self.col)


def run_rc(args):
    args = ["rc", "--socket-file=" + socket_file] + args
    print "Running rc: [%s]" % ', '.join(map(str, args))
    return sp.check_output(args)


def wait_for(p, match):
    while p.poll() is None:
        l = p.stdout.readline()  # This blocks until it receives a newline.
        print l
        if match in l:
            break


def run(rdm, project_dir, test_dir, test_files, rc_command, expected_locations):
    print 'running test'
    actual_locations = \
        read_locations(project_dir,
                       run_rc([c.format(test_dir) for c in rc_command]))
    # Compare that we have the same results in length and content
    assert_that(actual_locations, has_length(len(expected_locations)))
    print 'checking location'
    for expected_location_string in expected_locations:
        expected_location = Location.from_str(expected_location_string.format(test_dir))
        assert_that(actual_locations, has_item(expected_location))

def setup_rdm(test_dir, test_files):
    rdm = sp.Popen(["rdm", "-n", socket_file, "-d", rdm_temp_file, "-o", "-B", "-C", "--log-flush" ],
                   stdout=sp.PIPE, stderr=sp.STDOUT)
    wait_for(rdm, "Includepaths")

    compile_commands = create_compile_commands(test_dir, test_files)
    for c in compile_commands:
        run_rc(["-c", c['command']])
        wait_for(rdm, "Jobs took")
    return rdm

def test_generator():
    base_test_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.abspath(os.path.join(base_test_dir, os.path.pardir))
    for test_dir, _, test_files in tuple(os.walk(base_test_dir))[1:]:
        print 'Test directory:',test_dir
        print 'Test files:',test_files
        if "ForwardDeclaration" in test_dir:
          continue
        expectations = json.load(open(os.path.join(test_dir, "expectation.json"), 'r'))
        rdm = setup_rdm(test_dir, test_files)
        for e in expectations:
            test_generator.__name__ = os.path.basename(test_dir)
            yield run, rdm, project_dir, test_dir, test_files, e["rc-command"], e["expectation"]
        rdm.terminate()
        rdm.wait()
