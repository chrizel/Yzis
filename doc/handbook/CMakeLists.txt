

ADD_CUSTOM_COMMAND( 
	OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/YzisUserHandbook.pdf
	COMMAND   pdflatex
	ARGS      ${CMAKE_CURRENT_SOURCE_DIR}/YzisUserHandbook.tex -o ${CMAKE_CURRENT_BINARY_DIR}/YzisUserHandbook.pdf &&  pdflatex ${CMAKE_CURRENT_SOURCE_DIR}/YzisUserHandbook.tex -o ${CMAKE_CURRENT_BINARY_DIR}/YzisUserHandbook.pdf
	DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/YzisUserHandbook.tex
	COMMENT   "latex"
)

ADD_CUSTOM_COMMAND( 
	OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/YzisDeveloperHandbook.pdf
	COMMAND   pdflatex
	ARGS      ${CMAKE_CURRENT_SOURCE_DIR}/YzisDeveloperHandbook.tex -o ${CMAKE_CURRENT_BINARY_DIR}/YzisDeveloperHandbook.pdf &&  pdflatex ${CMAKE_CURRENT_SOURCE_DIR}/YzisDeveloperHandbook.tex -o ${CMAKE_CURRENT_BINARY_DIR}/YzisDeveloperHandbook.pdf
	DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/YzisDeveloperHandbook.tex
	COMMENT   "latex"
)

add_custom_target(handbook ALL echo
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/YzisUserHandbook.pdf ${CMAKE_CURRENT_BINARY_DIR}/YzisDeveloperHandbook.pdf
)

