PROJECT(at_voxelmaster_trackerNavigationPlugin)

SET(PLUGIN_export_directive "at_voxelmaster_trackerNavigationPlugin_EXPORT")




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



INCLUDE_DIRECTORIES(
${at_voxelmaster_trackerNavigationPlugin_SOURCE_DIR}/igstkHelpers
) 



SET(PLUGIN_SRCS
  	trackerNavigationPlugin.cpp
	igstkHelpers/igstkFiducialsPlan.cxx
	igstkHelpers/igstkFiducialsPlanIO.cxx
	igstkHelpers/igstkPathPlan.cxx
	igstkHelpers/igstkPathPlanIO.cxx
)


# Files which should be processed by Qts moc
SET(PLUGIN_MOC_SRCS
	trackerNavigationPlugin.h
	trackerNavigationInterface.h
	igstkHelpers/igstkFiducialsPlan.h
	igstkHelpers/igstkFiducialsPlanIO.h
	igstkHelpers/igstkPathPlan.h
	igstkHelpers/igstkPathPlanIO.h
)


# Qt Designer files which should be processed by Qts uic
SET(PLUGIN_UI_FORMS
    toolFrameTrackerNavigationGUI.ui
    mainFrameTrackerNavigationViewGUI.ui
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
  TARGET_LIBRARIES IGSTK ${PLUGIN_target_libraries} ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} IGSTK QVTK
)


