
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

function printBufferContent()
    local i=1
    print("Buffer content:")
    while i<=linecount() do
        print("line "..i.." : '"..line(i).."'")
        i = i + 1
    end
end

function clearBuffer()
    while linecount() > 1 do
        deleteline(1)
    end
    deleteline(1)
end

function assertPos(line,col)
    assertEquals( winline(), line )
    assertEquals( wincol(), col )
end
function assertScrPos(line,col)
    assertEquals( scrline(), line )
    assertEquals( scrcol(), col )
end

MODE_NORMAL = "[ Awaiting Command ]"
MODE_INSERT = "[ Insert ]"
MODE_COMMAND = "[ Ex ]"
