########### install files ###############
if (KDE4_FOUND)
	kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )
else (KDE4_FOUND)
	foreach(dimension 16 32 64 128)
		install(FILES cr${dimension}-app-qyzis.png DESTINATION ${ICON_INSTALL_DIR}/hicolor/${dimension}x${dimension}/apps RENAME qyzis.png)
	endforeach(dimension)
endif (KDE4_FOUND)

if (APPLE)
	# install icon for bundle
	install(FILES qyzis.icns DESTINATION ${APPLE_APPLICATION_DIR}/qyzis.app/Contents/Resources)
endif (APPLE)

