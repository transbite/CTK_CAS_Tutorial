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

#include <unistd.h>
#include <iostream>


#include <QCoreApplication>
#include <QApplication>
#include <QDebug>

#include <ctkPluginFrameworkLauncher.h>
#include <ctkPluginException.h>
#include <ctkPluginContext.h>

#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>
#include <service/event/ctkEventHandler.h>
#include <service/event/ctkEventConstants.h>



bool startPlugin(QString pluginName)
{
	bool SUCCEEDED= false;

	try
	{
		SUCCEEDED = ctkPluginFrameworkLauncher::start(pluginName);
	}

	catch(ctkPluginException &e)
	{
		std::cout << "Error in " <<pluginName.toStdString() << " "<< e.message().toStdString() << std::endl;
		const ctkException* e2 = e.cause();

		if(e2)
			std::cout << e2->message().toStdString() << std::endl;

		return false;
	}

	catch(ctkRuntimeException &e)
	{
		std::cout << "Error in " <<pluginName.toStdString() << " "<< e.what() << std::endl;
		const ctkException* e2 = e.cause();

		if(e2)
			std::cout << e2->message().toStdString() << std::endl;

		return false;
	}


	return SUCCEEDED;
}


int main(int argc, char** argv)
{


	QApplication app(argc, argv);

	app.setOrganizationName("Voxelmaster");
	app.setOrganizationDomain("voxelmaster.at");
	app.setApplicationName("CTK Tutorial");


	ctkPluginFrameworkLauncher::addSearchPath("plugins",true);
    ctkPluginFrameworkLauncher::start();

    if(startPlugin("org.commontk.eventadmin"))
	{
		std::cout << "Event Admin loading -> SUCCEEDED" << std::endl;
	}
	else
	{
		std::cout << "Event Admin loading-> FAILED" << std::endl;
	}


    if(startPlugin("at.voxelmaster.emptyPlugin"))
	{

		std::cout << "Empty plugin loading-> SUCCEEDED" << std::endl;
	}
	else
	{
		std::cout << "Empty plugin loading-> FAILED" << std::endl;
	}



    if(startPlugin("at.voxelmaster.eventBasedCommunicationReceiverPlugin"))
	{
		std::cout << "Event based communication receiver plugin loading-> SUCCEEDED" << std::endl;
	}
	else
	{
		std::cout << "Event based communication receiver plugin loading-> FAILED" << std::endl;
	}



    if(startPlugin("at.voxelmaster.eventBasedCommunicationSenderPlugin"))
	{
		std::cout << "Event based communication sender plugin loading-> SUCCEEDED" << std::endl;
	}
	else
	{
		std::cout << "Event based communication sender plugin loading-> FAILED" << std::endl;
	}




    if(startPlugin("at.voxelmaster.serviceBasedCommunicationReceiverPlugin"))
	{

		std::cout << "Service based communication receiver plugin loading-> SUCCEEDED" << std::endl;
	}
	else
	{
		std::cout << "Service based communication receiver plugin loading-> FAILED" << std::endl;
	}



    if(startPlugin("at.voxelmaster.serviceBasedCommunicationSenderPlugin"))
	{
		std::cout << "Service based communication sender plugin loading-> SUCCEEDED" << std::endl;
	}
	else
	{
		std::cout << "Service based communication sender plugin loading-> FAILED" << std::endl;
	}




	return app.exec();
}





