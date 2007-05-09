Aborting before end of test:
- test_lua_binding
- test_search



Yzis tests:
===========
- test_regexp: 
- test_utils
- test_search
- test_movements
- test_lua_binding
- test_bugs1
- test_vim_pattern.lua: Many vim patterns are fed into VimRegexp() for validating the conversion.
- test_all: all tests run at once
- run_test.sh


test_vim_patterns.vim

Utilities:
==========
- lua_unit
- test_lua_unit

Other scripts:
==============
- test_vim_patterm.vim
List of many vim regexp pattern. The file self-tests itself when run under
vim. The file is used to generate the test_vim_pattern.lua

- convert_vim_test_to_lua.lua : script that generates the test_vim_pattern.lua
  from the test_vim_pattern.vim

- test_vim_to_lua.lua: tests for the vim_to_lua script.
- vim_to_lua.lua: convert a vim script into a lua script
- vst.vim: example of a big vim script to convert to lua
- vst.lua: converted script from vst.vim
