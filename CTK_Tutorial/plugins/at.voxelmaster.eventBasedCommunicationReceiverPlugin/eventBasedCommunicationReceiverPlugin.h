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



#ifndef eventBasedCommunicationReceiverPlugin_H
#define eventBasedCommunicationReceiverPlugin_H

#include <ctkPluginActivator.h>


#include <ctkServiceTracker.h>
#include <service/event/ctkEventHandler.h>
#include <service/event/ctkEvent.h>



class eventBasedCommunicationReceiverPlugin : public QObject, public ctkEventHandler, public ctkPluginActivator
{
	Q_OBJECT
    Q_PLUGIN_METADATA(IID "at.voxelmaster.eventBasedCommunicationReceiverPlugin")
	Q_INTERFACES(ctkEventHandler ctkPluginActivator)



public:

	eventBasedCommunicationReceiverPlugin();
	~eventBasedCommunicationReceiverPlugin();

	void start(ctkPluginContext* context);
	void stop(ctkPluginContext* context);

	static eventBasedCommunicationReceiverPlugin* getInstance();

	ctkPluginContext* getPluginContext() const;

	void handleEvent(const ctkEvent& event);

private:


	static eventBasedCommunicationReceiverPlugin* instance;
	ctkPluginContext* context;

	QString receiverText;
	int receiverActualLoopNumber;
	int receiverNumberOfLoop;



}; // eventBasedCommunicationReceiverPlugin

#endif // eventBasedCommunicationReceiverPlugin_H
