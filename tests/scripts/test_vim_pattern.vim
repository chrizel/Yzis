" Build some test patterns for Vim Regexp
" This file contains a lot of Regexp patterns from Vim as a self-test. We use
" it to validate that the regexp support of Vim work as expected.
" Then, we use all the patterns to test the VimRegexp function.

set magic
set noignorecase

let g:count = 0

function! AssertEquals(expr, expected)
	let g:count = g:count + 1
	if a:expr == a:expected
		return
	endif
	echo "Error at " . g:count . " !!!"
endfunction

echo '******'
let g:count = 0
call AssertEquals( 'aaab' =~ 'a*b', 1 )
call AssertEquals( 'ab' =~ 'a*b', 1 )
call AssertEquals( 'b' =~ 'a*b', 1 )
call AssertEquals( 'a' =~ 'a*b', 0 )

echo '++++++'
let g:count = 0
call AssertEquals( 'aaab' =~ 'a\+b', 1 )
call AssertEquals( 'ab' =~ 'a\+b', 1 )
call AssertEquals( 'b' =~ 'a\+b', 0 )
call AssertEquals( 'a' =~ 'a\+b', 0 )

echo '======'
let g:count = 0
call AssertEquals( 'aaab' =~ 'a\=b', 1 )
call AssertEquals( 'ab' =~ 'a\=b', 1 )
call AssertEquals( 'b' =~ 'a\=b', 1 )
call AssertEquals( 'a' =~ 'a\=b', 0 )
call AssertEquals( 'abbb' =~ 'ab\=', 1 )

echo '??????'
let g:count = 0
call AssertEquals( 'aaab' =~ 'a\?b', 1 )
call AssertEquals( 'ab' =~ 'a\?b', 1 )
call AssertEquals( 'b' =~ 'a\?b', 1 )
call AssertEquals( 'a' =~ 'a\?b', 0 )
call AssertEquals( 'abbb' =~ 'ab\?', 1 )

echo '{{{{{{}}}}}}'
let g:count = 0
call AssertEquals( 'ab' =~ 'a\{1}b', 1 )
call AssertEquals( 'b' =~ 'a\{1}b', 0 )

call AssertEquals( 'ab' =~ 'a\{0,1}b', 1 )
call AssertEquals( 'b' =~ 'a\{0,1}b', 1 )

call AssertEquals( 'ab' =~ 'a\{,2}b', 1 )
call AssertEquals( 'b' =~ 'a\{,2}b', 1 )
call AssertEquals( 'aab' =~ 'a\{,2}b', 1 )

call AssertEquals( 'ab' =~ 'a\{}b', 1 )
call AssertEquals( 'b' =~ 'a\{}b', 1 )
call AssertEquals( 'aab' =~ 'a\{}b', 1 )

echo '^^^'
let g:count = 0
call AssertEquals( 'aa^bb' =~ '^aa', 1 )
call AssertEquals( 'aa^bb' =~ 'aa^', 1 )
call AssertEquals( 'aa^bb' =~ '^bb', 0 )
call AssertEquals( 'aa^bb' =~ '^aa^bb', 1 )
call AssertEquals( 'aa^bb' =~ '^aa\^bb', 1 )
call AssertEquals( 'aa^bb' =~ '\^aa\^bb', 0 )
call AssertEquals( '^aa^bb' =~ '\^aa\^bb', 1 )


echo '[ [ [ ] ] ]'
call AssertEquals( 'a' =~ '[a]', 1 )
call AssertEquals( 'a' =~ '[^a]', 0 )
call AssertEquals( 'a' =~ '[a^]', 1 )
call AssertEquals( '^' =~ '[a^]', 1 )

let g:count = 10
call AssertEquals( 'aa^bb' =~ 'x\|y\|^aa', 1 )
call AssertEquals( 'aa^bb' =~ '\(^aa\)', 1 )
call AssertEquals( 'aa^bb' =~ 'x\|y\|^bb', 0 )
call AssertEquals( 'aa^bb' =~ '\(^bb\)', 0 )

