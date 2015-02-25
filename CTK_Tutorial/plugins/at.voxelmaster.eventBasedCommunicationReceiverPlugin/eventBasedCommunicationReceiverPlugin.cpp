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



#include "eventBasedCommunicationReceiverPlugin.h"

#include <iostream>

#include <QtPlugin>

#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>


eventBasedCommunicationReceiverPlugin* eventBasedCommunicationReceiverPlugin::instance = 0;

eventBasedCommunicationReceiverPlugin::eventBasedCommunicationReceiverPlugin()
  : context(0)
{

	std::cout << "eventBasedCommunicationReceiverReceiverPlugin constructor" << std::endl;

}

eventBasedCommunicationReceiverPlugin::~eventBasedCommunicationReceiverPlugin()
{

}

void eventBasedCommunicationReceiverPlugin::start(ctkPluginContext* context)
{

	instance = this;
	this->context = context;

	ctkDictionary props;
	props[ctkEventConstants::EVENT_TOPIC] = "SendEventUpdated";

	context->registerService<ctkEventHandler>(instance, props);


	std::cout << "eventBasedCommunicationReceiverReceiverPlugin start" << std::endl;
}

void eventBasedCommunicationReceiverPlugin::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)
}

eventBasedCommunicationReceiverPlugin* eventBasedCommunicationReceiverPlugin::getInstance()
{
	return instance;
}

ctkPluginContext* eventBasedCommunicationReceiverPlugin::getPluginContext() const
{
	return context;
}


void eventBasedCommunicationReceiverPlugin::handleEvent(const ctkEvent& event)
{

	qDebug()<<"Receive Event From Sender";

	receiverText= event.getProperty("sendText").toString();
	receiverActualLoopNumber = event.getProperty("sendActualLoopNumber").toInt();
	//receiverNumberOfLoop=  event.getProperty("sendNumberOfLoop").toInt();


		qDebug()<<receiverText<< receiverActualLoopNumber;



}



//Q_EXPORT_PLUGIN2(at_voxelmaster_eventBasedCommunicationReceiverPlugin, eventBasedCommunicationReceiverPlugin)


