--[[ =======================================================================

Script to convert a vim script in a lua script.

The script tries to convert the maximum number of constructions from vim to
lua. The things that can cause trouble:
- in lua, 0 is a truth value while in vim, 0 is a false value. Any script
  relying on 0 to be considered as failure will not be converted properly
- some keywords are unsupported and are marked as comments in the final
  script. See unsupportedKeywords table for exhaustive list.
- all types of variable are converted directly into lua variables,
  irrespective of whether they are buffer, window or whatever variables.

Apart from that, the script does a reasonable job. You sure need a
post-processing phase.

Author : Philippe Fremy
License: LGPL
Version: 0.1

======================================================================== ]]--

require('utils')

reString = [[(?:'.*'|".*")]]
reFunction = [[(?:\w+\(.*\))]]
reVariable = [[(\w+(?:\[(?:\w+|]]..reString..[[|-?\d+)\])?)]]
reAnyString = [[((?:\w+(?:\[(?:\w+|]]..reString..[[|-?\d+)\])?)]]..
				'|'..reString..
				'|'..reFunction..
				')'

-- substitutions to perform to convert a vim line into a lua line
substTable = {
	-- vim grammar keywords
	[Regexp([[function!]])] 	= [[function]],
	[Regexp([[\blet\s+]])] 		= [[]],
	[Regexp([[endfor]])] 		= [[end]],
	[Regexp([[endif]])] 		= [[end]],
	[Regexp([[endfunction]])] 	= [[end]],
	[Regexp([[endwhile]])] 		= [[end]],
	[Regexp([[call\s+]])] 		= [[]],
	[Regexp([[^(\s*\b(else)?if\b.*)$]])]	= [[\1 then]],
	[Regexp([[^(\s*\bfor\b.*)$]])]	= [[\1 do]],
	[Regexp([[^(\s*\bwhile\b.*)$]])]	= [[\1 do]],

	-- comments
	[Regexp([[^(\s*)"(.*)$]])] 	= [[\1--\2]],

	-- functions
	[Regexp([[echo]])] 			= [[print]],
	[Regexp([[^(\s+)insert\((.*),(.*),(.*)\)$]])]	= [[\1table.insert(\2,\3,\4)]],
	[Regexp([[^(\s+)insert\((.*),(.*)\)]])]	= [[\1table.insert(\2, 0,\3)]],
	[Regexp([[^(\s+)add\((.*),(.*)\)$]])]	= [[\1table.insert(\2,\3)]],

	-- variables and options
	[Regexp('g:'..reVariable)] 		= [[\1]],
	[Regexp('w:'..reVariable)] 		= [[\1]],
	[Regexp('s:'..reVariable)] 		= [[\1]],
	[Regexp('b:'..reVariable)] 		= [[\1]],
	[Regexp('a:'..reVariable)] 		= [[\1]],
	[Regexp('l:'..reVariable)] 		= [[\1]],

	-- options
	--[Regexp([[&(\w+)]])] 				= [[getOption('\1')]], -- too strong
	[Regexp([[set\s+no(\w+)]])] 			= [[setOption('\1', false )]],
	[Regexp([[set\s+(?!no)(\w+)]])] 			= [[setOption('\1', true )]],

	-- operators
	[Regexp([[&&]])] 			= [[and]],
	[Regexp([[\|\|]])] 			= [[or]],
	[Regexp([[!=]])] 			= [[~=]],
	[Regexp(reAnyString..[[\s*=~\s*]]..reAnyString)] 	= [[VimRegexp(\2):match(\1)]],
	[Regexp(reAnyString..[[\s*!~\s*]]..reAnyString)] 	= [[not VimRegexp(\2):match(\1)]], 
	[Regexp(reVariable..[[\s*\.=\s*(\S.*)]])]	= [[\1 = \1.\2]],

	-- i += 1
	[Regexp(reVariable..'\\s\\+=\\s*-?(\\d+|'..reVariable..')')] 	= [[\1 = \1 + \2]],

	-- array
	[Regexp(reVariable..'(\\s*=\\s*)\\[(\\s*)\\]')]	= [[\1\2{\3}]],
	[Regexp(reVariable..'(\\s\\+=\\s*)\\[(.*)\\]')]	= [[table_insert( \1, \3 )]],

	-- vim functions that clash with lua
	[Regexp([[\brepeat\b]])]	= [[_repeat]]

}

-- post substitutions to perform on a line already converted to lua
postSubstTable = {
	-- string contatenation
	[Regexp(reAnyString..'(?!table)\\.(?!\\insert)')] 	= [[\1..]],
	[Regexp('table_insert')] 	= [[table.insert]],
}

-- Vim to Lua unsupported keywords table
unsupportedKeywords = {
	['try'] 	= true,
	['endtry'] 	= true,
	['finally'] = true,
	['catch'] 	= true,
	['throw'] 	= true,
	['continue'] = true,
	['==#'] 	= true,
	['==\\?'] 	= true,
	['command'] = true,
	['v:'..reVariable] = true,
	['unlet'] = true,
	['silent'] = true,
	['syntax'] = true,
	['@\\w+'] = true,
	['%s/'] = true,
	['&\\w+'] = true,
	['\\bexe\\b'] = true,
}


-- Process one line of a vim script and return the lua equivalent
function process_line( line )
	--print("before:"..line)

	for re, subst in substTable do
		--print( "subst: '"..re:pattern().."' -> '"..subst.."'" )
		--print( "before subst: "..line )
		line = re:replace( line, subst )
		--print( "after  subst: "..line )
	end

	for before, after in postSubstTable do
		--print( "subst: '"..re:pattern().."' -> '"..subst.."'" )
		--print( "before subst: "..line )
		line = before:replace( line, after )
		--print( "after  subst: "..line )
	end

	--print("after subst : "..line)

	-- &option, &g:option, &l:option
	-- @register
	-- $VAR
	-- b:buffer, w:window, g:global, s:script -> global
	-- l:local -> global
	-- v:vim variable -> not supported
	-- =~, !~ : matching

	for unsupported in unsupportedKeywords do
		if Regexp(unsupported):match(line) then
			--print("XXX '"..unsupported.."' is not supported" )
			--print("line: '"..line.."'")
			line = Regexp('^(\\s*)(.*)$'):replace(line, '\\1-- XXX \\2' )
		end
	end

	--print( "after : '"..line.."'" )
	return line
end

-- Take the vim filename f and output an yzis lua script on the standard
-- output
function process_file(f)
	local i = 0
	out = {n=0}
	state = {}
	for line in io.lines(f) do
		table.insert( out, process_line( line ) )
		i = i + 1
		--if i == 200 then return end
	end
end

function main()
	FINPUT = 'vst.vim'
	FOUTPUT = 'vst.lua'
	io.output(FOUTPUT)
	io.write( "--	Autogenerated conversion from "..FINPUT..' to '..FOUTPUT..'\n\n' )

	out = {n=0}
	state = {}
	process_file( FINPUT )

	for i = 1,#out
	do
		io.write( out[i]..'\n' )
	end
	print("Done!")
end

if not _REQUIREDNAME then main() end