echo '$$$$$'
let g:count = 0
call AssertEquals( 'aa$bb' =~ 'bb$', 1 )
call AssertEquals( 'aa$bb' =~ '$bb', 1 )
call AssertEquals( 'aa$bb' =~ 'aa$', 0 )
call AssertEquals( 'aa$bb' =~ 'aa$bb$', 1 )

let g:count = 10
call AssertEquals( 'aa$bb' =~ 'bb$\|x\|y', 1 )
call AssertEquals( 'aa$bb' =~ 'aa$\|x\|y', 0 )
call AssertEquals( 'aa$bb' =~ '\(bb$\)', 1 )
call AssertEquals( 'aa$bb' =~ '\(aa$\)', 0 )

echo '<<<<'
let g:count = 0
call AssertEquals( 'aa bbccdd' =~ '\<aa', 1 )
call AssertEquals( 'aa bbccdd' =~ '\<bb', 1 )
call AssertEquals( 'aa bbccdd' =~ '\<cc', 0 )
call AssertEquals( 'aa bbccdd' =~ '\<dd', 0 )

echo '>>>'
let g:count = 0
call AssertEquals( 'aa bbccdd' =~ 'aa\>', 1 )
call AssertEquals( 'aa bbccdd' =~ 'bb\>', 0 )
call AssertEquals( 'aa bbccdd' =~ 'cc\>', 0 )
call AssertEquals( 'aa bbccdd' =~ 'dd\>', 1 )

echo '|||||'
let g:count = 0
call AssertEquals( 'a' =~ 'a|b\|c', 0 )
call AssertEquals( 'b' =~ 'a|b\|c', 0 )
call AssertEquals( 'c' =~ 'a|b\|c', 1 )
call AssertEquals( 'a|b' =~ 'a|b\|c', 1 )
call AssertEquals( 'b|c' =~ 'a|b\|c', 1 )
call AssertEquals( 'a|b|c' =~ 'a|b\|c', 1 )

echo '(((())))'
let g:count = 0
call AssertEquals( 'a(b)(c)' =~ 'a(b)\(c\)', 0 )
call AssertEquals( 'ab(c)' =~ 'a(b)\(c\)', 0 )
call AssertEquals( 'a(b)c' =~ 'a(b)\(c\)', 1 )
call AssertEquals( 'abc' =~ 'a(b)\(c\)', 0 )

echo '\1\2\3'
let g:count = 0
call AssertEquals( 'a(b)c_c' =~ 'a(b)\(c\)_\1', 1 )
call AssertEquals( 'a(b)c_b' =~ 'a(b)\(c\)_\1', 0 )

echo '\d\D'
let g:count = 0
call AssertEquals( '1' =~ '\d', 1 )
call AssertEquals( 'a' =~ '\d', 0 )
call AssertEquals( '_' =~ '\d', 0 )
call AssertEquals( ' ' =~ '\d', 0 )
call AssertEquals( '\n' =~ '\d', 0 )
call AssertEquals( '\t' =~ '\d', 0 )

let g:count = 10
call AssertEquals( '1' =~  '\D', 0 )
call AssertEquals( 'a' =~  '\D', 1 )
call AssertEquals( '_' =~  '\D', 1 )
call AssertEquals( ' ' =~  '\D', 1 )
call AssertEquals( '\n' =~ '\D', 1 )
call AssertEquals( '\t' =~ '\D', 1 )

echo '\s\S'
let g:count = 0
call AssertEquals( '1'  =~ '\s', 0 )
call AssertEquals( 'a'  =~ '\s', 0 )
call AssertEquals( '_'  =~ '\s', 0 )
call AssertEquals( ' '  =~ '\s', 1 )
"call AssertEquals( '\t' =~ '\s', 1 )	" bug!
"call AssertEquals( '\n' =~ '\s', 1 )	" bug!

