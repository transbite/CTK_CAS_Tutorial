/*
 * microscopeVideoServiceTracker.h
 *
 *  Created on: 25.07.2013
 *      Author: florian
 */

#ifndef MICROSCOPEVIDEOSERVICETRACKER_H_
#define MICROSCOPEVIDEOSERVICETRACKER_H_

#include "microscopeNavigationPlugin.h"

#include <QObject>
#include <QString>
#include <ctkPluginActivator.h>


#include <iostream>
#include <fstream>

#include <QtPlugin>

#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>
#include <ctkServiceEvent.h>
#include "ctkServiceTrackerCustomizer.h"
#include <ctkServiceReference.h>
#include <QtMultimediaKit/qvideoframe.h>


class microscopeVideoServiceTracker :public ctkServiceTrackerCustomizer<microscopeVideoInterface*>
  {



public:
	microscopeVideoServiceTracker(ctkPluginContext* context,microscopeNavigationPlugin* microscopeNavigationPluginObject){
		this->context= context;
		this->microscopeNavigationPluginObject= microscopeNavigationPluginObject;
	}


private:

	ctkPluginContext* context;
	microscopeNavigationPlugin* microscopeNavigationPluginObject;


	microscopeVideoInterface* addingService (const ctkServiceReference &reference){
		std::cout << "microscopeVideo Tracker: start"  << std::endl;

		if(reference.getProperty("microscopeVideoServiceEventID")!=QVariant::Invalid && reference.getProperty("microscopeVideoServiceEventID").toInt(new bool())>0){
			modifiedService(reference,context->getService<microscopeVideoInterface>(reference));
		}

		return context->getService<microscopeVideoInterface>(reference);
	}

	void modifiedService (const ctkServiceReference &reference, microscopeVideoInterface* service){
		microscopeNavigationPluginObject->visualizeVideo(service->microscopeVideoInterfaceBuffers,*service->size,service->pixelFormat);

	}


	void removedService (const ctkServiceReference &reference, microscopeVideoInterface* service){

	}

};



#endif /* MICROSCOPEVIDEOSERVICETRACKER_H_ */
