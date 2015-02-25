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


#ifndef microscopeCommunicationPlugin_H
#define microscopeCommunicationPlugin_H


#include "microscopeVideoInterface.h"
#include "BGAPIVideoBuffer.h"
#include <ctkPluginActivator.h>





class microscopeCommunicationPlugin : public QObject, public ctkPluginActivator
{
	Q_OBJECT
	Q_INTERFACES(ctkPluginActivator)


public:

	microscopeCommunicationPlugin();
	~microscopeCommunicationPlugin();

	void start(ctkPluginContext* context);
	void stop(ctkPluginContext* context);

	static microscopeCommunicationPlugin* getInstance();

	ctkPluginContext* getPluginContext() const;
	void startCameraInitialization();

	public slots:
		void startMicroscopeVideoService(vector<QAbstractVideoBuffer*> cameraBuffers);

private:

	static microscopeCommunicationPlugin			*instance;
	ctkPluginContext								*context;

	microscopeVideoInterface						*microscopeVideoInterfaceInstance;

	ctkServiceRegistration 							microscopeVideoServiceReference;
	QThread											*threadInstance;

}; // microscopeCommunicationPlugin
/*
class InitializeBGAPIThread : public QThread{
	Q_OBJECT

	public:
			InitializeBGAPIThread(){


				connect(this, SIGNAL(startMicroscopeVideoService(vector<QAbstractVideoBuffer*>)),
						microscopeCommunicationPlugin::getInstance(), SLOT(startMicroscopeVideoService(vector<QAbstractVideoBuffer*>)));
			}


	private:



	 void run(){
	 		std::cout<<"Thread RUN InitializeBGAPIThread.h"<<std::endl;

	 		emit startMicroscopeVideoService( BGAPIVideoBuffer::createQAbstractVideoBuffers());
	 		//microscopeCommunicationPlugin::getInstance()->startMicroscopeVideoService( BGAPIVideoBuffer::createQAbstractVideoBuffers());
	 }

	 signals:
	 	 void startMicroscopeVideoService(vector<QAbstractVideoBuffer*> videoBuffers);

};
*/
#endif // microscopeCommunicationPlugin_H
