cmake_minimum_required(VERSION 2.8.4)

project(CTKP_Demo)


#
#FIND CTK
#
FIND_PACKAGE(CTK)
#IF (CTK_USE_FILE)
#  INCLUDE (${CTK_USE_FILE} )
#ELSE (CTK_USE_FILE)
#  MESSAGE( FATAL_ERROR "This application requires CTK. One of these components is missing. Please verify configuration")
#ENDIF (CTK_USE_FILE)

#
#FIND Qt4
#
FIND_PACKAGE(Qt4)
IF (QT_USE_FILE)
  INCLUDE (${QT_USE_FILE} )
ELSE (QT_USE_FILE)
  MESSAGE( FATAL_ERROR "This application requires QT. One of these components is missing. Please verify configuration")
ENDIF (QT_USE_FILE)




set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")


macro(GetMyTargetLibraries all_target_libraries varname)
  set(re_ctkplugin "^at_voxelmaster_[a-zA-Z0-9_]+$")
  set(_tmp_list)
  list(APPEND _tmp_list ${all_target_libraries})
  ctkMacroListFilter(_tmp_list re_ctkplugin OUTPUT_VARIABLE ${varname})
endmacro()

set(plugins
  plugins/at.voxelmaster.loadDicomPlugin:ON
  plugins/at.voxelmaster.trackerNavigationPlugin:ON
  plugins/at.voxelmaster.microscopeCommunicationPlugin:ON
  plugins/at.voxelmaster.microscopeNavigationPlugin:ON
  plugins/at.voxelmaster.trackerCommunicationPlugin:ON
)

ctkMacroSetupPlugins(${plugins})


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
  ${CTK_Demo_SOURCE_DIR}
  ${CTK_Demo_BINARY_DIR}
  ${CTK_Demo_SOURCE_DIR}/src
  ${CTK_Demo_SOURCE_DIR}/resources
  ${CTK_Demo_SOURCE_DIR}/gui
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CTK_INCLUDE_DIRS} 

  )

set(my_libs
  CTKPluginFramework
)

SET(CTK_Demo_GUI_SRCS
  	gui/mainWindowGUI.ui
   )

SET(CTK_Demo_RCCS 
    resources/CTK_Demo.qrc
   )



# generate rules for building source files from the resources
QT4_ADD_RESOURCES(CTK_Demo_RCC_SRCS 
                  ${CTK_Demo_RCCS}
                 )

QT4_WRAP_UI(CTK_Demo_QT_UI_SRCS 
            ${CTK_Demo_GUI_SRCS}
           )

SET(CTK_Demo_SRCS
    src/main.cxx
    src/mainWindowGUI.cxx
 
   )
  
SET(CTK_Demo_HDRS 
    src/mainWindowGUI.h

   )  


QT4_WRAP_CPP(CTK_Demo_MOC_SRCS)
QT4_AUTOMOC(${CTK_Demo_SRCS} ${CTK_Demo_HDRS})

ADD_EXECUTABLE(CTK_Demo ${CTK_Demo_QT_UI_SRCS} ${CTK_Demo_RCC_SRCS} ${CTK_Demo_SRCS} ${CTK_Demo_HDRS} ${CTK_Demo_MOC_SRCS} )

##TARGET_LINK_LIBRARIES(CTK_Demo QVTK ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} ${CTK_LIBRARIES} ${my_libs} )
TARGET_LINK_LIBRARIES(CTK_Demo ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} ${CTK_LIBRARIES} ${my_libs} )


