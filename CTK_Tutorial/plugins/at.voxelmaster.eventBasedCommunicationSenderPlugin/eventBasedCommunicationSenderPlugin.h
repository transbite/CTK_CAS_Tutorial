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



#ifndef eventBasedCommunicationSenderPlugin_H
#define eventBasedCommunicationSenderPlugin_H

#include <ctkPluginActivator.h>


#include <QString>
#include <QLibrary>


#include <ctkServiceTracker.h>
#include <service/event/ctkEventHandler.h>
#include <service/event/ctkEvent.h>
#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>



class eventBasedCommunicationSenderPlugin :  public QObject, public ctkPluginActivator
{
	Q_OBJECT
    Q_PLUGIN_METADATA(IID "at.voxelmaster.eventBasedCommunicationSenderPlugin")
	Q_INTERFACES(ctkPluginActivator)



public:

	eventBasedCommunicationSenderPlugin();
	~eventBasedCommunicationSenderPlugin();

	void start(ctkPluginContext* context);
	void stop(ctkPluginContext* context);

	static eventBasedCommunicationSenderPlugin* getInstance();

	ctkPluginContext* getPluginContext() const;

	void sendEventToReceiver(QString, int, int);



private:

	static eventBasedCommunicationSenderPlugin* instance;
	ctkPluginContext* context;


	QString sendText;
	int sendActualLoopNumber;
	int sendNumberOfLoop;


}; // eventBasedCommunicationSenderPlugin

#endif // eventBasedCommunicationSenderPlugin_H
