--[[

Description: Test undo and redo

Author: Heikki Naski 
Version: 0.1
License: LGPL

]]--

require('luaunit')
require('utils')


TestUndo = {} --class

    function TestUndo:setUp() 
        clearBuffer()
    end

    function TestUndo:tearDown()
         -- TODO Is this enough tearing down? Are there any undo stacks etc. that we need to clear?
        clearBuffer()
    end

    function TestUndo:test_undo_i_a()
        clearBuffer()
        sendkeys("iFirst<ESC>")
        sendkeys("aUndoed<ESC>")
        sendkeys("u")
        assertEquals(bufferContent(), "First")
    end

    function TestUndo:test_undo_i_a_A()
        sendkeys("iFirst<ESC>")
        sendkeys("aSecond<ESC>")
        sendkeys("AUndoed<ESC>")
        sendkeys("u")
        assertEquals(bufferContent(), "FirstSecond")
    end

    function TestUndo:test_undo_dd()
        sendkeys("iFirst<ESC>")
        sendkeys("aSecond<ESC>")
        sendkeys("dd")
        sendkeys("u")
        assertEquals(bufferContent(), "FirstSecond")
    end

    function TestUndo:test_undo_x() 
        sendkeys("iFirst<ESC>")
        sendkeys("aSecond<ESC>")
        sendkeys("x")
        sendkeys("u")
        assertEquals(bufferContent(), "FirstSecond")
    end

    function TestUndo:test_undo_ex_s() 
        sendkeys("iFirst<ESC>")
        sendkeys("aSecond<ESC>")
        sendkeys(":s/Sec//<Cr>")
        sendkeys("u")
        assertEquals(bufferContent(), "FirstSecond")
    end

    function TestUndo:test_undo_shift_indent() 
        sendkeys("iFirst<ESC>")
        sendkeys(">>")
        sendkeys("u")
        assertEquals(bufferContent(), "First")
    end

    function TestUndo:test_undo_insertmode_delete_word() 
        sendkeys("iFirst<ESC>")
        sendkeys("a<C-W><Esc>")
        sendkeys("u")
        assertEquals(bufferContent(), "First")
    end

    function TestUndo:test_undo_insertmode_insert_character_above() 
        sendkeys("iFirst<ESC>")
        -- TODO The following line written with <C-e> doesn't work with lower case, should it?
        -- sendkeys("oFi<C-e><Esc>")
        sendkeys("oFi<C-E><Esc>")
        sendkeys("u")
        assertEquals(bufferContent(), "First")
    end

    function TestUndo:test_undo_yy_p() 
        sendkeys("iFirst<ESC>")
        sendkeys("yy")
        sendkeys("p")
        sendkeys("u")
        assertEquals(bufferContent(), "First")
    end

    --[[
    -- TODO This could be useful, but it seems like the paste takes a lot of time so it's probably needless to test it until pasting gets optimized.
    function TestUndo:test_undo_paste_10000() 
        sendkeys("iFirst<ESC>")
        sendkeys("yy")
        sendkeys("10000p")
        sendkeys("u")
        assertEquals(bufferContent(), "First")
    end
    ]]--

    function TestUndo:test_undo_insertmode_delete_word_yy_p() 
        -- TODO The following line written with <C-W> doesn't work with upper case, should it?
        --sendkeys("iFirst Deleted<C-W><ESC>")
        sendkeys("iFirst Deleted<C-w><ESC>")
        sendkeys("yyp")
        sendkeys("u")
        assertEquals(bufferContent(), "First ")
    end

    -- We need a proper way of resetting libyzis from lua before being able to do such a test
    --[[
    function TestUndo:test_undo_nothing() 
        clearBuffer()
        sendkeys("u")
        assertEquals(bufferContent(), "")
    end
    ]]--

    function TestUndo:test_undo_redo_nothing() 
        sendkeys("<C-r>")
        assertEquals(bufferContent(), "")
    end

    -- TODO This fails now, should we comment it out for the time being?
    -- orzel 2008/12 : indeed, this is not implemented yet
    --[[
    function TestUndo:test_undoall_i_a() 
        sendkeys("iFirst<ESC>")
        sendkeys("aSecond<ESC>")
        sendkeys("U")
        assertEquals(bufferContent(), "")
    end
    ]]--

    function TestUndo:test_undo_redo_insert() 
        sendkeys("iFirst<ESC>")
        sendkeys("aSecond<ESC>")
        sendkeys("u")
        sendkeys("<C-r>")
        assertEquals(bufferContent(), "FirstSecond")
    end

    function TestUndo:test_undo_redo_dd() 
        sendkeys("iFirst<ESC>")
        sendkeys("dd")
        sendkeys("u")
        sendkeys("<C-r>")
        assertEquals(bufferContent(), "")
    end

    function TestUndo:test_undo_redo_undo_dd() 
        sendkeys("iFirst<ESC>")
        sendkeys("dd")
        sendkeys("u")
        sendkeys("<C-r>")
        sendkeys("u")
        assertEquals(bufferContent(), "First")
    end

if not _REQUIREDNAME then
   ret = LuaUnit:run()
   setLuaReturnValue( ret )
end
