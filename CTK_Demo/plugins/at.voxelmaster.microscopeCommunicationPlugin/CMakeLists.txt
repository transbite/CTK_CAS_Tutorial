PROJECT(at_voxelmaster_microscopeCommunicationPlugin)

SET(PLUGIN_export_directive "at_voxelmaster_microscopeCommunicationPlugin_EXPORT")

SET(PLUGIN_SRCS
    microscopeCommunicationPlugin.cpp
	BGAPIVideoBuffer.cpp
)


# Files which should be processed by Qts moc
SET(PLUGIN_MOC_SRCS
    microscopeCommunicationPlugin.h
    microscopeVideoInterface.h
    BGAPIVideoBuffer.h
)


# Qt Designer files which should be processed by Qts uic
SET(PLUGIN_UI_FORMS

)


# QRC Files which should be compiled into the plugin
SET(PLUGIN_resources

)


INCLUDE_DIRECTORIES(
/usr/include/QtMobility
/usr/include/QtMultimediaKit
)


#
#FIND QtMobility And QtMultimediaKit DIRECTORIES
#
FIND_LIBRARY(QtMobility_DIRECTORY QtMobility)
IF(QtMobility_DIRECTORY)
	INCLUDE_DIRECTORIES(${QtMobility_DIRECTORY}) 
ELSE(QtMobility_DIRECTORY)
  	MESSAGE(INFORMATION " This application requires QtMobility directory. One of these components is missing. Please verify configuration")
ENDIF(QtMobility_DIRECTORY)


FIND_LIBRARY(QtMultimediaKit_DIRECTORY QtMultimediaKit)
IF(QtMultimediaKit_DIRECTORY)
	INCLUDE_DIRECTORIES(${QtMultimediaKit_DIRECTORY}) 
ELSE(QtMultimediaKit_DIRECTORY)
  	MESSAGE(INFORMATION " This application requires QtMultimediaKit directory. One of these components is missing. Please verify configuration")
ENDIF(QtMultimediaKit_DIRECTORY)




SET(CMAKE_CXX_FLAGS "-D_GNULINUX")


#
#FIND Baumer_Gapi LIBRARIES
#
FIND_LIBRARY(BAUMERGAPI_LIBRARY bgapi)
IF(BAUMERGAPI_LIBRARY)
	SET(${BAUMERGAPI_LIBRARY}) 
ELSE(BAUMERGAPI_LIBRARY)
  	MESSAGE(INFORMATION " This application requires bgapi library. One of these components is missing. Please verify configuration")
ENDIF(BAUMERGAPI_LIBRARY)




# Copy library files from source directory to binary directory of CTK_Demo 
FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/ DESTINATION ${CMAKE_BINARY_DIR})


#Compute the plugin dependencies
ctkFunctionGetTargetLibraries(PLUGIN_target_libraries)

ctkMacroBuildPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${PLUGIN_export_directive}
  SRCS ${PLUGIN_SRCS}
  MOC_SRCS ${PLUGIN_MOC_SRCS}
  UI_FORMS ${PLUGIN_UI_FORMS}
  RESOURCES ${PLUGIN_resources}
  TARGET_LIBRARIES ${PLUGIN_target_libraries} -lQtMultimediaKit  ${BAUMERGAPI_LIBRARY}
)
