PROJECT(at_voxelmaster_trackerCommunicationPlugin)

SET(PLUGIN_export_directive "at_voxelmaster_trackerCommunicationPlugin_EXPORT")


SET(PLUGIN_SRCS
    optotrak.cpp
    trackerCommunicationPlugin.cpp
    datatypes.h
    optotrak.h
)


# Files which should be processed by Qts moc
SET(PLUGIN_MOC_SRCS
    trackerCommunicationPlugin.h
)


#FIND_LIBRARY(OPTOTRAK_LIBRARY ndilib)  
#SET(${OPTOTRAK_LIBRARY}) 


#
#FIND Optotrak LIBRARIES
#
FIND_LIBRARY(OPTOTRAK_LIBRARY liboapi)
IF(OPTOTRAK_LIBRARY)
	SET(${OPTOTRAK_LIBRARY}) 
ELSE(OPTOTRAK_LIBRARY)
  	MESSAGE(INFORMATION " This application requires liboapi library. One of these components is missing. Please verify configuration")
ENDIF(OPTOTRAK_LIBRARY)




#SET(CMAKE_C_FLAGS " -O0 -g3 -W -Wimplicit-int -Wbad-function-cast -Wparentheses -Wmissing-braces -Wreturn-type -Wswitch -Wtrigraphs -Wunused-function -Wunused-label -Wfloat-equal -Wcast-align -Wmissing-prototypes -Wmissing-declarations -Wunreachable-code -Wredundant-decls -Winline -Wundef -Wno-unused-parameter -Wno-unused-value -DPLATFORM_LINUX -I ./"
#  )


#SET(CMAKE_CXX_FLAGS " -O0 -g3 -W -Wimplicit-int -Wbad-function-cast -Wparentheses -Wmissing-braces -Wreturn-type -Wswitch -Wtrigraphs -Wunused-function -Wunused-label -Wfloat-equal -Wcast-align -Wmissing-prototypes -Wmissing-declarations -Wunreachable-code -Wredundant-decls -Winline -Wundef -Wno-unused-parameter -Wno-unused-value -DPLATFORM_LINUX -I ./"
#   )



# Qt Designer files which should be processed by Qts uic
SET(PLUGIN_UI_FORMS

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
  INCLUDE_DIRECTORIES ${at_voxelmaster_trackerCommunicationPlugin_SOURCE_DIR} 
  RESOURCES ${PLUGIN_resources}
  TARGET_LIBRARIES  ${PLUGIN_target_libraries} 
)
# ${OPTOTRAK_LIBRARY}