let g:count = 10
call AssertEquals( '1'  =~ '\S', 1 )
call AssertEquals( 'a'  =~ '\S', 1 )
call AssertEquals( '_'  =~ '\S', 1 )
call AssertEquals( ' '  =~ '\S', 0 )
"call AssertEquals( '\t' =~ '\S', 0 )	" bug!
"call AssertEquals( '\n' =~ '\S', 0 )	" bug!

echo '\x\X'
let g:count = 0
call AssertEquals( '1'  =~ '\x', 1 )
call AssertEquals( 'a'  =~ '\x', 1 )
call AssertEquals( 'A'  =~ '\x', 1 )
call AssertEquals( 'g'  =~ '\x', 0 )
call AssertEquals( '_'  =~ '\x', 0 )
call AssertEquals( ' '  =~ '\x', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '\X', 0 )
call AssertEquals( 'a'  =~ '\X', 0 )
call AssertEquals( 'A'  =~ '\X', 0 )
call AssertEquals( 'g'  =~ '\X', 1 )
call AssertEquals( '_'  =~ '\X', 1 )
call AssertEquals( ' '  =~ '\X', 1 )

echo '\o\O'
let g:count = 0
call AssertEquals( '1'  =~ '\o', 1 )
call AssertEquals( 'a'  =~ '\o', 0 )
call AssertEquals( 'A'  =~ '\o', 0 )
call AssertEquals( '8'  =~ '\o', 0 )
call AssertEquals( '_'  =~ '\o', 0 )
call AssertEquals( ' '  =~ '\o', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '\O', 0 )
call AssertEquals( 'a'  =~ '\O', 1 )
call AssertEquals( 'A'  =~ '\O', 1 )
call AssertEquals( '8'  =~ '\O', 1 )
call AssertEquals( '_'  =~ '\O', 1 )
call AssertEquals( ' '  =~ '\O', 1 )

echo '\w\W'
let g:count = 0
call AssertEquals( '1'  =~ '\w', 1 )
call AssertEquals( 'a'  =~ '\w', 1 )
call AssertEquals( 'A'  =~ '\w', 1 )
call AssertEquals( '_'  =~ '\w', 1 )
call AssertEquals( ' '  =~ '\w', 0 )
call AssertEquals( ','  =~ '\w', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '\W', 0 )
call AssertEquals( 'a'  =~ '\W', 0 )
call AssertEquals( 'A'  =~ '\W', 0 )
call AssertEquals( '_'  =~ '\W', 0 )
call AssertEquals( ' '  =~ '\W', 1 )
call AssertEquals( ','  =~ '\W', 1 )

echo '\h\H'
let g:count = 0
call AssertEquals( '1'  =~ '\h', 0 )
call AssertEquals( 'a'  =~ '\h', 1 )
call AssertEquals( 'A'  =~ '\h', 1 )
call AssertEquals( '_'  =~ '\h', 1 )
call AssertEquals( ' '  =~ '\h', 0 )
call AssertEquals( ','  =~ '\h', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '\H', 1 )
call AssertEquals( 'a'  =~ '\H', 0 )
call AssertEquals( 'A'  =~ '\H', 0 )
call AssertEquals( '_'  =~ '\H', 0 )
call AssertEquals( ' '  =~ '\H', 1 )
call AssertEquals( ','  =~ '\H', 1 )

echo '\a\A'
let g:count = 0
call AssertEquals( '1'  =~ '\a', 0 )
call AssertEquals( 'a'  =~ '\a', 1 )
call AssertEquals( 'A'  =~ '\a', 1 )
call AssertEquals( '_'  =~ '\a', 0 )
call AssertEquals( ' '  =~ '\a', 0 )
call AssertEquals( ','  =~ '\a', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '\A', 1 )
call AssertEquals( 'a'  =~ '\A', 0 )
call AssertEquals( 'A'  =~ '\A', 0 )
call AssertEquals( '_'  =~ '\A', 1 )
call AssertEquals( ' '  =~ '\A', 1 )
call AssertEquals( ','  =~ '\A', 1 )

