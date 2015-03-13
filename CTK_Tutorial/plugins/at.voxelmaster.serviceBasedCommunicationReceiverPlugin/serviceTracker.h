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


#ifndef serviceTracker_H_
#define serviceTracker_H_

#include "serviceBasedCommunicationReceiverPlugin.h"

#include <iostream>

#include <QtPlugin>
#include <QObject>
#include <QString>

#include <ctkPluginActivator.h>


#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>
#include <ctkServiceEvent.h>
#include <ctkServiceTrackerCustomizer.h>
#include <ctkServiceReference.h>



class serviceTracker :public ctkServiceTrackerCustomizer<serviceInterface*>
{



public:

	serviceTracker(ctkPluginContext* context,serviceBasedCommunicationReceiverPlugin* serviceBasedCommunicationReceiverPluginObject)
	{
		this->context= context;
		this->serviceBasedCommunicationReceiverPluginObject= serviceBasedCommunicationReceiverPluginObject;
	}


private:

	ctkPluginContext* context;
	serviceBasedCommunicationReceiverPlugin *serviceBasedCommunicationReceiverPluginObject;

	serviceInterface* addingService (const ctkServiceReference &reference)
	{
		if(reference.getProperty("serviceEventID")!= QVariant::Invalid && reference.getProperty("serviceEventID").toInt(new bool())> 0)
		{
			modifiedService(reference,context->getService<serviceInterface>(reference));
		}

		return context->getService<serviceInterface>(reference);
	}

	void modifiedService (const ctkServiceReference &reference, serviceInterface* service)
	{
		serviceBasedCommunicationReceiverPluginObject->receiveServiceFromSender(service->sendText, service->sendActualLoopNumber, service->sendNumberOfLoop);
		QStringList propKeys = reference.getPropertyKeys();
		for(QStringList::iterator it = propKeys.begin(); it != propKeys.end(); ++it)
		{
			QString key = *it;
			QVariant prop = reference.getProperty(key);
			qDebug() << key << " - " << prop;
		}
//		qDebug() << "Service reference: " << serviceInterface->sendActualLoopNumber << " - " << serviceInterface->sendText;


	}


	void removedService (const ctkServiceReference &reference, serviceInterface* service)
	{
	}

};



#endif /* serviceTracker_H_ */

