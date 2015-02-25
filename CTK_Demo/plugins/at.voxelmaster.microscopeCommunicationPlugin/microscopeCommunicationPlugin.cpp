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


#include "microscopeCommunicationPlugin.h"

#include <iostream>
#include <fstream>

#include <QObject>
#include <QString>

#include <QtGui>


#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>
#include <unistd.h>

microscopeCommunicationPlugin* microscopeCommunicationPlugin::instance = 0;

microscopeCommunicationPlugin::microscopeCommunicationPlugin()
  : context(0)
{
	std::cout << "Microscope Communication plugin constructor" << std::endl;
}



microscopeCommunicationPlugin::~microscopeCommunicationPlugin()
{
  
}



void microscopeCommunicationPlugin::start(ctkPluginContext* context)
{
	std::cout << "Microscope Communication plugin start" << std::endl;

	qRegisterMetaType<vector<QAbstractVideoBuffer*> >("vector<QAbstractVideoBuffer*>");

	instance = this;
	this->context = context;

	startCameraInitialization();

}

void microscopeCommunicationPlugin::startCameraInitialization(){
/*threadInstance = new InitializeBGAPIThread();
	threadInstance->start();
	*/
	microscopeCommunicationPlugin::getInstance()->startMicroscopeVideoService( BGAPIVideoBuffer::createQAbstractVideoBuffers());

}


void microscopeCommunicationPlugin::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)
}



microscopeCommunicationPlugin* microscopeCommunicationPlugin::getInstance()
{
	return instance;
}



ctkPluginContext* microscopeCommunicationPlugin::getPluginContext() const
{
	std::cout << "Microscope Communication context" << std::endl;
	return context;
}

void microscopeCommunicationPlugin::startMicroscopeVideoService(vector<QAbstractVideoBuffer*> cameraBuffers)
{
	ctkDictionary properties;
	properties["microscopeVideoServiceEventID"]=0;
	std::cout << "start microscope Video service..." << std::endl;
	microscopeVideoInterfaceInstance = new microscopeVideoInterface();



	microscopeVideoInterfaceInstance->pixelFormat = QVideoFrame::Format_RGB24;
	microscopeVideoInterfaceInstance->size = new QSize(776,578);

	microscopeVideoInterfaceInstance->microscopeVideoInterfaceBuffers=cameraBuffers;

	microscopeVideoServiceReference = context->registerService<microscopeVideoInterface>(microscopeVideoInterfaceInstance, properties);



	properties["microscopeVideoServiceEventID"]=1;
	microscopeVideoServiceReference.setProperties(properties);

	std::cout << "microscope Video service started!" << std::endl;

}


Q_EXPORT_PLUGIN2(at_voxelmaster_microscopeCommunicationPlugin, microscopeCommunicationPlugin)


