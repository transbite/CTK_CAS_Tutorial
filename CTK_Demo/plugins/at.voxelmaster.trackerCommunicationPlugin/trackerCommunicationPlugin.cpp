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



#include "trackerCommunicationPlugin.h"

#include <iostream>
#include <fstream>
#include <set>

#include <unistd.h>


#include <QtPlugin>
#include <QString>
#include <QLibrary>

#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>


trackerCommunicationPlugin* trackerCommunicationPlugin::instance = 0;

trackerCommunicationPlugin::trackerCommunicationPlugin()
  : context(0)
{

	std::cout << "Tracker Communication plugin constructor" << std::endl;


}



trackerCommunicationPlugin::~trackerCommunicationPlugin()
{

}



void trackerCommunicationPlugin::start(ctkPluginContext* context)
{
	std::cout << "Tracker Communication plugin start" << std::endl;

	instance = this;
	this->context = context;

	float coordsX[] = {1.4,1.2,1.5};
	float coordsY[] = {2.4,2.2,2.5};
	float coordsZ[] = {3.4,3.2,3.5};

	igstkNDICertusTrackerCommunicationOptotrak();

}


void trackerCommunicationPlugin::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)
}



trackerCommunicationPlugin* trackerCommunicationPlugin::getInstance()
{
	return instance;
}


ctkPluginContext* trackerCommunicationPlugin::getPluginContext() const
{
	std::cout << "Tracker Communication context" << std::endl;
	return context;
}


void trackerCommunicationPlugin::updateTrackerCoordinates(float markerCoordsX[],float markerCoordsY[],float markerCoordsZ[],float markerRotX[],float markerRotY[],float markerRotZ[],int markerAmount)
  {
    ctkServiceReference ref = context->getServiceReference<ctkEventAdmin>();
    if (ref)
    {
      ctkEventAdmin* eventAdmin = context->getService<ctkEventAdmin>(ref);
      ctkDictionary properties;
      properties["markerAmount"] = markerAmount;
      sys_time_t t;
      system_time(&t);
      properties["timestamp"] = time_to_msec(t);
      for(int i=1;i<=markerAmount;i++){
    	  properties["xCoordMarker"+QString::number(i)] = markerCoordsX[i-1];
    	  properties["yCoordMarker"+QString::number(i)] = markerCoordsY[i-1];
    	  properties["zCoordMarker"+QString::number(i)] = markerCoordsZ[i-1];
    	  properties["xRotMarker"+QString::number(i)] = markerRotX[i-1];
    	  properties["yRotMarker"+QString::number(i)] = markerRotY[i-1];
    	  properties["zRotMarker"+QString::number(i)] = markerRotZ[i-1];
      }
      ctkEvent reportGeneratedEvent("TrackingCoordinatesUpdated", properties);
      eventAdmin->sendEvent(reportGeneratedEvent);

      //sleep(50000);
      //TransputerShutdownSystem();

    }
  }



void trackerCommunicationPlugin::igstkNDICertusTrackerCommunicationOptotrak()
{
	/*
	 QLibrary myLib("/home/florian/workspace/dynamicSharedLibrary/bin/libshared.so");
	 myLib.setLoadHints(QLibrary::ExportExternalSymbolsHint);
	 myLib.loadHints();
	 myLib.load();
	 if(myLib.isLoaded()){
		 myclass m;
		 cout << m.getx() << endl;
	     m.setx(10);
	     cout << m.getx() << endl;
	 }
	 */

	threadInstance = new optotrak();
	threadInstance->start();



}





Q_EXPORT_PLUGIN2(at_voxelmaster_trackerCommunicationPlugin, trackerCommunicationPlugin)


