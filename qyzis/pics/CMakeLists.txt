########### install files ###############
if (KDE4_FOUND)
	kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )
else (KDE4_FOUND)
	#not sure but could not find anything better yet
	install(FILES cr128-app-qyzis.png DESTINATION share/icons/crystalsvg/128x128/apps/ RENAME qyzis.png)
	install(FILES cr64-app-qyzis.png DESTINATION share/icons/crystalsvg/64x64/apps/ RENAME qyzis.png)
	install(FILES cr32-app-qyzis.png DESTINATION share/icons/crystalsvg/32x32/apps/ RENAME qyzis.png)
	install(FILES cr16-app-qyzis.png DESTINATION share/icons/crystalsvg/16x16/apps/ RENAME qyzis.png)
endif (KDE4_FOUND)

