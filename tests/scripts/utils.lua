--[[ =======================================================================

Utilities useful for Yzis lua scripting.

Author : Philippe Fremy
License: LGPL
Version: 0.1

======================================================================== ]]--

-- Returns a string containing all the lines of the current buffer
function bufferContent()
    local i,s
    i = 2
    s = line(1)
    while i<=linecount() do
        s = s.."\n".. line(i)
        i = i + 1
    end
    return s
end

-- Display the complete buffer content
function printBufferContent()
    local i=1
    print("Buffer content:")
    while i<=linecount() do
        print("line "..i.." : '"..line(i).."'")
        i = i + 1
    end
end

-- Clear all the lines of the current buffer
function clearBuffer()
    while linecount() > 1 do
        deleteline(1)
    end
    deleteline(1)
end

-- Assertion for line,col
function assertPos(line,col)
    if ((winline() == line) and (wincol() == col)) then return end
    errorMsg = "expected: (line,col)=("..tostring(line)..","..tostring(col)..") , actual: ("..tostring(winline())..","..tostring(wincol())..")\n"
    assertMsg( nil, errorMsg )
end


function assertScrPos(line,col)
    assertEquals( scrline(), line )
    assertEquals( scrcol(), col )
end

MODE_NORMAL = "[ Awaiting Command ]"
MODE_INSERT = "[ Insert ]"
MODE_COMMAND = "[ Ex ]"

