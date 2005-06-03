
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

function inTable( t, ... )
    -- check whether the values passed in argument are keys of table t
    --print("InTable for table")
    --table.foreach(t, print)
    for i = 1,arg['n']
    do
        v = arg[i] 
        --print("\tChecking index "..tostring(i)..": '"..tostring(v).."'")
        --print ("t[v]="..tostring(t[v]))
        if t[v] then 
            --print("\tMatch for '"..v.."'")
            return v 
        end
    end
    --print "\tNo match"
    return nil
end

VimSubstTable = {}
VimSubstTable['\\+'] = '+'
VimSubstTable['\\='] = '?'
VimSubstTable['\\?'] = '?'
VimSubstTable['\\{'] = '{'
VimSubstTable['\\|'] = '|'
VimSubstTable['|'  ] = '\\|'
VimSubstTable['\\('] = '('
VimSubstTable['('  ] = '\\('
VimSubstTable[')'  ] = '\\)'
VimSubstTable['\\)'] = ')'
VimSubstTable['\\<'] = '\\b'
VimSubstTable['\\>'] = '\\b'
VimSubstTable['\\x'] = '[0-9a-fA-F]'
VimSubstTable['\\X'] = '[^0-9a-fA-F]'
VimSubstTable['\\o'] = '[0-7]'
VimSubstTable['\\O'] = '[^0-7]'
VimSubstTable['\\h'] = '[a-zA-Z_]'
VimSubstTable['\\H'] = '[^a-zA-Z_]'
VimSubstTable['\\a'] = '[a-zA-Z]'
VimSubstTable['\\A'] = '[^a-zA-Z]'
VimSubstTable['\\l'] = '[a-z]'
VimSubstTable['\\L'] = '[^a-z]'
VimSubstTable['\\u'] = '[A-Z]'
VimSubstTable['\\U'] = '[^A-Z]'
VimSubstTable['[:alnum:]'] = 'a-zA-Z0-9'
VimSubstTable['[:alpha:]'] = 'a-zA-Z'
VimSubstTable['[:blank:]'] = '\\s'
VimSubstTable['[:digit:]'] = '0-9'
VimSubstTable['[:xdigit:]'] = '0-9A-Fa-f'
VimSubstTable['[:lower:]'] = 'a-z'
VimSubstTable['[:upper:]'] = 'A-Z'
VimSubstTable['[:space:]'] = ' '
VimSubstTable['\\%('] = '(?:'
VimSubstTable['\\_^'] = '^'
VimSubstTable['\\^'] = '\\^'
VimSubstTable['\\_$'] = '$'
VimSubstTable['\\$'] = '\\$'