echo '\l\L'
let g:count = 0
call AssertEquals( '1'  =~ '\l', 0 )
call AssertEquals( 'a'  =~ '\l', 1 )
call AssertEquals( 'A'  =~ '\l', 0 )
call AssertEquals( '_'  =~ '\l', 0 )
call AssertEquals( ' '  =~ '\l', 0 )
call AssertEquals( ','  =~ '\l', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '\L', 1 )
call AssertEquals( 'a'  =~ '\L', 0 )
call AssertEquals( 'A'  =~ '\L', 1 )
call AssertEquals( '_'  =~ '\L', 1 )
call AssertEquals( ' '  =~ '\L', 1 )
call AssertEquals( ','  =~ '\L', 1 )

echo '\u\U'
let g:count = 0
call AssertEquals( '1'  =~ '\u', 0 )
call AssertEquals( 'a'  =~ '\u', 0 )
call AssertEquals( 'A'  =~ '\u', 1 )
call AssertEquals( '_'  =~ '\u', 0 )
call AssertEquals( ' '  =~ '\u', 0 )
call AssertEquals( ','  =~ '\u', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '\U', 1 )
call AssertEquals( 'a'  =~ '\U', 1 )
call AssertEquals( 'A'  =~ '\U', 0 )
call AssertEquals( '_'  =~ '\U', 1 )
call AssertEquals( ' '  =~ '\U', 1 )
call AssertEquals( ','  =~ '\U', 1 )

echo '[:alnum:]]'
let g:count = 0
call AssertEquals( '1'  =~ '[[:alnum:]]', 1 )
call AssertEquals( 'a'  =~ '[[:alnum:]]', 1 )
call AssertEquals( 'A'  =~ '[[:alnum:]]', 1 )
call AssertEquals( '_'  =~ '[[:alnum:]]', 0 )
call AssertEquals( ' '  =~ '[[:alnum:]]', 0 )
call AssertEquals( ','  =~ '[[:alnum:]]', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '[^[:alnum:]]', 0 )
call AssertEquals( 'a'  =~ '[^[:alnum:]]', 0 )
call AssertEquals( 'A'  =~ '[^[:alnum:]]', 0 )
call AssertEquals( '_'  =~ '[^[:alnum:]]', 1 )
call AssertEquals( ' '  =~ '[^[:alnum:]]', 1 )
call AssertEquals( ','  =~ '[^[:alnum:]]', 1 )

echo '[:alpha:]]'
let g:count = 0
call AssertEquals( '1'  =~ '[[:alpha:]]', 0 )
call AssertEquals( 'a'  =~ '[[:alpha:]]', 1 )
call AssertEquals( 'A'  =~ '[[:alpha:]]', 1 )
call AssertEquals( '_'  =~ '[[:alpha:]]', 0 )
call AssertEquals( ' '  =~ '[[:alpha:]]', 0 )
call AssertEquals( ','  =~ '[[:alpha:]]', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '[^[:alpha:]]', 1 )
call AssertEquals( 'a'  =~ '[^[:alpha:]]', 0 )
call AssertEquals( 'A'  =~ '[^[:alpha:]]', 0 )
call AssertEquals( '_'  =~ '[^[:alpha:]]', 1 )
call AssertEquals( ' '  =~ '[^[:alpha:]]', 1 )
call AssertEquals( ','  =~ '[^[:alpha:]]', 1 )

