
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

MODE_NORMAL='n'
MODE_INSERT='i'

