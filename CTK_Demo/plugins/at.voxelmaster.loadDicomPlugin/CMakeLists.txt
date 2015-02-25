PROJECT(at_voxelmaster_loadDicomPlugin)

SET(PLUGIN_export_directive "at_voxelmaster_loadDicomPlugin_EXPORT")




#
#FIND IGSTK
#
FIND_PACKAGE(IGSTK)
IF (IGSTK_USE_FILE)
  INCLUDE (${IGSTK_USE_FILE} )
ELSE (IGSTK_USE_FILE)
  MESSAGE( FATAL_ERROR "This application requires IGSTK. One of these components is missing. Please verify configuration")
ENDIF (IGSTK_USE_FILE)


#
#FIND OPEN IGTLINK
#
FIND_PACKAGE(OpenIGTLink)
IF (OpenIGTLink_USE_FILE)
  INCLUDE (${OpenIGTLink_USE_FILE} )
ELSE (OpenIGTLink_USE_FILE)
  MESSAGE( FATAL_ERROR "This application requires OpenIGTLink. One of these components is missing. Please verify configuration")
ENDIF (OpenIGTLink_USE_FILE)



#
#FIND ITK
#
FIND_PACKAGE(ITK)
IF (USE_ITK_FILE)
  INCLUDE (${USE_ITK_FILE})
ELSE (USE_ITK_FILE)
  MESSAGE( FATAL_ERROR "This application requires ITK. One of these components is missing. Please verify configuration")
ENDIF (USE_ITK_FILE)



#
#FIND VTK
#
FIND_PACKAGE(VTK)
IF(USE_VTK_FILE)
	INCLUDE(${USE_VTK_FILE})
ELSE (USE_VTK_FILE)
	MESSAGE( FATAL_ERROR "This application requires VTK. One of these components is missing. Please verify configuration")
ENDIF (USE_VTK_FILE)




SET(PLUGIN_SRCS
    loadDicomPlugin.cpp
  
)

# Files which should be processed by Qts moc
SET(PLUGIN_MOC_SRCS
	loadDicomPlugin.h
	loadDicomInterface.h
	dicomDataInterface.h
)

# Qt Designer files which should be processed by Qts uic
SET(PLUGIN_UI_FORMS
    toolFrameLoadDicomGUI.ui
    mainFrameDicomViewGUI.ui
)

# QRC Files which should be compiled into the plugin
SET(PLUGIN_resources

)

#Compute the plugin dependencies
ctkFunctionGetTargetLibraries(PLUGIN_target_libraries)

ctkMacroBuildPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${PLUGIN_export_directive}
  SRCS ${PLUGIN_SRCS}
  MOC_SRCS ${PLUGIN_MOC_SRCS}
  UI_FORMS ${PLUGIN_UI_FORMS}
  RESOURCES ${PLUGIN_resources}
  TARGET_LIBRARIES ${PLUGIN_target_libraries} ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} ${CTK_LIBRARIES} IGSTK QVTK 

)


