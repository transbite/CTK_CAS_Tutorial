
#if defined(_MSC_VER)
//  Warning about: identifier was truncated to '255' characters
//  in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4284 )
#endif

#if defined (_WIN32)

#include <direct.h>
#include <windows.h>


#else


#endif

#include "mainWindowGUI.h"

#include <stdio.h>
#include <iostream>


/** QT header files */
#include <QtGui>
#include <QDialog>
#include <QApplication>
#include <QObject>
#include <QMainWindow>
//#include <QtTest/QTest>
#include <QtCore/QTimer>
#include <QProcess>
#include <QString>
#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>
#include <QEvent>
#include <QCursor>
#include <QColorDialog>
#include <QString>
#include <QWidget>

#include <ctkPluginFrameworkLauncher.h>
#include <ctkPluginFrameworkFactory.h>
#include <ctkPluginFramework.h>
#include <ctkPluginException.h>
#include <ctkPluginContext.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>
#include <service/event/ctkEventConstants.h>

#include "plugins/at.voxelmaster.loadDicomPlugin/loadDicomInterface.h"
#include "plugins/at.voxelmaster.trackerNavigationPlugin/trackerNavigationInterface.h"
#include "plugins/at.voxelmaster.microscopeNavigationPlugin/microscopeNavigationInterface.h"


bool startPlugin(QString pluginName){
		bool succeeded= false;

		try
		{
			succeeded = ctkPluginFrameworkLauncher::start(pluginName);
		}

		catch(ctkPluginException &e)
		{
			std::cout << "Error in " <<pluginName.toStdString() << " "<< e.message().toStdString() << std::endl;
			const ctkException* e2 = e.cause();
			if(e2)
				std::cout << e2->message().toStdString() << std::endl;

			return false;
		}
		catch(ctkRuntimeException &e)
		{
			std::cout << "Error in " <<pluginName.toStdString() << " "<< e.what() << std::endl;
			const ctkException* e2 = e.cause();
			if(e2)
				std::cout << e2->message().toStdString() << std::endl;

			return false;
		}


		return succeeded;
}


int main(int argc, char** argv)
{


	QApplication app(argc, argv);

	app.setOrganizationName("Voexelmaster");
	app.setOrganizationDomain("voxelmaster.at");
	app.setApplicationName("CTK Demo");


	mainWindowGUI mainWindow;

	mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> CTK Demo Application is started \n");

	ctkPluginFrameworkLauncher::addSearchPath("plugins",true);


	if(startPlugin("org.commontk.eventadmin")){
		mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> EventAdmin loading: succeeded \n");
	}else mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> EventAdmin loading: not succeeded \n");
	
	if(startPlugin("at.voxelmaster.loadDicomPlugin")){
		ctkServiceReference loadDicomReference = ctkPluginFrameworkLauncher::getPluginContext()->getServiceReference<loadDicomInterface>();
		loadDicomInterface* loadDicomInterfaceInstance = ctkPluginFrameworkLauncher::getPluginContext()->getService<loadDicomInterface>(loadDicomReference);

		mainWindow.getMainWindowGUI()->documentTreeToolBox->addItem(loadDicomInterfaceInstance->toolFrameLoadDicomInterfaceWidget, loadDicomInterfaceInstance->toolFrameLoadDicomInterfaceWidgetString);
		mainWindow.getMainWindowGUI()->documentTreeToolBox->setCurrentWidget(loadDicomInterfaceInstance->toolFrameLoadDicomInterfaceWidget);

		mainWindow.getMainWindowGUI()->mainViewStackedWidget->addWidget(loadDicomInterfaceInstance->mainFrameVisualizationDicomViewInterfaceWidget);
		mainWindow.getMainWindowGUI()->mainViewStackedWidget->setCurrentWidget(loadDicomInterfaceInstance->mainFrameVisualizationDicomViewInterfaceWidget);

		mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> Load Dicom plugin loading: succeeded \n");
	}
	else mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> Load Dicom plugin loading: not succeeded \n");



	if(startPlugin("at.voxelmaster.trackerCommunicationPlugin")){
		mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> Tracker Communication plugin loading: succeeded \n");
	}
	else mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> Tracker Communication plugin loading: not succeeded \n");


	if(startPlugin("at.voxelmaster.trackerNavigationPlugin")){
		ctkServiceReference trackerNavigationReference = ctkPluginFrameworkLauncher::getPluginContext()->getServiceReference<trackerNavigationInterface>();
		trackerNavigationInterface* trackerNavigationInterfaceInstance = ctkPluginFrameworkLauncher::getPluginContext()->getService<trackerNavigationInterface>(trackerNavigationReference);

		mainWindow.getMainWindowGUI()->documentTreeToolBox->addItem(trackerNavigationInterfaceInstance->toolFrameTrackerNavigationInterfaceWidget,trackerNavigationInterfaceInstance->toolFrameTrackerNavigationWidgetString);

		mainWindow.getMainWindowGUI()->mainViewStackedWidget->addWidget(trackerNavigationInterfaceInstance->mainFrameVisualizationTrackerNavigationViewInterfaceWidget);

		mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> Tracker Navigation plugin loading: succeeded \n");
	}
	else mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> Tracker Navigation plugin loading: not succeeded \n");


	if(startPlugin("at.voxelmaster.microscopeCommunicationPlugin")){
		mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> Microscope Communication plugin loading: succeeded \n");
	}
	else mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> Microscope Communication plugin loading: not succeeded \n");



	if(startPlugin("at.voxelmaster.microscopeNavigationPlugin")){
		ctkServiceReference microscopeNavigationReference = ctkPluginFrameworkLauncher::getPluginContext()->getServiceReference<microscopeNavigationInterface>();
		microscopeNavigationInterface* microscopeNavigationInterfaceInstance = ctkPluginFrameworkLauncher::getPluginContext()->getService<microscopeNavigationInterface>(microscopeNavigationReference);

		mainWindow.getMainWindowGUI()->documentTreeToolBox->addItem(microscopeNavigationInterfaceInstance->toolFrameMicroscopeNavigationInterfaceWidget, microscopeNavigationInterfaceInstance->toolFrameMicroscopeNavigationInterfaceWidgetString);

		mainWindow.getMainWindowGUI()->mainViewStackedWidget->addWidget(microscopeNavigationInterfaceInstance->mainFrameVisualizationMicroscopeViewInterfaceWidget);
		mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> Microscope Navigation plugin loading: succeeded \n");

	}else mainWindow.getMainWindowGUI()->toolFrameTextEdit->insertPlainText("-> Microscope Navigation plugin loading: not succeeded \n");


	mainWindow.showMaximized();

	return app.exec();

}