unsupportedTokens = {}
unsupportedTokens[ '[:return:]' ] = true
unsupportedTokens[ '[:tab:]'    ] = true
unsupportedTokens[ '[:graph:]' ] = true
unsupportedTokens[ '[:print:]' ] = true
unsupportedTokens[ '[:punct:]' ] = true
unsupportedTokens[ [[\&]] ] = true
unsupportedTokens[ [[{-]] ] = true
unsupportedTokens[ [[\@>]] ] = true
unsupportedTokens[ [[\@=]] ] = true
unsupportedTokens[ [[\@!]] ] = true
unsupportedTokens[ [[\@<=]] ] = true
unsupportedTokens[ [[\@<!]] ] = true
unsupportedTokens[ [[\zs]] ] = true
unsupportedTokens[ [[\ze]] ] = true
unsupportedTokens[ [[\_.]] ] = true
unsupportedTokens[ [[\%^]] ] = true
unsupportedTokens[ [[\%$]] ] = true
unsupportedTokens[ [[\%#]] ] = true
unsupportedTokens[ [[\%23l]] ] = true
unsupportedTokens[ [[\%23c]] ] = true
unsupportedTokens[ [[\%23v]] ] = true
unsupportedTokens[ [[\i]] ] = true
unsupportedTokens[ [[\I]] ] = true
unsupportedTokens[ [[\k]] ] = true
unsupportedTokens[ [[\K]] ] = true
unsupportedTokens[ [[\f]] ] = true
unsupportedTokens[ [[\F]] ] = true
unsupportedTokens[ [[\p]] ] = true
unsupportedTokens[ [[\P]] ] = true
unsupportedTokens[ [[\t]] ] = true
unsupportedTokens[ [[\e]] ] = true
unsupportedTokens[ [[\r]] ] = true
unsupportedTokens[ [[\b]] ] = true
unsupportedTokens[ [[\n]] ] = true
unsupportedTokens[ [[\~]] ] = true
unsupportedTokens[ [[\z1]] ] = true
unsupportedTokens[ [[\z9]] ] = true
unsupportedTokens[ [[\%[]] ] = true
unsupportedTokens[ [[\c]] ] = true
unsupportedTokens[ [[\C]] ] = true
unsupportedTokens[ [[\Z]] ] = true
unsupportedTokens[ [[\v]] ] = true
unsupportedTokens[ [[\V]] ] = true
unsupportedTokens[ [[\m]] ] = true
unsupportedTokens[ [[\M]] ] = true

tokens = {}
tokens['\\^'] = true 
tokens['[^'] = true 

table.foreach( VimSubstTable, function(idx, val)
        tokens[idx] = val
    end )

table.foreach( unsupportedTokens, function(idx, val)
        tokens[idx] = val
    end )

function tokenizer( tokens, s )
    -- analyse s and returns a list of tokens or character
    ret = { n=0 }
    idx = 1
    while idx <= string.len(s)
    do
        c1 = string.sub(s,idx,idx)
        c2 = string.sub(s,idx,idx+1)
        c3 = string.sub(s,idx,idx+2)
        c4 = string.sub(s,idx,idx+3)
        c5 = string.sub(s,idx,idx+4)
        c6 = string.sub(s,idx,idx+5)
        c7 = string.sub(s,idx,idx+6)
        c8 = string.sub(s,idx,idx+7)
        c9 = string.sub(s,idx,idx+8)
        c10 = string.sub(s,idx,idx+9)
        c11 = string.sub(s,idx,idx+10)
        token = inTable( tokens, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11 ) or c1
        --print("token found: "..token)
        table.insert(ret, token)
        idx_inc = string.len(token)
        idx = idx + idx_inc
    end
    return ret
end


--TestTokenizer()

function VimRegexp( re )
    -- transform re
    --print("Before: '"..re.."'")
    out = ''
    tokenizedRe = tokenizer( tokens, re )
    --print("Tokenized re")
    --table.foreach( tokenizedRe, print )
    for i = 1, tokenizedRe['n']
    do
        token = tokenizedRe[i]
        if VimSubstTable[token]
        then
            out = out .. VimSubstTable[token]        

        -- handle ^
        elseif (token == '^' and i ~= 1  
            and tokenizedRe[i-1] ~= '\\|'
            and tokenizedRe[i-1] ~= '\\(' )
        then
            out = out .. '\\^'

        -- handle $
        elseif (token == '$' and tokenizedRe[i+1] ~= nil  
            and tokenizedRe[i+1] ~= '\\|'
            and tokenizedRe[i+1] ~= '\\)' )
        then
            out = out .. '\\$'

        elseif unsupportedTokens[token]
        then
            error("Token "..token.." is not supported")
        else
            out = out .. token
        end
        --print("Partial Result: '"..out.."'")
    end
    --print("Result: '"..out.."'")
    return Regexp(out)
end


-- Split text into a list consisting of the strings in text,
-- separated by strings matching delimiter (which may be a pattern). 
-- example: strsplit(",%s*", "Anna, Bob, Charlie,Dolores")
function strsplit(delimiter, text)
    local list = {}
    local pos = 1
    if string.find("", delimiter, 1) then -- this would result in endless loops
        error("delimiter matches empty string!")
    end
    while 1 do
        local first, last = string.find(text, delimiter, pos)
        if first then -- found?
            table.insert(list, string.sub(text, pos, first-1))
            pos = last+1
        else
            table.insert(list, string.sub(text, pos))
            break
        end
    end
    return list
end
