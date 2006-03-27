include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckPrototypeExists)
include(CheckTypeSize)

if(GETTEXT_FOUND)
	set(ENABLE_NLS 1)
else(GETTEXT_FOUND)
	set(ENABLE_NLS 0)
endif(GETTEXT_FOUND)


