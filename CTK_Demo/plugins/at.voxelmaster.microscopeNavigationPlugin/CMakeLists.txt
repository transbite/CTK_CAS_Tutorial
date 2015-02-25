PROJECT(at_voxelmaster_microscopeNavigationPlugin)

SET(PLUGIN_export_directive "at_voxelmaster_microscopeNavigationPlugin_EXPORT")

SET(PLUGIN_SRCS
  microscopeNavigationPlugin.cpp
  videowidgetsurface.cpp
  VideoWidget.cpp
  QVideoDecoder.cpp
  QVideoEncoder.cpp
)

# Files which should be processed by Qts moc
SET(PLUGIN_MOC_SRCS
  microscopeNavigationPlugin.h
  microscopeNavigationInterface.h
  videowidgetsurface.h
  VideoWidget.h
  QVideoDecoder.h
  QVideoEncoder.h
)



# Qt Designer files which should be processed by Qts uic
SET(PLUGIN_UI_FORMS
  toolFrameMicroscopeNavigationGUI.ui
  mainFrameMicroscopeViewGUI.ui
)

# QRC Files which should be compiled into the plugin
SET(PLUGIN_resources
)


#
#FIND ffmpeg LIBRARIES
#
FIND_LIBRARY(FFMPEG_AVUTIL_LIBRARY avutil)
IF(FFMPEG_AVUTIL_LIBRARY)
	SET(${FFMPEG_AVUTIL_LIBRARY}) 
ELSE(FFMPEG_AVUTIL_LIBRARY)
  	MESSAGE(INFORMATION " This application requires avutil library. One of these components is missing. Please verify configuration")
ENDIF(FFMPEG_AVUTIL_LIBRARY)


FIND_LIBRARY(FFMPEG_AVCODEC_LIBRARY avcodec)
IF(FFMPEG_AVCODEC_LIBRARY)
	SET(${FFMPEG_AVCODEC_LIBRARY}) 
ELSE(FFMPEG_AVCODEC_LIBRARY)
  	MESSAGE(INFORMATION " This application requires avcodec library. One of these components is missing. Please verify configuration")
ENDIF(FFMPEG_AVCODEC_LIBRARY)


FIND_LIBRARY(FFMPEG_AVFORMAT_LIBRARY avformat)
IF(FFMPEG_AVFORMAT_LIBRARY)
	SET(${FFMPEG_AVFORMAT_LIBRARY}) 
ELSE(FFMPEG_AVFORMAT_LIBRARY)
  	MESSAGE(INFORMATION " This application requires avformat library. One of these components is missing. Please verify configuration")
ENDIF(FFMPEG_AVFORMAT_LIBRARY)


FIND_LIBRARY(FFMPEG_SWSCALE_LIBRARY swscale)
IF(FFMPEG_SWSCALE_LIBRARY)
	SET(${FFMPEG_SWSCALE_LIBRARY}) 
ELSE(FFMPEG_SWSCALE_LIBRARY)
  	MESSAGE(INFORMATION " This application requires swscale library. One of these components is missing. Please verify configuration")
ENDIF(FFMPEG_SWSCALE_LIBRARY)



INCLUDE_DIRECTORIES(
/usr/include/QtMobility
/usr/include/QtMultimediaKit
)




SET(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")

#Compute the plugin dependencies
ctkFunctionGetTargetLibraries(PLUGIN_target_libraries)

ctkMacroBuildPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${PLUGIN_export_directive}
  SRCS ${PLUGIN_SRCS}
  MOC_SRCS ${PLUGIN_MOC_SRCS}
  UI_FORMS ${PLUGIN_UI_FORMS}
  RESOURCES ${PLUGIN_resources}
  TARGET_LIBRARIES ${PLUGIN_target_libraries} -lQtMultimediaKit 
  ) 
  
#${FFMPEG_AVFORMAT_LIBRARY} ${FFMPEG_SWSCALE_LIBRARY} ${FFMPEG_AVCODEC_LIBRARY} ${FFMPEG_AVUTIL_LIBRARY}