echo '[:blank:]]'
let g:count = 0
call AssertEquals( '1'  =~ '[[:blank:]]', 0 )
call AssertEquals( 'a'  =~ '[[:blank:]]', 0 )
call AssertEquals( 'A'  =~ '[[:blank:]]', 0 )
call AssertEquals( '_'  =~ '[[:blank:]]', 0 )
call AssertEquals( ' '  =~ '[[:blank:]]', 1 )
call AssertEquals( ','  =~ '[[:blank:]]', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '[^[:blank:]]', 1 )
call AssertEquals( 'a'  =~ '[^[:blank:]]', 1 )
call AssertEquals( 'A'  =~ '[^[:blank:]]', 1 )
call AssertEquals( '_'  =~ '[^[:blank:]]', 1 )
call AssertEquals( ' '  =~ '[^[:blank:]]', 0 )
call AssertEquals( ','  =~ '[^[:blank:]]', 1 )

echo '[:digit:]]'
let g:count = 0
call AssertEquals( '1'  =~ '[[:digit:]]', 1 )
call AssertEquals( 'a'  =~ '[[:digit:]]', 0 )
call AssertEquals( 'A'  =~ '[[:digit:]]', 0 )
call AssertEquals( '_'  =~ '[[:digit:]]', 0 )
call AssertEquals( ' '  =~ '[[:digit:]]', 0 )
call AssertEquals( ','  =~ '[[:digit:]]', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '[^[:digit:]]', 0 )
call AssertEquals( 'a'  =~ '[^[:digit:]]', 1 )
call AssertEquals( 'A'  =~ '[^[:digit:]]', 1 )
call AssertEquals( '_'  =~ '[^[:digit:]]', 1 )
call AssertEquals( ' '  =~ '[^[:digit:]]', 1 )
call AssertEquals( ','  =~ '[^[:digit:]]', 1 )

echo '[:lower:]]'
let g:count = 0
call AssertEquals( '1'  =~ '[[:lower:]]', 0 )
call AssertEquals( 'a'  =~ '[[:lower:]]', 1 )
call AssertEquals( 'A'  =~ '[[:lower:]]', 0 )
call AssertEquals( '_'  =~ '[[:lower:]]', 0 )
call AssertEquals( ' '  =~ '[[:lower:]]', 0 )
call AssertEquals( ','  =~ '[[:lower:]]', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '[^[:lower:]]', 1 )
call AssertEquals( 'a'  =~ '[^[:lower:]]', 0 )
call AssertEquals( 'A'  =~ '[^[:lower:]]', 1 )
call AssertEquals( '_'  =~ '[^[:lower:]]', 1 )
call AssertEquals( ' '  =~ '[^[:lower:]]', 1 )
call AssertEquals( ','  =~ '[^[:lower:]]', 1 )

echo '[[:space:]]'
let g:count = 0
call AssertEquals( '1'  =~ '[[:space:]]', 0 )
call AssertEquals( 'a'  =~ '[[:space:]]', 0 )
call AssertEquals( 'A'  =~ '[[:space:]]', 0 )
call AssertEquals( '_'  =~ '[[:space:]]', 0 )
call AssertEquals( ' '  =~ '[[:space:]]', 1 )
call AssertEquals( ','  =~ '[[:space:]]', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '[^[:space:]]', 1 )
call AssertEquals( 'a'  =~ '[^[:space:]]', 1 )
call AssertEquals( 'A'  =~ '[^[:space:]]', 1 )
call AssertEquals( '_'  =~ '[^[:space:]]', 1 )
call AssertEquals( ' '  =~ '[^[:space:]]', 0 )
call AssertEquals( ','  =~ '[^[:space:]]', 1 )

echo '[[:upper:]]'
let g:count = 0
call AssertEquals( '1'  =~ '[[:upper:]]', 0 )
call AssertEquals( 'a'  =~ '[[:upper:]]', 0 )
call AssertEquals( 'A'  =~ '[[:upper:]]', 1 )
call AssertEquals( '_'  =~ '[[:upper:]]', 0 )
call AssertEquals( ' '  =~ '[[:upper:]]', 0 )
call AssertEquals( ','  =~ '[[:upper:]]', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '[^[:upper:]]', 1 )
call AssertEquals( 'a'  =~ '[^[:upper:]]', 1 )
call AssertEquals( 'A'  =~ '[^[:upper:]]', 0 )
call AssertEquals( '_'  =~ '[^[:upper:]]', 1 )
call AssertEquals( ' '  =~ '[^[:upper:]]', 1 )
call AssertEquals( ','  =~ '[^[:upper:]]', 1 )

