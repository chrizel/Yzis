########### install files ###############
if (KDE4_FOUND)
	kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )
else (KDE4_FOUND)
	#not sure but could not find anything better yet
	install_files(/share/icons/crystalsvg/128x128/apps/ FILES qyzis.png )
	install_files(/share/icons/crystalsvg/64x64/apps/ FILES qyzis.png )
	install_files(/share/icons/crystalsvg/32x32/apps/ FILES qyzis.png )
	install_files(/share/icons/crystalsvg/16x16/apps/ FILES qyzis.png )
endif (KDE4_FOUND)

