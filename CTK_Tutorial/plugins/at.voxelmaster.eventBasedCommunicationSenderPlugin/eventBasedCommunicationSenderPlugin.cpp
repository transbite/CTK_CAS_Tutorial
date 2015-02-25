/*=============================================================================

  Library: CTK

  Copyright (c) <voxelmaster/ 4D-Visualization, Univ. ENT Clinic, Innsbruck Medical University>

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



#include "eventBasedCommunicationSenderPlugin.h"

#include <iostream>

#include <QtPlugin>


#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>




eventBasedCommunicationSenderPlugin* eventBasedCommunicationSenderPlugin::instance = 0;

eventBasedCommunicationSenderPlugin::eventBasedCommunicationSenderPlugin()
  : context(0)
{

	std::cout << "eventBasedCommunicationSenderPlugin constructor" << std::endl;

}

eventBasedCommunicationSenderPlugin::~eventBasedCommunicationSenderPlugin()
{
  
}

void eventBasedCommunicationSenderPlugin::start(ctkPluginContext* context)
{
	instance = this;
	this->context = context;

	sendText= "Send Event Text ";
	sendActualLoopNumber=0;
	sendNumberOfLoop= 10;

	this->sendEventToReceiver(sendText, sendActualLoopNumber, sendNumberOfLoop);

	std::cout << "eventBasedCommunicationSenderPlugin start" << std::endl;
}

void eventBasedCommunicationSenderPlugin::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)
}

eventBasedCommunicationSenderPlugin* eventBasedCommunicationSenderPlugin::getInstance()
{
	return instance;
}

ctkPluginContext* eventBasedCommunicationSenderPlugin::getPluginContext() const
{
	return context;
}




void eventBasedCommunicationSenderPlugin::sendEventToReceiver(QString sendText, int sendActualLoopNumber, int sendNumberOfLoop)
{

	ctkServiceReference reference= context->getServiceReference<ctkEventAdmin>();


    if(reference)
    {

      for(sendActualLoopNumber= 0; sendActualLoopNumber<= sendNumberOfLoop; sendActualLoopNumber++)
      {
    	  qDebug()<<"Send Event To Receiver";

          ctkEventAdmin* eventAdmin= context->getService<ctkEventAdmin>(reference);

          ctkDictionary properties;
          properties["sendText"]= sendText;
          properties["sendActualLoopNumber"]= sendActualLoopNumber;
          properties["sendNumberOfLoop"]= sendNumberOfLoop;


    	  properties[sendText + QString::number(sendActualLoopNumber) + QString::number(sendNumberOfLoop)];

    	  qDebug()<<sendText + QString::number(sendActualLoopNumber);

          ctkEvent reportGeneratedEvent("SendEventUpdated", properties);

          eventAdmin->sendEvent(reportGeneratedEvent);
      }



      //eventAdmin->postEvent(reportGeneratedEvent);

    }
  }




//Q_EXPORT_PLUGIN2(at_voxelmaster_eventBasedCommunicationSenderPlugin, eventBasedCommunicationSenderPlugin)