echo '[[:xdigit:]]'
let g:count = 0
call AssertEquals( '1'  =~ '[[:xdigit:]]', 1 )
call AssertEquals( 'a'  =~ '[[:xdigit:]]', 1 )
call AssertEquals( 'A'  =~ '[[:xdigit:]]', 1 )
call AssertEquals( 'g'  =~ '[[:xdigit:]]', 0 )
call AssertEquals( '_'  =~ '[[:xdigit:]]', 0 )
call AssertEquals( ' '  =~ '[[:xdigit:]]', 0 )
call AssertEquals( ','  =~ '[[:xdigit:]]', 0 )

let g:count = 10
call AssertEquals( '1'  =~ '[^[:xdigit:]]', 0 )
call AssertEquals( 'a'  =~ '[^[:xdigit:]]', 0 )
call AssertEquals( 'A'  =~ '[^[:xdigit:]]', 0 )
call AssertEquals( 'g'  =~ '[^[:xdigit:]]', 1 )
call AssertEquals( '_'  =~ '[^[:xdigit:]]', 1 )
call AssertEquals( ' '  =~ '[^[:xdigit:]]', 1 )
call AssertEquals( ','  =~ '[^[:xdigit:]]', 1 )

echo '\%( \)'
let g:count = 0
call AssertEquals( 'abcd_a_d' =~ '\(a\)b\%(c\)\(d\)_\1_\2', 1 )
call AssertEquals( 'abcd_a_c' =~ '\(a\)b\%(c\)\(d\)_\1_\2', 0 )

echo '\_^'
let g:count = 0
call AssertEquals( 'aa' =~ '\_^aa', 1 )
call AssertEquals( '^aa' =~ '\_^aa', 0 )
call AssertEquals( 'aa^bb' =~ 'aa\_^bb', 0 )
call AssertEquals( 'bb' =~ 'aa\_^bb', 0 )
call AssertEquals( 'aa^bb' =~ 'aa\^bb', 1 )
call AssertEquals( 'aa^bb' =~ 'aa^bb', 1 )

echo '\_$'
let g:count = 0
call AssertEquals( 'aa' =~ 'aa\_$', 1 )
call AssertEquals( 'aa$' =~ 'aa\_$', 0 )
call AssertEquals( 'aa$bb' =~ 'aa\_$bb', 0 )
call AssertEquals( 'bb' =~ 'aa\_$bb', 0 )
call AssertEquals( 'aa$bb' =~ 'aa\$bb', 1 )
call AssertEquals( 'aa$bb' =~ 'aa$bb', 1 )

echo 'unsupported'
" unsupported [:return:]
" unsupported [:tab:]
" unsupported [:graph:]
" unsupported [:print:]
" unsupported [:punct:]
" unsupported \&
" unsupported {-
" unsupported \@>	
" unsupported \@=
" unsupported \@!
" unsupported \@<=
" unsupported \@<!
" unsupported \zs
" unsupported \ze
" unsupported \_.
" unsupported \%^
" unsupported \%$
" unsupported \%#
" unsupported \%23l
" unsupported \%23c
" unsupported \%23v
" unsupported \i
" unsupported \I
" unsupported \k
" unsupported \K
" unsupported \f
" unsupported \F
" unsupported \p
" unsupported \P
" unsupported \t
" unsupported \e
" unsupported \r
" unsupported \b
" unsupported \n
" unsupported \~
" unsupported \z1
" unsupported \z9
" unsupported \%[
" unsupported \c
" unsupported \C
" unsupported \Z
" unsupported \v
" unsupported \V
" unsupported \m
" unsupported \M

