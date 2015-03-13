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



#include "serviceBasedCommunicationSenderPlugin.h"

#include <iostream>

#include <QtPlugin>


#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>




serviceBasedCommunicationSenderPlugin* serviceBasedCommunicationSenderPlugin::instance = 0;

serviceBasedCommunicationSenderPlugin::serviceBasedCommunicationSenderPlugin()
  : context(0)
{

	std::cout << "serviceBasedCommunicationSenderPlugin constructor" << std::endl;

}

serviceBasedCommunicationSenderPlugin::~serviceBasedCommunicationSenderPlugin()
{
  
}

void serviceBasedCommunicationSenderPlugin::start(ctkPluginContext* context)
{
	instance = this;
	this->context = context;

    interfaceInstance = new serviceInterface();

    sendServiceToReceiver();

	std::cout << "serviceBasedCommunicationSenderPlugin start" << std::endl;
}

void serviceBasedCommunicationSenderPlugin::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)
}

serviceBasedCommunicationSenderPlugin* serviceBasedCommunicationSenderPlugin::getInstance()
{
	return instance;
}

ctkPluginContext* serviceBasedCommunicationSenderPlugin::getPluginContext() const
{
	return context;
}



void serviceBasedCommunicationSenderPlugin::sendServiceToReceiver()
{

	ctkDictionary properties;
    properties["serviceEventID"]=0;

////	interfaceInstance = new serviceInterface();

	interfaceInstance->sendText= "Send Service Text ";
	interfaceInstance->sendActualLoopNumber=0;
	interfaceInstance->sendNumberOfLoop=10;

    serviceRegistrationReference = context->registerService<serviceInterface>(interfaceInstance, properties);

    for(interfaceInstance->sendActualLoopNumber = 0; interfaceInstance->sendActualLoopNumber <= interfaceInstance->sendNumberOfLoop;
        interfaceInstance->sendActualLoopNumber++)
    {
		qDebug()<<"Send Service To Receiver";

		qDebug()<<interfaceInstance->sendText << interfaceInstance->sendActualLoopNumber;

        //serviceRegistrationReference = context->registerService<serviceInterface>(interfaceInstance, properties);
        serviceRegistrationReference = context->getServiceReference<serviceInterface>();
        properties["serviceEventID"] = 15 + interfaceInstance->sendActualLoopNumber;// An ID pro event
		serviceRegistrationReference.setProperties(properties);
    }

}


//Q_EXPORT_PLUGIN2(at_voxelmaster_serviceBasedCommunicationSenderPlugin, serviceBasedCommunicationSenderPlugin)

