/*
 * dicomDataServiceTracker.h
 *
 *  Created on: 25.07.2013
 *      Author: florian
 */

#ifndef DICOMDATASERVICETRACKER_H_
#define DICOMDATASERVICETRACKER_H_

#include "trackerNavigationPlugin.h"

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


class dicomDataServiceTracker :public ctkServiceTrackerCustomizer<dicomDataInterface*>
  {



public:
	dicomDataServiceTracker(ctkPluginContext* context,trackerNavigationPlugin* trackerNavigationPluginObject){
		this->context= context;
		this->trackerNavigationPluginObject= trackerNavigationPluginObject;
	}


private:

	ctkPluginContext* context;
	trackerNavigationPlugin* trackerNavigationPluginObject;


	dicomDataInterface* addingService (const ctkServiceReference &reference){
		std::cout << "dicomData Tracker: start"  << std::endl;
		return context->getService<dicomDataInterface>(reference);

	}

	void modifiedService (const ctkServiceReference &reference, dicomDataInterface* service){

		std::cout << "trackerNavigation: "<<service->dicomSpatialObject->GetMTime() << "\nEventID: " << reference.getProperty("dicomDataServiceEventID").toString().toStdString() << std::endl;

		trackerNavigationPluginObject->readCTDataset(service->dicomSpatialObject);

	}


	void removedService (const ctkServiceReference &reference, dicomDataInterface* service){

	}


};



#endif /* DICOMDATASERVICETRACKER_H_ */
