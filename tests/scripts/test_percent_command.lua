require('luaunit')
require('utils')

Test_percentCommand = {} --class
    function Test_percentCommand:setUp() 
        clearBuffer()
    end

    function Test_percentCommand:tearDown() 
        clearBuffer()
    end

 
    function Test_percentCommand:test_global()
        --[[ Test the % command : test imbricated chars, multiline... ]]--
        assertEquals( bufferContent(), "" )
        sendkeys("iint main(int argc, char *argv[]) {<ESC>")
        sendkeys("o    //(test(a[100]))<ESC>")
        sendkeys("o}<ESC>")
        sendkeys("gg^%")
        assertPos(1, 32)
        sendkeys("%")
        assertPos(1, 9)
        sendkeys("t{%")
        assertPos(3,1)
        sendkeys("%")
        assertPos(1, 34)
        sendkeys("j^")
        sendkeys("%")
        assertPos(2, 20)
        sendkeys("%")
        assertPos(2, 7)
        sendkeys("l%")
        assertPos(2, 19)
        sendkeys("%")
        assertPos(2, 12)
        sendkeys("l%")
        assertPos(2, 18)
    end

    function Test_percentCommand:test_unbalanced()
        sendkeys("itest ( with some text after<ESC>")
        sendkeys("^")
        sendkeys("%")
        assertPos(1,1)

        clearBuffer()
        sendkeys("otest ) with some text<ESC>")
        goto(2,2)
        sendkeys("%")
        assertPos(2,2)

        clearBuffer()
        insertline(1, "{ ()) }")
        goto(1,1)
        sendkeys("%")
        assertPos(1, 7)
        sendkeys("%")
        assertPos(1,1)
    end


if not _REQUIREDNAME then
    -- ret = LuaUnit:run('Test_percentCommand:test_char_movement') -- will execute only one test
    ret = LuaUnit:run() -- will execute all tests
    setLuaReturnValue( ret )
end

