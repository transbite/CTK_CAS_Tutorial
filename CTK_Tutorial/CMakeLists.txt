cmake_minimum_required(VERSION 2.8.4)

project(CTK_Tutorial)


#
#FIND CTK
#
FIND_PACKAGE(CTK REQUIRED)
##IF (CTK_USE_FILE)
##  INCLUDE (${CTK_USE_FILE} )
##ELSE (CTK_USE_FILE)
##  MESSAGE( FATAL_ERROR "This application requires CTK. One of these components is missing. Please verify configuration")
##ENDIF (CTK_USE_FILE)

#
#FIND Qt4
#
##FIND_PACKAGE(Qt4)
##IF (QT_USE_FILE)
##  INCLUDE (${QT_USE_FILE} )
##ELSE (QT_USE_FILE)
##  MESSAGE( FATAL_ERROR "This application requires QT. One of these components is missing. Please verify configuration")
##ENDIF (QT_USE_FILE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets)
find_package(Qt5OpenGL)
find_package(Qt5Concurrent)
find_package(Qt5Xml)
find_package(Qt5Sql)
find_package(Qt5WebKitWidgets)
find_package(Qt5UiTools)
find_package(Qt5XmlPatterns)
find_package(Qt5Network)
find_package(Qt5Script)

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

include_directories(${CTK_INCLUDE_DIRS})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")


macro(GetMyTargetLibraries all_target_libraries varname)
  set(re_ctkplugin "^at_voxelmaster_[a-zA-Z0-9_]+$")
  set(_tmp_list)
  list(APPEND _tmp_list ${all_target_libraries})
  ctkMacroListFilter(_tmp_list re_ctkplugin OUTPUT_VARIABLE ${varname})
endmacro()

set(plugins
  plugins/at.voxelmaster.emptyPlugin:ON   
  plugins/at.voxelmaster.eventBasedCommunicationSenderPlugin:ON
  plugins/at.voxelmaster.eventBasedCommunicationReceiverPlugin:ON
  plugins/at.voxelmaster.serviceBasedCommunicationSenderPlugin:ON
  plugins/at.voxelmaster.serviceBasedCommunicationReceiverPlugin:ON

)

ctkMacroSetupPlugins(${plugins})



INCLUDE_DIRECTORIES(
  ${CTK_Tutorial_SOURCE_DIR}
  ${CTK_Tutorial_BINARY_DIR}
  ${CTK_Tutorial_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}

  )

set(my_libs
  CTKPluginFramework
)

SET(CTK_Tutorial_GUI_SRCS
  	
   )

SET(CTK_Tutorial_RCCS 

   )



# generate rules for building source files from the resources
##QT4_ADD_RESOURCES(CTK_Tutorial_RCC_SRCS 
##                  ${CTK_Tutorial_RCCS}
##                 )

##QT4_WRAP_UI(CTK_Tutorial_QT_UI_SRCS 
##            ${CTK_Tutorial_GUI_SRCS}
##           )

qt5_add_resources(CTK_Tutorial_RCC_SRCS ${CTK_Tutorial_RCCS})
qt5_wrap_ui(CTK_Tutorial_QT_UI_SRCS ${CTK_Tutorial_GUI_SRCS})

SET(CTK_Tutorial_SRCS
    src/main.cpp
  
   )
  
##SET(CTK_Tutorial_HDRS 
##
##   )  


##QT4_WRAP_CPP(CTK_Tutorial_MOC_SRCS)

qt5_wrap_cpp(CTK_Tutorial_MOC_SRCS)

##QT4_AUTOMOC(${CTK_Tutorial_SRCS} ${CTK_Tutorial_HDRS})

##ADD_EXECUTABLE(CTK_Tutorial ${CTK_Tutorial_QT_UI_SRCS} ${CTK_Tutorial_RCC_SRCS} ${CTK_Tutorial_SRCS} ${CTK_Tutorial_HDRS} ${CTK_Tutorial_MOC_SRCS} )
add_executable(CTK_Tutorial ${CTK_Tutorial_QT_UI_SRCS} ${CTK_Tutorial_RCC_SRCS} ${CTK_Tutorial_SRCS} ${CTK_Tutorial_MOC_SRCS})

##TARGET_LINK_LIBRARIES(CTK_Tutorial QVTK ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} ${CTK_LIBRARIES} ${my_libs} )
target_link_libraries(CTK_Tutorial ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${CTK_LIBRARIES} ${my_libs})

qt5_use_modules(CTK_Tutorial Widgets OpenGL Concurrent Xml Sql WebKitWidgets UiTools XmlPatterns Network Script)


