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



#include "serviceBasedCommunicationReceiverPlugin.h"
#include "serviceTracker.h"


#include <iostream>

#include <QtPlugin>

#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>


serviceBasedCommunicationReceiverPlugin* serviceBasedCommunicationReceiverPlugin::instance = 0;

serviceBasedCommunicationReceiverPlugin::serviceBasedCommunicationReceiverPlugin()
  : context(0)
{

	std::cout << "serviceBasedCommunicationReceiverReceiverPlugin constructor" << std::endl;

}

serviceBasedCommunicationReceiverPlugin::~serviceBasedCommunicationReceiverPlugin()
{

}

void serviceBasedCommunicationReceiverPlugin::start(ctkPluginContext* context)
{

	instance = this;
	this->context = context;


	this->initServiceTracker();

	std::cout << "serviceBasedCommunicationReceiverReceiverPlugin start" << std::endl;
}

void serviceBasedCommunicationReceiverPlugin::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)
}

serviceBasedCommunicationReceiverPlugin* serviceBasedCommunicationReceiverPlugin::getInstance()
{
	return instance;
}

ctkPluginContext* serviceBasedCommunicationReceiverPlugin::getPluginContext() const
{
	return context;
}



void serviceBasedCommunicationReceiverPlugin::initServiceTracker()
{

	serviceTracker* sTracker = new serviceTracker(context,this);

	ctkServiceTracker<serviceInterface*,serviceInterface*>* tracker = new ctkServiceTracker<serviceInterface*,serviceInterface*>(context,sTracker);
	tracker->open();


}


void serviceBasedCommunicationReceiverPlugin::receiveServiceFromSender(QString sendText, int sendActualLoopNumber, int sendNumberOfLoop)
{
	qDebug()<< "Receive Service From Sender \n"<< sendText << sendActualLoopNumber; //<< sendNumberOfLoop;

}


//Q_EXPORT_PLUGIN2(at_voxelmaster_serviceBasedCommunicationReceiverPlugin, serviceBasedCommunicationReceiverPlugin)


