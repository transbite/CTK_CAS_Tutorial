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



#ifndef serviceBasedCommunicationReceiverPlugin_H
#define serviceBasedCommunicationReceiverPlugin_H


#include "serviceInterface.h"

#include <ctkPluginActivator.h>

#include <ctkServiceTracker.h>
#include <service/event/ctkEventHandler.h>
#include <service/event/ctkEvent.h>



class serviceBasedCommunicationReceiverPlugin : public QObject, public ctkPluginActivator
{
	Q_OBJECT
    Q_PLUGIN_METADATA(IID "at.voxelmaster.serviceBasedCommunicationReceiverPlugin")
	Q_INTERFACES(ctkPluginActivator)



public:

	serviceBasedCommunicationReceiverPlugin();
	~serviceBasedCommunicationReceiverPlugin();

	void start(ctkPluginContext* context);
	void stop(ctkPluginContext* context);

	static serviceBasedCommunicationReceiverPlugin* getInstance();

	ctkPluginContext* getPluginContext() const;

	void receiveServiceFromSender(QString, int, int);

private:


	static serviceBasedCommunicationReceiverPlugin* instance;
	ctkPluginContext* context;

	void initServiceTracker();


}; // serviceBasedCommunicationReceiverPlugin

#endif // serviceBasedCommunicationReceiverPlugin_H
