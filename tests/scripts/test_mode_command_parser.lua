--[[

Description: Attempt to test the parser in YModeCommand

Tests:

nocount,noregister NoargCmd
nocount,noregister NoargMotion
nocount,noregister ArgCharMotion
nocount,noregister ArgMotionCmd with/without count -- NoargMotion, ArgCharMotion

Same with count, with registers

Author: Tim Northover
Version: 0.1
License: LGPL

]]--

require('luaunit')
require('utils')

TestParser = {} -- class
    function TestParser:test_command_parser()
        -- Basic command
        clearBuffer()
        sendkeys("iWibble<ESC>0")
        assertEquals( bufferContent(), "Wibble")
        assertPos(1, 1)
        sendkeys("D")
        assertEquals( bufferContent(), "" )

        -- Basic motion
        sendkeys("iWib<ENTER>Wob<ESC>gg")
        assertPos(1,1)
        sendkeys("j")
        assertPos(2, 1)
        
        -- Motion with ArgChar, Command with ArgChar, then motion with Mark
        clearBuffer()
        sendkeys("iWibble a Wobble<ESC>0")
        assertPos(1,1)
        sendkeys("fa")
        assertPos(1,8)
        sendkeys("ma0")
        assertPos(1,1)
        sendkeys("`a")
        assertPos(1,8)

        -- Cmd with basic motion
        clearBuffer()
        sendkeys("i1 2 3 4 5 7 7 8 9<ESC>0")
        assertEquals( bufferContent(), "1 2 3 4 5 7 7 8 9")
        sendkeys("dw")
        assertEquals( bufferContent(), "2 3 4 5 7 7 8 9")
        
        -- Cmd with counted motion
        sendkeys("d2w")
        assertEquals( bufferContent(), "4 5 7 7 8 9")
        
        -- Cmd with basic argchar motion
        -- This test may fail if dt... is changed to conform with vim
        -- nothing to do with parser
        sendkeys("dt5")
        assertEquals( bufferContent(), " 5 7 7 8 9")
        
        -- Cmd with countet argchar motion
        -- Same issue
        sendkeys("d2t7")
        assertEquals( bufferContent(), " 7 8 9")

        -- Cmd with initial and mid count
        clearBuffer()
        sendkeys("i1 2 3 4 5 6 7 8 9<ESC>0")
        assertEquals( bufferContent(), "1 2 3 4 5 6 7 8 9")
        sendkeys("3d2w")
        assertEquals( bufferContent(), "7 8 9")
        
        -- Cmd with initial and mid count, the motion is argchar
        clearBuffer()
        sendkeys("i1 1 1 1 1 1 1 2 3<ESC>0")
        assertEquals( bufferContent(), "1 1 1 1 1 1 1 2 3")
        sendkeys("3d2t1")
        assertEquals( bufferContent(), " 1 2 3")
        
        -- Cmd with registers
        clearBuffer()
        sendkeys("i12<ESC>0")
        sendkeys("\"adl")
        sendkeys("\"bdl")
        assertEquals( bufferContent(), "")
        sendkeys("\"ap")
        assertEquals( bufferContent(), "1")
        sendkeys("\"bp")
        assertEquals( bufferContent(), "12")

        -- Counts and registers
        sendkeys("3\"ap")
        assertEquals( bufferContent(), "12111")
        sendkeys("\"b3p")
        assertEquals( bufferContent(), "12111222")
    end

if not _REQUIREDNAME then
   ret = LuaUnit:run()
   setLuaReturnValue( ret )
end
