/*=============================================================================

  Library: CTK

  Copyright (c) 4DVisualizationLab

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/




#include <stdio.h>
#include <stdlib.h>


#ifndef trackerCommunicationPlugin_H
#define trackerCommunicationPlugin_H




/** CTK header files */
#include <ctkPluginActivator.h>

/** QT header files */
#include <QString>
#include <QFileInfo>
#include <QErrorMessage>
#include <QFileDialog>
#include <QtGui>
#include <QObject>
#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QtCore/QTimer>
#include <QDesktopServices>
#include <QDialog>
#include <QFileDialog>
#include "QVTKWidget.h"
#include <QPushButton>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QWidget>
#include <QThread>


#include "optotrak.h"

/** IGSTK header files */
/*
#include "igstkSystemInformation.h"
#include "igstkSerialCommunication.h"
#include "igstkNDICertusTracker.h"
#include "igstkNDICertusTrackerTool.h"
//#include "igstkAuroraTrackerTool.h"
#include "igstkTransform.h"
#include "igstkTransformObserver.h"
#include "igstkLogger.h"

*/
/** ITK header files */
//#include "itkCommand.h"
//#include "itkStdStreamLogOutput.h"
//#include "itkVector.h"
//#include "itkVersor.h"




// public itk::Command

#if !defined(_WIN32) && !defined(_WIN64) // Linux - Unix
    #  include <sys/time.h>
    typedef timeval sys_time_t;
    inline void system_time(sys_time_t* t) {
        gettimeofday(t, NULL);
    }
    inline long long time_to_msec(const sys_time_t& t) {
        return t.tv_sec * 1000LL + t.tv_usec / 1000;
    }
    #else // Windows and MinGW
    #  include <sys/timeb.h>
    typedef _timeb sys_time_t;
    inline void system_time(sys_time_t* t) { _ftime(t); }
    inline long long time_to_msec(const sys_time_t& t) {
        return t.time * 1000LL + t.millitm;
    }
#endif



class trackerCommunicationPlugin :  public QObject, public ctkPluginActivator
{
	Q_OBJECT
	Q_INTERFACES(ctkPluginActivator)

public:

	trackerCommunicationPlugin();
	~trackerCommunicationPlugin();

	void start(ctkPluginContext* context);
	void stop(ctkPluginContext* context);

	static trackerCommunicationPlugin* getInstance();

	ctkPluginContext* getPluginContext() const;

/*
	typedef  trackerCommunicationPlugin   							Self;
	typedef  itk::Command                							Superclass;
	typedef itk::SmartPointer<Self>      							Pointer;
	itkNewMacro(Self);



public:

	void Execute(itk::Object *caller, const itk::EventObject & event)
    {
		Execute( (const itk::Object *)caller, event);
    }

	void Execute(const itk::Object * object, const itk::EventObject & event)
    {
		// don't print "CompletedEvent", only print interesting events
		if(!igstk::CompletedEvent().CheckEvent(&event) && !itk::DeleteEvent().CheckEvent(&event))
		{
			std::cout << event.GetEventName() << std::endl;
		}
    }
*/

	void updateTrackerCoordinates(float[],float[],float[],float[],float[],float[],int);

private:

	static trackerCommunicationPlugin							*instance;
	ctkPluginContext											*context;



	void igstkNDICertusTrackerCommunicationOptotrak();

	optotrak* threadInstance;

}; // trackerCommunicationPlugin

#endif // trackerCommunicationPlugin_H
