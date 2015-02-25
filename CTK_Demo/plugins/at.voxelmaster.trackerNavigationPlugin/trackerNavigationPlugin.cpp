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

#include "trackerNavigationPlugin.h"

#include <iostream>
#include <fstream>

#include <QtPlugin>

#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>
#include "dicomDataServiceTracker.h"

#define VIEW_2D_REFRESH_RATE 15
#define VIEW_3D_REFRESH_RATE 15



trackerNavigationPlugin* trackerNavigationPlugin::instance = 0;

trackerNavigationPlugin::trackerNavigationPlugin()
  : context(0)
{
	std::cout << "Tracker Navigation  plugin constructor" << std::endl;
}

trackerNavigationPlugin::~trackerNavigationPlugin()
{
  
}

void trackerNavigationPlugin::start(ctkPluginContext* context)
{
	std::cout << "Tracker Navigation plugin start" << std::endl;

	instance = this;
	this->context = context;

	QTimer *pulseTimer = new QTimer();
	connect(pulseTimer, SIGNAL(timeout()), this, SLOT(pulseTimerEvent()));
	pulseTimer->start(25);

	isRegistered=false;
	toolPosition = new RegistrationType::LandmarkTrackerPointType();
	toolRotation = new RegistrationType::LandmarkTrackerPointType();

	sys_time_t t;
	system_time(&t);
	lastGUIRefreshing=time_to_msec(t);


	std::srand(5);

	m_maxFiducials= 0;
	m_FiducialSize = 5;
	m_maxFiducialNumber= 25;
	m_ActualFiducialNumber= 0;


	m_maxFiducialsTracker= 0;
	m_FiducialSizeTracker = 5;
	m_maxFiducialNumberTracker= 25;
	m_ActualFiducialNumberTracker= 0;


	m_TrackerRMS = 0.0;

	m_firstRow= 0;
	m_secondRow= 0;
	m_thirdRow= 0;
	m_fourthRow= 0;
	m_fifthRow= 0;

	m_firstColumn= 0;
	m_secondColumn= 1;
	m_thirdColumn= 2;
	m_fourthColumn= 3;
	m_fifthColumn= 4;
	m_sixthColumn= 5;


	m_firstRowTracker= 0;
	m_secondRowTracker= 0;
	m_thirdRowTracker= 0;
	m_fourthRowTracker= 0;
	m_fifthRowTracker= 0;

	m_firstColumnTracker= 0;
	m_secondColumnTracker= 1;
	m_thirdColumnTracker= 2;
	m_fourthColumnTracker= 3;
	m_fifthColumnTracker= 4;
	m_sixthColumnTracker= 5;


	m_ImageLoaded= false;
	m_ModifyImageFiducialsEnabled= false;
	m_ModifyTrackerFiducialsEnabled= false;
	m_NavigationEnabled = false;
	m_TrackingEnabled = false;

	m_CTImageDirectory = "";

	m_DefaultZoomImageAxial= 1.5;
	m_DefaultZoomImageCoronal= 1.5;
	m_DefaultZoomImageSagittal= 1.5;
	m_DefaultZoomImageMPR= 1.5;

	m_WindowLevelImage= 500;
	m_WindowWidthImage= 300;

	m_MinWindowLevelValue= -1000;
	m_MaxWindowLevelValue= 4000;

	m_MinWindowWidhtValue= 0;
	m_MaxWindowWidhtValue= m_MaxWindowLevelValue - m_MinWindowLevelValue;

	//Qt styles
	m_BorderGreenStyle = "border: 2px solid green;";
	m_BorderRedStyle = "border: 2px solid red;";
	m_BorderGreyStyle = "border: 10px solid grey;";
	m_BorderYellowStyle = "border: 10px solid yellow;";
	m_BackgroundWhiteStyle = "background-color:rgb(255, 255, 255)";
	m_BackgroundBlueStyle = "background-color: rgb(10, 128, 255)";
	m_BackgroundGreenStyle = "background-color:rgb(85, 255, 0)";
	m_BackgroundYellowStyle = "background-color: rgb(252, 255, 47)";


	// Setup logger, for all igstk components.
	m_Logger   = LoggerType::New();
	this->GetLogger()->SetTimeStampFormat( itk::LoggerBase::HUMANREADABLE );
	this->GetLogger()->SetHumanReadableFormat("%Y %b %d, %H:%M:%S");
	this->GetLogger()->SetPriorityLevel( LoggerType::DEBUG);

	// Direct the application log message to the std::cout
	itk::StdStreamLogOutput::Pointer m_LogCoutOutput = itk::StdStreamLogOutput::New();
	m_LogCoutOutput->SetStream( std::cout );
	this->GetLogger()->AddLogOutput( m_LogCoutOutput );

	// Direct the igstk components log message to the file.
	itk::StdStreamLogOutput::Pointer m_LogFileOutput= itk::StdStreamLogOutput::New();


	//Return if fail to open the log file
	igstkLogMacro2(m_Logger, DEBUG,"Logger ist startet:" << "\n" );

	/*
	  std::string   logFileName;
	  logFileName = "logCTK_Demo" + itksys::SystemTools::GetCurrentDateTime( "_%Y_%m_%d_%H_%M_%S" ) + ".txt";
	  m_LogFile.open( logFileName.c_str() );

	  if( m_LogFile.fail() )
	  {
	    //Return if fail to open the log file
	    igstkLogMacro2( m_Logger, DEBUG,  "Problem opening Log file:" << logFileName << "\n" );
	    return;
	  }

	  m_LogFileOutput->SetStream( m_LogFile );
	  this->GetLogger()->AddLogOutput( m_LogFileOutput );
	  igstkLogMacro2( m_Logger, DEBUG,  "Successfully opened Log file:" << logFileName << "\n" );
	 */



	//m_TrackerToolUpdateObserver= LoadedObserverType::New();
	//m_TrackerToolUpdateObserver->SetCallbackFunction( this, &trackerNavigationPlugin::TrackerToolUpdate);

	m_WorldReference = igstk::AxesObject::New();


	igstk::Transform transform;
	transform.SetToIdentity( 1e300 );

	m_PickedPoint= EllipsoidType::New();
	m_PickedPoint->RequestSetTransformAndParent( transform , this->m_ViewMPR);

	m_PickedPointRepresentation= EllipsoidRepresentationType::New();
	//m_PickedPointRepresentation->SetLogger(logger);
	m_PickedPoint->SetRadius( 2, 2, 2 );
	m_PickedPointRepresentation->RequestSetEllipsoidObject( m_PickedPoint );
	m_PickedPointRepresentation->SetColor(1.0,1.0,0.0);
	m_PickedPointRepresentation->SetOpacity(1.0);


	m_TrackerPoint = EllipsoidType::New();
	m_TrackerPoint->RequestSetTransformAndParent( transform , this->m_ViewMPR);

	m_TrackerPointPointRepresentation= EllipsoidRepresentationType::New();
	//m_TrackerPointPointRepresentation->SetLogger(logger);
	m_TrackerPoint->SetRadius(2, 2, 2);
	m_TrackerPointPointRepresentation->RequestSetEllipsoidObject( m_TrackerPoint );
	m_TrackerPointPointRepresentation->SetColor(1.0,1.0,0.0);
	m_TrackerPointPointRepresentation->SetOpacity(1.0);


	this->createWidgetsAndSignalSlotActions();
	this->initImageDataTracker();

	this->initializeQuadrantView();



	//this->readCTDataset();



	ctkDictionary props;
	props[ctkEventConstants::EVENT_TOPIC] = "TrackingCoordinatesUpdated";

	context->registerService<ctkEventHandler>(instance, props);

	connect(m_toolFrameTrackerNavigationGUI.addImageFiducialsPushButton, SIGNAL(clicked()), this, SLOT(addImageFiducials()));
	connect(m_toolFrameTrackerNavigationGUI.modifyFiducialsPushButton, SIGNAL(clicked()), this, SLOT(setImageFiducialProcessing()));
	connect(m_toolFrameTrackerNavigationGUI.saveFiducialsPushButton, SIGNAL(clicked()), this, SLOT(saveFiducialsInFile()));
	connect(m_toolFrameTrackerNavigationGUI.readFiducialsPushButton, SIGNAL(clicked()), this, SLOT(readFiducialsFromFile()));
	connect(m_toolFrameTrackerNavigationGUI.addTrackerFiducialsPushButton, SIGNAL(clicked()), this, SLOT(addTrackerFiducialsProcessing()));
	connect(m_toolFrameTrackerNavigationGUI.modifyTrackerFiducialsPushButton, SIGNAL(clicked()), this, SLOT(setTrackerFiducialProcessing()));
	connect(m_toolFrameTrackerNavigationGUI.startRegistrationNavigationPushButton, SIGNAL(clicked()), this, SLOT(startRegistrationNavigationProcessing()));
	connect(m_toolFrameTrackerNavigationGUI.finishRegistrationNavigationPushButton, SIGNAL(clicked()), this, SLOT(finishRegistrationNavigationProcessing()));
	connect(m_toolFrameTrackerNavigationGUI.clearTrackerFiducialsPushButton, SIGNAL(clicked()), this, SLOT(deleteAllTrackerFiducials()));
	connect(m_toolFrameTrackerNavigationGUI.finishImageAndTrackerFiducialsPushButton, SIGNAL(clicked()), this, SLOT(finishImageAndTrackerFiducialsProcessing()));
	connect(m_toolFrameTrackerNavigationGUI.clearFiducialsPushButton, SIGNAL(clicked()), this, SLOT(newFiducialPlan()));


	connect(m_mainFrameNavigationViewGUI.scrollBarAxial, SIGNAL(valueChanged(int)), this, SLOT(resliceImageViews(int)));
	connect(m_mainFrameNavigationViewGUI.scrollBarSagittal, SIGNAL(valueChanged(int)), this, SLOT(resliceImageViews(int)));
	connect(m_mainFrameNavigationViewGUI.scrollBarCoronal, SIGNAL(valueChanged(int)), this, SLOT(resliceImageViews(int)));



	m_toolFrameTrackerNavigationGUI.startRegistrationNavigationPushButton->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.finishRegistrationNavigationPushButton->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.modifyTrackerFiducialsPushButton->setVisible(false);


}

void trackerNavigationPlugin::pulseTimerEvent()
{
	igstk::PulseGenerator::CheckTimeouts();
}

void trackerNavigationPlugin::handleEvent(const ctkEvent& event){


	int markerAmount = event.getProperty("markerAmount").toInt(new bool());
	long long eventTimestamp = event.getProperty("timestamp").toLongLong(new bool());
	sys_time_t t;
	system_time(&t);
	long long dif=(time_to_msec(t)- eventTimestamp);

	if(dif>5)
	{
		std::cout << "Navigation event lag: " << QVariant(dif).toString().toStdString()  << std::endl;
	}

	m_MissingText= "MISSING";

	for(int i=1;i<=markerAmount;i++)
	{
		if(event.getProperty("xCoordMarker"+QString::number(i)).toFloat(new bool())<=(float)-3.0E28 ||
		   event.getProperty("yCoordMarker"+QString::number(i)).toFloat(new bool())<=(float)-3.0E28 ||
		   event.getProperty("zCoordMarker"+QString::number(i)).toFloat(new bool())<=(float)-3.0E28)
		{

		//std::cout << "Navigation event (Marker"<<QString::number(i).toStdString() << "): Missing"<< std::endl;

		m_toolFrameTrackerNavigationGUI.trackerPointXQLabel->setText(m_MissingText);
		m_toolFrameTrackerNavigationGUI.trackerPointYQLabel->setText(m_MissingText);
		m_toolFrameTrackerNavigationGUI.trackerPointZQLabel->setText(m_MissingText);
		//mutex.lock();
		//m_toolFrameTrackerNavigationGUI.trackerPointXQLabel->setStyleSheet(m_BorderRedStyle);
		//m_toolFrameTrackerNavigationGUI.trackerPointYQLabel->setStyleSheet(m_BorderRedStyle);
		//m_toolFrameTrackerNavigationGUI.trackerPointZQLabel->setStyleSheet(m_BorderRedStyle);
		//mutex.unlock();
		}
		else
		{
			/*
			 std::cout << "Navigation event (Marker"<<QString::number(i).toStdString() << ") "
					   " x:"<< event.getProperty("xCoordMarker"+QString::number(i)).toString().toStdString()
					<< " y:"<< event.getProperty("yCoordMarker"+QString::number(i)).toString().toStdString()
					<< " z:"<< event.getProperty("zCoordMarker"+QString::number(i)).toString().toStdString()
					<< std::endl;
			*/

			m_XCoordiante= event.getProperty("xCoordMarker"+QString::number(i)).toString();
			m_YCoordiante= event.getProperty("yCoordMarker"+QString::number(i)).toString();
			m_ZCoordiante= event.getProperty("zCoordMarker"+QString::number(i)).toString();

			m_toolFrameTrackerNavigationGUI.trackerPointXQLabel->setText(m_XCoordiante);
			m_toolFrameTrackerNavigationGUI.trackerPointYQLabel->setText(m_YCoordiante);
			m_toolFrameTrackerNavigationGUI.trackerPointZQLabel->setText(m_ZCoordiante);
			//mutex.lock();
			//m_toolFrameTrackerNavigationGUI.trackerPointXQLabel->setStyleSheet(m_BorderGreenStyle);
			//m_toolFrameTrackerNavigationGUI.trackerPointYQLabel->setStyleSheet(m_BorderGreenStyle);
			//m_toolFrameTrackerNavigationGUI.trackerPointZQLabel->setStyleSheet(m_BorderGreenStyle);
			//mutex.unlock();
			//toolPosition = new LandmarkTrackerPointType();

			//p[2]=event.getProperty("zCoordMarker"+QString::number(i)).toFloat(new bool());

			if(i==1)
			{
				mutex.lock();

				toolPosition->SetElement(0,event.getProperty("xCoordMarker"+QString::number(i)).toFloat(new bool()));
				toolPosition->SetElement(1,event.getProperty("yCoordMarker"+QString::number(i)).toFloat(new bool()));
				toolPosition->SetElement(2,event.getProperty("zCoordMarker"+QString::number(i)).toFloat(new bool()));



				toolRotation->SetElement(0,event.getProperty("xRotMarker"+QString::number(i)).toFloat(new bool()));
				toolRotation->SetElement(1,event.getProperty("yRotMarker"+QString::number(i)).toFloat(new bool()));
				toolRotation->SetElement(2,event.getProperty("zRotMarker"+QString::number(i)).toFloat(new bool()));



				if(isRegistered)
				{
					mutex.unlock();

					//system_time(&t);
					//if((time_to_msec(t)- lastGUIRefreshing)>(1000/VIEW_2D_REFRESH_RATE)){
					//	lastGUIRefreshing=time_to_msec(t);
					this->setCursorPosition();
					//}
				}
				else
				{
					mutex.unlock();
				}

			}


		}

	}

}




void trackerNavigationPlugin::setCursorPosition(){



			  mutex.lock();

			  igstk::Transform transform;

			  typedef igstk::Transform::VectorType TranslationType;
			  TranslationType translation;

			  translation[0] = toolPosition->GetElement(0);
			  translation[1] = toolPosition->GetElement(1);
			  translation[2] = toolPosition->GetElement(2);

			  mutex.unlock();

			  //typedef igstk::Transform::VersorType RotationType;
			  //RotationType rotation;

			  //rotation.Set(toolRotation->GetElement(0), toolRotation->GetElement(1), toolRotation->GetElement(2), 1.0);
			  //rotation.Set(0, 0, 0, 1.0);

			  igstk::TimeStamp renderTime;
			  renderTime.SetStartTimeNowAndExpireAfter( 1000.0 / VIEW_2D_REFRESH_RATE );

			  transform.SetTranslation(translation, 0.1,
					  renderTime.GetExpirationTime());
			  //transform.SetTranslationAndRotation(translation, rotation, 0.1,
				//	  renderTime.GetExpirationTime());





			  igstk::Transform  newTransform = igstk::Transform::TransformCompose(m_RegistrationTransform,transform);
/*
			  std::cout << "Navigation event (Marker"<< ") "
					  				   " x:"<< QString::number(translation[0]).toStdString() << " - " << QString::number(newTransform.GetTranslation()[0]).toStdString()
					  				<< " y:"<< QString::number(translation[1]).toStdString() << " - " << QString::number(newTransform.GetTranslation()[1]).toStdString()
					  				<< " z:"<< QString::number(translation[2]).toStdString() << " - " << QString::number(newTransform.GetTranslation()[2]).toStdString()
					  				<< std::endl;
*/

			  /*
			  itk::Point< double, 3 > ph;
			  ph = m_Plan->m_FiducialPoints[ rand() % 4 + 1];

*/

			 // m_FiducialPointVector[m_ActualFiducialNumber]->RequestSetTransformAndParent( PointToTransform(point), m_WorldReference );

			  itk::Point< double, 3 > ph;
					  ph[0] = newTransform.GetTranslation()[0];
					  ph[1] = newTransform.GetTranslation()[1];
					  ph[2] = newTransform.GetTranslation()[2];


					  /*
			  itk::Point< double, 3 > ph;
			  				  ph[0] = p[0];
			  				  ph[1] = p[1];
			  				  ph[2] = p[2];

 itk::Point< double, 3 > ph;
			  				ph = m_Plan->m_FiducialPoints[1];

*/
					  /*
			  				std::cout << "Navigation event ) " <<
			  								  				   " x:"<< QString::number(ph[0]).toStdString()
			  								  				<< " y:"<< QString::number(ph[1]).toStdString()
			  								  				<< " z:"<< QString::number(ph[2]).toStdString()
			  								  				<< std::endl;

*/

				if( m_ImageSpatialObject->IsInside( ph ) )	{


	  				//std::cout << "inside!!!!!!!!!!!!!!!!!!!!!!!!"<< std::endl;
					IndexType index;
					m_ImageSpatialObject->TransformPhysicalPointToIndex( ph, index);


					if(!(index[0]==crossHairIndex[0]&&index[1]==crossHairIndex[1]&&index[2]==crossHairIndex[2])){

						crossHairIndex[0]=index[0];
						crossHairIndex[1]=index[1];
						crossHairIndex[2]=index[2];


						/*
						igstk::PointEvent positionEvent;
						positionEvent.Set(ph);

						m_CrossHair->InvokeEvent( positionEvent );


						const double *data = ph.GetVnlVector().data_block();


						m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
						m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
						m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
						m_CrossHair->RequestSetCursorPosition( data );
*/



						resliceImage(index);

					}

					//resetImageCameraSettings();
					//refreshImageViews();
					/*
					m_mainFrameNavigationViewGUI.scrollBarAxial->setValue(index[2]);
									m_mainFrameNavigationViewGUI.scrollBarSagittal->setValue(index[0]);
									m_mainFrameNavigationViewGUI.scrollBarCoronal->setValue(index[1]);


					std::cout << "Navigation event (Marker"<<QString::number(i).toStdString() << ") "
																	  				   " x:"<< QString::number(index.m_Index[0]).toStdString()
																	  				<< " y:"<< QString::number(index.m_Index[1]).toStdString()
																	  				<< " z:"<< QString::number(index.m_Index[2]).toStdString()
																	  				<< std::endl;
	*/
			/*		m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
					m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
					m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
					*/
					//m_CrossHair->RequestSetCursorPosition( data );

	/*

					m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
						m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
						m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
						m_CrossHair->RequestSetCursorPosition( data );


					m_mainFrameNavigationViewGUI.navigationViewDisplayAxial->repaint();
					m_mainFrameNavigationViewGUI.navigationViewDisplaySagittal->repaint();
					m_mainFrameNavigationViewGUI.navigationViewDisplayCoronal->repaint();
					m_mainFrameNavigationViewGUI.navigationViewDisplayMPR->repaint();

					*/


	/*

						m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
							m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
							m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
							m_CrossHair->RequestSetCursorPosition( data );

							m_AxialPlaneSpatialObject->RequestComputeReslicingPlane(  );
													m_SagittalPlaneSpatialObject->RequestComputeReslicingPlane(  );
													m_CoronalPlaneSpatialObject->RequestComputeReslicingPlane(  );


					m_CrossHair->RequestSetCursorPosition( data );
					igstk::TimeStamp ts;
m_AxialCrossHairRepresentation->RequestUpdateRepresentation(ts,igstk::Friends::CoordinateSystemHelper::GetCoordinateSystem(m_CrossHair));

	*/
				}





}



void trackerNavigationPlugin::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)
}



trackerNavigationPlugin* trackerNavigationPlugin::getInstance()
{
	return instance;
}



ctkPluginContext* trackerNavigationPlugin::getPluginContext() const
{
	std::cout << "Tracker Navigation plugin context" << std::endl;
	
	return context;
}

void trackerNavigationPlugin::initImageDataTracker(){
	dicomDataServiceTracker* dst = new dicomDataServiceTracker(context,this);
	ctkServiceTracker<dicomDataInterface*,dicomDataInterface*>* tracker = new ctkServiceTracker<dicomDataInterface*,dicomDataInterface*>(context,dst);
	tracker->open();

}

void trackerNavigationPlugin::createWidgetsAndSignalSlotActions()
{
	trackerNavigationInterfaceInstance = new trackerNavigationInterface();

	toolFrameTrackerNavigationWidget = new QWidget();
	mainFrameVisualizationTrackerNavigationViewWidget = new QWidget();

	m_toolFrameTrackerNavigationGUI.setupUi(toolFrameTrackerNavigationWidget);
	m_mainFrameNavigationViewGUI.setupUi(mainFrameVisualizationTrackerNavigationViewWidget);

	trackerNavigationInterfaceInstance->toolFrameTrackerNavigationInterfaceWidget= toolFrameTrackerNavigationWidget;
	trackerNavigationInterfaceInstance->toolFrameTrackerNavigationWidgetString="2    Registration / Tracker";

	trackerNavigationInterfaceInstance->mainFrameVisualizationTrackerNavigationViewInterfaceWidget= mainFrameVisualizationTrackerNavigationViewWidget;
	trackerNavigationInterfaceInstance->mainFrameVisualizationTrackerNavigationViewInterfaceWidgetString="Tracker Navigation View";


	ctkDictionary properties;
	properties["message"] = "toolFrameTrackerNavigationWidget";

	context->registerService<trackerNavigationInterface>(trackerNavigationInterfaceInstance,properties);

	m_toolFrameTrackerNavigationGUI.pb1->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb2->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb3->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb4->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb5->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb6->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb11->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb16->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb21->setVisible(false);

	m_toolFrameTrackerNavigationGUI.pb_1->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb_2->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb_3->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb_4->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb_5->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb_6->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb_11->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb_16->setVisible(false);
	m_toolFrameTrackerNavigationGUI.pb_21->setVisible(false);


}



void trackerNavigationPlugin::addImageFiducials()
{

	igstkLogMacro2(m_Logger, DEBUG,	"addImageFiducials called\n");

	//Setup a QPushButton to produce it dynamically
	pushButton = new QPushButton();
	pushButton->setFixedSize(30, 30);

	QFont font = pushButton->font();
	font.setPointSize(8);
	pushButton->setFont(font);

	if(m_maxFiducials!=25)
	{
		if(m_firstRow< 6)
		{
			m_toolFrameTrackerNavigationGUI.gridLayout1->addWidget(pushButton, m_firstRow, m_firstColumn);
			m_firstRow++;
		}

		if(m_firstRow>= 6)
		{
			m_toolFrameTrackerNavigationGUI.gridLayout1->addWidget(pushButton, m_secondRow, m_secondColumn);
			m_secondRow++;
		}

		if(m_secondRow>= 6)
		{
			m_toolFrameTrackerNavigationGUI.gridLayout1->addWidget(pushButton, m_thirdRow, m_thirdColumn);
			m_thirdRow++;
		}

		if(m_thirdRow>= 6)
		{
			m_toolFrameTrackerNavigationGUI.gridLayout1->addWidget(pushButton, m_fourthRow, m_fourthColumn);
			m_fourthRow++;
		}

		if(m_fourthRow>=6 && m_fifthRow!=5)
		{
			m_toolFrameTrackerNavigationGUI.gridLayout1->addWidget(pushButton, m_fifthRow, m_fifthColumn);
			m_fifthRow++;
		}

	m_maxFiducials++;

	std::cout<<"m_maxFiducials: "<<m_maxFiducials<<std::endl;

	pushButton->setText(QString::number(m_maxFiducials));

	buttongroup = new QButtonGroup(this);
	buttongroup->addButton(pushButton);
	buttongroup->setId(pushButton, m_maxFiducials);
	}
	else
	{
		m_messageDialog->information(NULL, "Information", "You have reached maximum fiducial number! \nPlease click on finish button.",  QMessageBox::Ok);
	}

	connect(buttongroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(test(QAbstractButton*)));

	m_toolFrameTrackerNavigationGUI.modifyFiducialsPushButton->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.clearFiducialsPushButton->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.saveFiducialsPushButton->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.fiducialSetNameLineEdit->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.addTrackerFiducialsPushButton->setEnabled(true);


}



void trackerNavigationPlugin::test(QAbstractButton *pushButton)
{

	if(pushButton->text()== "1")
	{
		qDebug()<<"Clicked on Button 1, set number of fiducial to 1 and call other classes"<<pushButton->text()<< endl;
		m_ActualFiducialNumber= 0;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}

	if(pushButton->text()== "2")
	{
		qDebug()<<"Clicked on Button 2, set number of fiducial to 2 and call other classes"<<pushButton->text()<< endl;
		m_ActualFiducialNumber= 1;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}

	if(pushButton->text()== "3")
	{
		m_ActualFiducialNumber= 2;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}

	if(pushButton->text()== "4")
	{
		m_ActualFiducialNumber= 3;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "5")
	{
		m_ActualFiducialNumber= 4;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "6")
	{
		m_ActualFiducialNumber= 5;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "7")
	{
		m_ActualFiducialNumber= 6;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "8")
	{
		m_ActualFiducialNumber= 7;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "9")
	{
		m_ActualFiducialNumber= 8;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "10")
	{
		m_ActualFiducialNumber= 9;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "11")
	{
		m_ActualFiducialNumber= 10;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "12")
	{
		m_ActualFiducialNumber= 11;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "13")
	{
		m_ActualFiducialNumber= 12;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "14")
	{
		m_ActualFiducialNumber= 13;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "15")
	{
		m_ActualFiducialNumber= 14;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "16")
	{
		m_ActualFiducialNumber= 15;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "17")
	{
		m_ActualFiducialNumber= 16;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "18")
	{
		m_ActualFiducialNumber= 17;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "18")
	{
		m_ActualFiducialNumber= 18;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "20")
	{
		m_ActualFiducialNumber= 19;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "21")
	{
		m_ActualFiducialNumber= 20;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "22")
	{
		m_ActualFiducialNumber= 21;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "23")
	{
		m_ActualFiducialNumber= 22;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "24")
	{
		m_ActualFiducialNumber= 23;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


	if(pushButton->text()== "25")
	{
		m_ActualFiducialNumber= 24;
		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		requestChangeSelectedFiducial();
	}


}




void trackerNavigationPlugin::deleteAllImageFiducials()
{
	while(!m_toolFrameTrackerNavigationGUI.gridLayout1->isEmpty())
	{
		QWidget *gridLayoutWidget = m_toolFrameTrackerNavigationGUI.gridLayout1->takeAt(0)->widget();
	    delete gridLayoutWidget;
	}


	for (int i=0; i<m_maxFiducials; i++)
	{
		m_ViewImageAxial->RequestRemoveObject( m_AxialFiducialRepresentationVector[i] );
		m_ViewImageSagittal->RequestRemoveObject( m_SagittalFiducialRepresentationVector[i] );
		m_ViewImageCoronal->RequestRemoveObject( m_CoronalFiducialRepresentationVector[i] );
		m_ViewMPR->RequestRemoveObject( m_3DViewFiducialRepresentationVector[i] );
	}

	//m_Plan->m_FiducialPoints.clear();

	m_maxFiducials= 0;
	m_maxFiducialNumber= 0;
	m_ActualFiducialNumber= 0;

	m_firstRow= 0;
	m_secondRow= 0;
	m_thirdRow= 0;
	m_fourthRow= 0;
	m_fifthRow= 0;

	m_firstColumn= 0;
	m_secondColumn= 1;
	m_thirdColumn= 2;
	m_fourthColumn= 3;
	m_fifthColumn= 4;
	m_sixthColumn= 5;


	m_toolFrameTrackerNavigationGUI.modifyFiducialsPushButton->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.clearFiducialsPushButton->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.finishImageAndTrackerFiducialsPushButton->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.fiducialSetNameLineEdit->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.saveFiducialsPushButton->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.addImageFiducialsPushButton->setEnabled(true);

	//m_toolFrameTrackerNavigationGUI.addTrackerFiducialsPushButton->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.startRegistrationNavigationPushButton->setEnabled(false);



	refreshImageViews();
	resetImageCameraSettings();

}







void trackerNavigationPlugin::addTrackerFiducials()
{

	igstkLogMacro2(m_Logger, DEBUG,	"addTrackerFiducials called\n");

	//Setup a QPushButton to produce it dynamically
	pushButtonTracker = new QPushButton();
	pushButtonTracker->setFixedSize(30, 30);

	QFont font = pushButtonTracker->font();
	font.setPointSize(8);
	pushButtonTracker->setFont(font);

	if(m_maxFiducialsTracker!=25)
	{
		if(m_firstRowTracker< 6)
		{
			m_toolFrameTrackerNavigationGUI.gridLayout2->addWidget(pushButtonTracker, m_firstRowTracker, m_firstColumnTracker);
			m_firstRowTracker++;
		}

		if(m_firstRowTracker>= 6)
		{
			m_toolFrameTrackerNavigationGUI.gridLayout2->addWidget(pushButtonTracker, m_secondRowTracker, m_secondColumnTracker);
			m_secondRowTracker++;
		}

		if(m_secondRowTracker>= 6)
		{
			m_toolFrameTrackerNavigationGUI.gridLayout2->addWidget(pushButtonTracker, m_thirdRowTracker, m_thirdColumnTracker);
			m_thirdRowTracker++;
		}

		if(m_thirdRowTracker>= 6)
		{
			m_toolFrameTrackerNavigationGUI.gridLayout2->addWidget(pushButtonTracker, m_fourthRowTracker, m_fourthColumnTracker);
			m_fourthRowTracker++;
		}

		if(m_fourthRowTracker>=6 && m_fifthRowTracker!=5)
		{
			m_toolFrameTrackerNavigationGUI.gridLayout2->addWidget(pushButtonTracker, m_fifthRowTracker, m_fifthColumnTracker);
			m_fifthRowTracker++;
		}

	m_maxFiducialsTracker++;

	std::cout<<"m_maxFiducialsTracker: "<<m_maxFiducialsTracker<<std::endl;

	pushButtonTracker->setText(QString::number(m_maxFiducialsTracker));

	buttonGroupTracker = new QButtonGroup(this);
	buttonGroupTracker->addButton(pushButtonTracker);
	buttonGroupTracker->setId(pushButton, m_maxFiducialsTracker);

	test2(pushButtonTracker);

	}

	else
	{
		m_messageDialog->information(NULL, "Information", "You have reached maximum fiducial number! \nPlease click on finish button.",  QMessageBox::Ok);
	}

	connect(buttonGroupTracker, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(test2(QAbstractButton*)));

	m_toolFrameTrackerNavigationGUI.modifyFiducialsPushButton->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.clearFiducialsPushButton->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.saveFiducialsPushButton->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.fiducialSetNameLineEdit->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.addTrackerFiducialsPushButton->setEnabled(true);


}





void trackerNavigationPlugin::test2(QAbstractButton *pushButtonTracker)
{

	if(pushButtonTracker->text()== "1")
	{
		//qDebug()<<"Clicked on Button 1, set number of fiducial to 1 and call other classes"<<pushButton->text()<< endl;
		m_ActualFiducialNumberTracker= 0;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
		//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();
	}

	if(pushButtonTracker->text()== "2")
	{
		m_ActualFiducialNumberTracker= 1;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
		//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();
	}

	if(pushButtonTracker->text()== "3")
	{
		m_ActualFiducialNumberTracker= 2;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "4")
	{
		m_ActualFiducialNumberTracker= 3;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "5")
	{
		m_ActualFiducialNumberTracker= 4;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "6")
	{
		m_ActualFiducialNumberTracker= 5;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "7")
	{
		m_ActualFiducialNumberTracker= 6;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "8")
	{
		m_ActualFiducialNumberTracker= 7;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}


	if(pushButtonTracker->text()== "9")
	{
		m_ActualFiducialNumberTracker= 8;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "10")
	{
		m_ActualFiducialNumberTracker= 9;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "11")
	{
		m_ActualFiducialNumberTracker= 10;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "12")
	{
		m_ActualFiducialNumberTracker= 11;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "13")
	{
		m_ActualFiducialNumberTracker= 12;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "14")
	{
		m_ActualFiducialNumberTracker= 13;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "15")
	{
		m_ActualFiducialNumberTracker= 14;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "16")
	{
		m_ActualFiducialNumberTracker= 15;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "17")
	{
		m_ActualFiducialNumberTracker= 16;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "18")
	{
		m_ActualFiducialNumberTracker= 17;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "19")
	{
		m_ActualFiducialNumberTracker= 18;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "20")
	{
		m_ActualFiducialNumberTracker= 19;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "21")
	{
		m_ActualFiducialNumberTracker= 20;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "22")
	{
		m_ActualFiducialNumberTracker= 21;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "23")
	{
		m_ActualFiducialNumberTracker= 22;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "24")
	{
		m_ActualFiducialNumberTracker= 23;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}

	if(pushButtonTracker->text()== "25")
	{
		m_ActualFiducialNumberTracker= 24;
		pushButtonTracker->setStyleSheet(m_BackgroundYellowStyle);
				//requestChangeSelectedTrackerFiducial();
		setTrackerFiducialProcessing();;
	}



}




void trackerNavigationPlugin::deleteAllTrackerFiducials()
{
	while(!m_toolFrameTrackerNavigationGUI.gridLayout2->isEmpty())
	{
		QWidget *gridLayoutWidget2 = m_toolFrameTrackerNavigationGUI.gridLayout2->takeAt(0)->widget();
	    delete gridLayoutWidget2;
	}


	for (int i=0; i<m_maxFiducialsTracker; i++)
	{
		m_ViewImageAxial->RequestRemoveObject(m_AxialTrackerFiducialRepresentationVector[i]);
		m_ViewImageSagittal->RequestRemoveObject(m_SagittalTrackerFiducialRepresentationVector[i]);
		m_ViewImageCoronal->RequestRemoveObject(m_CoronalTrackerFiducialRepresentationVector[i]);
		m_ViewMPR->RequestRemoveObject(m_3DViewTrackerFiducialRepresentationVector[i]);
	}

	//m_LandmarksContainer.clear();

	m_maxFiducialsTracker= 0;
	m_maxFiducialNumberTracker= 0;
	m_ActualFiducialNumberTracker= 0;

	m_firstRowTracker= 0;
	m_secondRowTracker= 0;
	m_thirdRowTracker= 0;
	m_fourthRowTracker= 0;
	m_fifthRowTracker= 0;

	m_firstColumnTracker= 0;
	m_secondColumnTracker= 1;
	m_thirdColumnTracker= 2;
	m_fourthColumnTracker= 3;
	m_fifthColumnTracker= 4;
	m_sixthColumnTracker= 5;


	m_toolFrameTrackerNavigationGUI.modifyTrackerFiducialsPushButton->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.clearTrackerFiducialsPushButton->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.finishImageAndTrackerFiducialsPushButton->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.startRegistrationNavigationPushButton->setEnabled(false);


	refreshImageViews();
	resetImageCameraSettings();

}



/** Initalize Axial, Sagittal, Coronal and MPR Views */
void trackerNavigationPlugin::initializeQuadrantView()
{

	//Create views
	m_ViewImageAxial = View2DType::New();
	m_ViewImageSagittal = View2DType::New();
	m_ViewImageCoronal = View2DType::New();
	m_ViewMPR = View3DType::New();

	// Set IGSTK view for Qt display
	m_mainFrameNavigationViewGUI.navigationViewDisplayMPR->RequestSetView (m_ViewMPR);
	m_mainFrameNavigationViewGUI.navigationViewDisplayAxial->RequestSetView (m_ViewImageAxial);
	m_mainFrameNavigationViewGUI.navigationViewDisplayCoronal->RequestSetView (m_ViewImageCoronal);
	m_mainFrameNavigationViewGUI.navigationViewDisplaySagittal->RequestSetView (m_ViewImageSagittal);

	// Enable Qt display interaction
	m_mainFrameNavigationViewGUI.navigationViewDisplayMPR->RequestEnableInteractions();
	m_mainFrameNavigationViewGUI.navigationViewDisplayAxial->RequestEnableInteractions();
	m_mainFrameNavigationViewGUI.navigationViewDisplayCoronal->RequestEnableInteractions();
	m_mainFrameNavigationViewGUI.navigationViewDisplaySagittal->RequestEnableInteractions();

	// set background color to the views
	m_ViewImageAxial->SetRendererBackgroundColor(0,0,0);
	m_ViewImageSagittal->SetRendererBackgroundColor(0,0,0);
	m_ViewImageCoronal->SetRendererBackgroundColor(0,0,0);
	m_ViewMPR->SetRendererBackgroundColor(0,0,0);
	//m_ViewMPR->SetRendererBackgroundColor(1,1,1);

	//m_ViewImageAxial->SetupColoredRectangle(1,0,0);
	//m_ViewImageSagittal->SetupColoredRectangle(0,1,0);
	//m_ViewImageCoronal->SetupColoredRectangle(0,0,1);
	//m_ViewMPR->SetupColoredRectangle(1,1,0);
	//refreshImageViews();


}


void trackerNavigationPlugin::readCTDataset(ImageSpatialObjectType::Pointer dicomSpatialObject) //std::string openFileWindowFilePath
{

	igstkLogMacro2(m_Logger, DEBUG,	"LoadImageProcessing called\n");

/*
	QFileDialog::Options options = QFileDialog::DontResolveSymlinks; options |= QFileDialog::DontUseNativeDialog;
	QString directory = QFileDialog::getExistingDirectory(toolFrameLoadDicomWidget, "Select DICOM directory!", "Select DICOM directory!", options);

	if (directory.isEmpty())
	{
		QErrorMessage *errorMessageDialog = new QErrorMessage(toolFrameLoadDicomWidget);
		errorMessageDialog->showMessage("No directory was selected\n");
		return;
	}


	m_CTImageDirectory= directory.toStdString();


	//m_CTImageDirectory= "/home/yusuf/workspace/Rhinospider_Data/RS3.0Phantom";
	//m_CTImageDirectory= "/home/yusuf/A_Test_CT_Images";
	//m_CTImageDirectory= "/home/florian/Dokumente/open4DNav_testdata/RS6.1PhantomV2";
	m_CTImageDirectory= "/home/yusuf/workspace/CT_images/phantom";
	//m_CTImageDirectory= "/home/yusuf/workspace/Rhinospider_Data/RS6.1PhantomV2";


	igstkLogMacro2(m_Logger, DEBUG,	"Set selected image-directory to readImageFiles\n");

	// Setup image reader
	m_ImageReader = ImageReaderType::New();

	// Build itk progress command to assess image load progress
	//itk::SmartPointer<ProgressCommandType>            progressCommand;
	//progressCommand = ProgressCommandType::New();

	// Set the callback to the itk progress command
	//progressCommand->SetCallbackFunction(this, &trackerNavigationPlugin::onITKProgressEvent);

	// Provide a progress observer to the image reader
	//m_ImageReader->RequestSetProgressCallback(progressCommand);

	// Add observer for invalid directory
	DICOMImageReaderInvalidDirectoryNameErrorObserver::Pointer didcb= DICOMImageReaderInvalidDirectoryNameErrorObserver::New();

	m_ImageReader->AddObserver(igstk::DICOMImageDirectoryIsNotDirectoryErrorEvent(), didcb);

	// Add observer for a non-existing directory
	DICOMImageReaderNonExistingDirectoryErrorObserver::Pointer dndcb= DICOMImageReaderNonExistingDirectoryErrorObserver::New();

	m_ImageReader->AddObserver(igstk::DICOMImageDirectoryDoesNotExistErrorEvent(), dndcb);

	// Add observer for a an empty directory name (null string)
	DICOMImageReaderEmptyDirectoryErrorObserver::Pointer decb= DICOMImageReaderEmptyDirectoryErrorObserver::New();
	m_ImageReader->AddObserver(igstk::DICOMImageDirectoryEmptyErrorEvent(), decb);

	// Add observer for a directory which does not have enough number of files
	DICOMImageDirectoryNameDoesNotHaveEnoughFilesErrorObserver::Pointer ddhefcb= DICOMImageDirectoryNameDoesNotHaveEnoughFilesErrorObserver::New();

	m_ImageReader->AddObserver(igstk::DICOMImageDirectoryDoesNotHaveEnoughFilesErrorEvent(), ddhefcb);

	// Add observer for a directory containing non-DICOM files
	DICOMImageDirectoryDoesNotContainValidDICOMSeriesErrorObserver::Pointer disgcb=	DICOMImageDirectoryDoesNotContainValidDICOMSeriesErrorObserver::New();

	m_ImageReader->AddObserver(igstk::DICOMImageSeriesFileNamesGeneratingErrorEvent(), disgcb);

	// Add observer for reading invalid/corrupted dicom files
	DICOMImageInvalidErrorObserver::Pointer dircb= DICOMImageInvalidErrorObserver::New();

	m_ImageReader->AddObserver(igstk::DICOMImageReadingErrorEvent(), dircb);

	// Set directory
	m_ImageReader->RequestSetDirectory(m_CTImageDirectory);

	if(didcb->GotDICOMImageReaderInvalidDirectoryNameError())
	{
		igstkLogMacro2(m_Logger, DEBUG,	"DICOMImageReaderInvalidDirectoryNameError\n");
		return;
	}

	if(dndcb->GotDICOMImageReaderNonExistingDirectoryError())
	{
		m_messageDialog->warning(NULL, "Warning", "You have no permission to see the content of this directory \n or the selected directory does not exist!", QMessageBox::Ok);
		igstkLogMacro2(m_Logger, DEBUG,	"DICOMImageReaderNonExistingDirectoryError\n");
		return;
	}

	if(decb->GotDICOMImageReaderEmptyDirectoryError())
	{
		m_messageDialog->warning(NULL, "Warning", "The selected directory does not have enough DICOM files \n or the directory is empty!", QMessageBox::Ok);
		igstkLogMacro2(m_Logger, DEBUG,	"DICOMImageReaderEmptyDirectoryError\n");
	    return;
	}

	if(ddhefcb->GotDICOMImageDirectoryNameDoesNotHaveEnoughFilesError())
	{
		m_messageDialog->warning(NULL, "Warning", "The selected directory does not have enough DICOM files \n or the directory is empty!",  QMessageBox::Ok);

		igstkLogMacro2(m_Logger, DEBUG,	"DICOMImageDirectoryNameDoesNotHaveEnoughFilesError\n");
		return;
	}

	if(disgcb->GotDICOMImageDirectoryDoesNotContainValidDICOMSeriesError())
	{
		m_messageDialog->warning(NULL, "Warning", "The selected directory does not contain valid DICOM files!", QMessageBox::Ok);

		igstkLogMacro2(m_Logger, DEBUG,	"DICOMImageDirectoryDoesNotContainValidDICOMSeriesError\n");
		return;
	}

	// Read CT Image
	m_ImageReader->RequestReadImage();

	m_ImageObserver = CTImageObserver::New();
	m_ImageReader->AddObserver(ImageReaderType::ImageModifiedEvent(), m_ImageObserver);

	m_ImageReader->RequestGetImage();

	if(!m_ImageObserver->GotCTImage())
	{
		m_messageDialog->warning(NULL, "Warning", "Could not read DICOM dataset!", QMessageBox::Ok);

		m_ImageObserver->RemoveAllObservers();
		m_ImageObserver = NULL;

		igstkLogMacro2(m_Logger, DEBUG,	"Could not read DICOM images\n");
		return;
	}

	if (m_ImageObserver.IsNotNull())
	{*/
		//m_ImageSpatialObject = m_ImageObserver->GetCTImage();
	m_ImageSpatialObject = dicomSpatialObject;

		igstk::Transform identity;
		identity.SetToIdentity( igstk::TimeStamp::GetLongestPossibleTime() );

		// Set transform and parent to the image spatial object
		m_ImageSpatialObject->RequestSetTransformAndParent(identity, m_WorldReference);


		// Create a directory for logging files
		/*
		m_logDirectoryName= m_CTImageReader->GetPatientName()+"_Log_Files";
		m_logDirectoryNameDate= itksys::SystemTools::GetCurrentDateTime("%d%m%Y_%H%M%S");

		m_logDirectoryNameCurrentlyUsed=(m_logDirectoryName + "/" + m_logDirectoryNameDate);
		*/

		displayCTDataSet();
		resetImageCameraSettings();
		refreshImageViews();
		//updateImageData();

		igstkLogMacro2(m_Logger, DEBUG,	"displayCTDataSet called\n");
	//}

}




void trackerNavigationPlugin::displayCTDataSet()
{
	igstkLogMacro2( m_Logger, DEBUG, "trackerNavigationPlugin::displayCTDataSet called...\n" )

		//once a dataset is loaded remove all objects from all views
		if(m_ImageLoaded)
		{

			m_ViewImageAxial->RequestRemoveObject( m_AxialPlaneRepresentation );
			m_ViewImageSagittal->RequestRemoveObject( m_SagittalPlaneRepresentation );
			m_ViewImageCoronal->RequestRemoveObject( m_CoronalPlaneRepresentation );

			m_ViewMPR->RequestRemoveObject( m_AxialPlaneRepresentation2 );
			m_ViewMPR->RequestRemoveObject( m_SagittalPlaneRepresentation2 );
			m_ViewMPR->RequestRemoveObject( m_CoronalPlaneRepresentation2 );

			m_ViewImageAxial->RequestRemoveObject( m_AxialCrossHairRepresentation );
			m_ViewImageSagittal->RequestRemoveObject( m_SagittalCrossHairRepresentation );
			m_ViewImageCoronal->RequestRemoveObject( m_CoronalCrossHairRepresentation );
			m_ViewMPR->RequestRemoveObject( m_3DViewCrossHairRepresentation );


			for (int i=0; i<m_maxFiducialNumber; i++)
			{
				m_ViewImageAxial->RequestRemoveObject(m_AxialFiducialRepresentationVector[i]);
			    m_ViewImageSagittal->RequestRemoveObject(m_SagittalFiducialRepresentationVector[i]);
			    m_ViewImageCoronal->RequestRemoveObject(m_CoronalFiducialRepresentationVector[i]);
			    m_ViewMPR->RequestRemoveObject(m_3DViewFiducialRepresentationVector[i]);
			}

			for (int j=0; j<m_maxFiducialNumberTracker; j++)
			{
				m_ViewImageAxial->RequestRemoveObject(m_AxialTrackerFiducialRepresentationVector[j]);
			    m_ViewImageSagittal->RequestRemoveObject(m_SagittalTrackerFiducialRepresentationVector[j]);
			    m_ViewImageCoronal->RequestRemoveObject(m_CoronalTrackerFiducialRepresentationVector[j]);
			    m_ViewMPR->RequestRemoveObject(m_3DViewTrackerFiducialRepresentationVector[j]);
			}

		}

		igstk::Transform identity;
		identity.SetToIdentity( igstk::TimeStamp::GetLongestPossibleTime() );

		/** Fiducials  */
		// instantiate a plan object
		m_Plan = new igstk::FiducialsPlan;

		//Set up image fiducual representations
		m_FiducialPointVector.resize(m_maxFiducialNumber);
		m_AxialFiducialRepresentationVector.resize(m_maxFiducialNumber);
		m_SagittalFiducialRepresentationVector.resize(m_maxFiducialNumber);
		m_CoronalFiducialRepresentationVector.resize(m_maxFiducialNumber);
		m_3DViewFiducialRepresentationVector.resize(m_maxFiducialNumber);

		for (int i=0; i<m_maxFiducialNumber; i++)
		{
			m_FiducialPointVector[i] = EllipsoidType::New();
			m_FiducialPointVector[i]->SetRadius( m_FiducialSize, m_FiducialSize, m_FiducialSize );

			m_AxialFiducialRepresentationVector[i] = EllipsoidRepresentationType::New();
			m_AxialFiducialRepresentationVector[i]->RequestSetEllipsoidObject( m_FiducialPointVector[i] );
			m_AxialFiducialRepresentationVector[i]->SetOpacity( 0.75 );

			m_SagittalFiducialRepresentationVector[i] = EllipsoidRepresentationType::New();
			m_SagittalFiducialRepresentationVector[i]->RequestSetEllipsoidObject( m_FiducialPointVector[i] );
			m_SagittalFiducialRepresentationVector[i]->SetOpacity( 0.75 );

			m_CoronalFiducialRepresentationVector[i] = EllipsoidRepresentationType::New();
			m_CoronalFiducialRepresentationVector[i]->RequestSetEllipsoidObject( m_FiducialPointVector[i] );
			m_CoronalFiducialRepresentationVector[i]->SetOpacity( 0.75 );

			m_3DViewFiducialRepresentationVector[i] = EllipsoidRepresentationType::New();
			m_3DViewFiducialRepresentationVector[i]->RequestSetEllipsoidObject( m_FiducialPointVector[i] );
			m_3DViewFiducialRepresentationVector[i]->SetOpacity( 0.75 );

			m_AxialFiducialRepresentationVector[i]->SetColor( 0.0, 0.0, 1.0);
			m_SagittalFiducialRepresentationVector[i]->SetColor( 0.0, 0.0, 1.0);
			m_CoronalFiducialRepresentationVector[i]->SetColor( 0.0, 0.0, 1.0);
			m_3DViewFiducialRepresentationVector[i]->SetColor( 0.0, 0.0, 1.0);
		}


		//Set up tracker fiducual representations
		m_TrackerFiducialPointVector.resize(m_maxFiducialNumberTracker);
		m_AxialTrackerFiducialRepresentationVector.resize(m_maxFiducialNumberTracker);
		m_SagittalTrackerFiducialRepresentationVector.resize(m_maxFiducialNumberTracker);
		m_CoronalTrackerFiducialRepresentationVector.resize(m_maxFiducialNumberTracker);
		m_3DViewTrackerFiducialRepresentationVector.resize(m_maxFiducialNumberTracker);

		for (int j=0; j<m_maxFiducialNumberTracker; j++)
		{
			m_TrackerFiducialPointVector[j] = EllipsoidType::New();
			m_TrackerFiducialPointVector[j]->SetRadius( m_FiducialSize, m_FiducialSize, m_FiducialSize );

			m_AxialTrackerFiducialRepresentationVector[j] = EllipsoidRepresentationType::New();
			m_AxialTrackerFiducialRepresentationVector[j]->RequestSetEllipsoidObject( m_TrackerFiducialPointVector[j] );
			m_AxialTrackerFiducialRepresentationVector[j]->SetOpacity( 0.75 );

			m_SagittalTrackerFiducialRepresentationVector[j] = EllipsoidRepresentationType::New();
			m_SagittalTrackerFiducialRepresentationVector[j]->RequestSetEllipsoidObject( m_TrackerFiducialPointVector[j] );
			m_SagittalTrackerFiducialRepresentationVector[j]->SetOpacity( 0.75 );

			m_CoronalTrackerFiducialRepresentationVector[j] = EllipsoidRepresentationType::New();
			m_CoronalTrackerFiducialRepresentationVector[j]->RequestSetEllipsoidObject( m_TrackerFiducialPointVector[j] );
			m_CoronalTrackerFiducialRepresentationVector[j]->SetOpacity( 0.75 );

			m_3DViewTrackerFiducialRepresentationVector[j] = EllipsoidRepresentationType::New();
			m_3DViewTrackerFiducialRepresentationVector[j]->RequestSetEllipsoidObject( m_TrackerFiducialPointVector[j] );
			m_3DViewTrackerFiducialRepresentationVector[j]->SetOpacity( 0.75 );

			m_AxialTrackerFiducialRepresentationVector[j]->SetColor( 85, 255, 0);
			m_SagittalTrackerFiducialRepresentationVector[j]->SetColor( 85, 255, 0);
			m_CoronalTrackerFiducialRepresentationVector[j]->SetColor( 85, 255, 0);
			m_3DViewTrackerFiducialRepresentationVector[j]->SetColor( 85, 255, 0);
		}


		/** ReslicePlanes */
		// create reslice plane spatial object for axial view
		m_AxialPlaneSpatialObject = ReslicerPlaneType::New();
		m_AxialPlaneSpatialObject->RequestSetReslicingMode( ReslicerPlaneType::Orthogonal );
		m_AxialPlaneSpatialObject->RequestSetOrientationType( ReslicerPlaneType::Axial );
		m_AxialPlaneSpatialObject->RequestSetBoundingBoxProviderSpatialObject( m_ImageSpatialObject );

		// create reslice plane spatial object for sagittal view
		m_SagittalPlaneSpatialObject = ReslicerPlaneType::New();
		m_SagittalPlaneSpatialObject->RequestSetReslicingMode( ReslicerPlaneType::Orthogonal );
		m_SagittalPlaneSpatialObject->RequestSetOrientationType( ReslicerPlaneType::Sagittal );
		m_SagittalPlaneSpatialObject->RequestSetBoundingBoxProviderSpatialObject( m_ImageSpatialObject );

		// create reslice plane spatial object for coronal view
		m_CoronalPlaneSpatialObject = ReslicerPlaneType::New();
		m_CoronalPlaneSpatialObject->RequestSetReslicingMode( ReslicerPlaneType::Orthogonal );
		m_CoronalPlaneSpatialObject->RequestSetOrientationType( ReslicerPlaneType::Coronal );
		m_CoronalPlaneSpatialObject->RequestSetBoundingBoxProviderSpatialObject( m_ImageSpatialObject );

		// create reslice plane representation for axial view
		m_AxialPlaneRepresentation = ImageRepresentationType::New();
		m_AxialPlaneRepresentation->SetFrameColor(1,0,0);
		m_AxialPlaneRepresentation->SetWindowLevel(m_WindowWidth, m_WindowLevel);
		m_AxialPlaneRepresentation->RequestSetImageSpatialObject( m_ImageSpatialObject );
		m_AxialPlaneRepresentation->RequestSetReslicePlaneSpatialObject( m_AxialPlaneSpatialObject );

		// create reslice plane representation for sagittal view
		m_SagittalPlaneRepresentation = ImageRepresentationType::New();
		m_SagittalPlaneRepresentation->SetFrameColor(0,1,0);
		m_SagittalPlaneRepresentation->SetWindowLevel(m_WindowWidth, m_WindowLevel);
		m_SagittalPlaneRepresentation->RequestSetImageSpatialObject( m_ImageSpatialObject );
		m_SagittalPlaneRepresentation->RequestSetReslicePlaneSpatialObject( m_SagittalPlaneSpatialObject );

		// create reslice plane representation for coronal view
		m_CoronalPlaneRepresentation = ImageRepresentationType::New();
		m_CoronalPlaneRepresentation->SetFrameColor(0,0,1);
		m_CoronalPlaneRepresentation->SetWindowLevel(m_WindowWidth, m_WindowLevel);
		m_CoronalPlaneRepresentation->RequestSetImageSpatialObject( m_ImageSpatialObject );
		m_CoronalPlaneRepresentation->RequestSetReslicePlaneSpatialObject( m_CoronalPlaneSpatialObject );

		// Set up cross hairs
		m_CrossHair = CrossHairType::New();
		m_CrossHair->RequestSetBoundingBoxProviderSpatialObject( m_ImageSpatialObject );

		// buid the cross hair representations
		m_AxialCrossHairRepresentation = CrossHairRepresentationType::New();
		m_AxialCrossHairRepresentation->SetOpacity(1);
		m_AxialCrossHairRepresentation->SetLineWidth(2);
		m_AxialCrossHairRepresentation->RequestSetCrossHairObject( m_CrossHair );
		m_AxialCrossHairRepresentation->SetColor(1,0,0);
		//m_AxialCrossHairRepresentation->SetColorX(0,0,1);
		//m_AxialCrossHairRepresentation->SetColorY(0,1,0);
		//m_AxialCrossHairRepresentation->SetColorZ(1,1,1);

		m_SagittalCrossHairRepresentation = CrossHairRepresentationType::New();
		m_SagittalCrossHairRepresentation->SetOpacity(1);
		m_SagittalCrossHairRepresentation->SetLineWidth(2);
		m_SagittalCrossHairRepresentation->RequestSetCrossHairObject( m_CrossHair );
		m_SagittalCrossHairRepresentation->SetColor(0,0,1);
		//m_SagittalCrossHairRepresentation->SetColorX(1,1,1);
		//m_SagittalCrossHairRepresentation->SetColorY(1,0,0);
		//m_SagittalCrossHairRepresentation->SetColorZ(0,0,1);

		m_CoronalCrossHairRepresentation = CrossHairRepresentationType::New();
		m_CoronalCrossHairRepresentation->SetOpacity(1);
		m_CoronalCrossHairRepresentation->SetLineWidth(2);
		m_CoronalCrossHairRepresentation->RequestSetCrossHairObject( m_CrossHair );
		m_CoronalCrossHairRepresentation->SetColor(0,1,0);
		//m_CoronalCrossHairRepresentation->SetColorX(1,0,0);
		//m_CoronalCrossHairRepresentation->SetColorY(1,1,1);
		//m_CoronalCrossHairRepresentation->SetColorZ(0,1,0);

		m_3DViewCrossHairRepresentation = CrossHairRepresentationType::New();
		// m_3DViewCrossHairRepresentation->SetColor(1,1,0);
		m_3DViewCrossHairRepresentation->SetOpacity(1);
		m_3DViewCrossHairRepresentation->SetLineWidth(2);
		m_3DViewCrossHairRepresentation->RequestSetCrossHairObject( m_CrossHair );
		m_3DViewCrossHairRepresentation->SetColor(1,1,0);
		//m_3DViewCrossHairRepresentation->SetColorX(1,1,0);
		//m_3DViewCrossHairRepresentation->SetColorY(1,1,0);
		//m_3DViewCrossHairRepresentation->SetColorZ(1,1,0);

		// add the cross hair representation to the different views
		m_ViewImageAxial->RequestAddObject( m_AxialCrossHairRepresentation );
		m_ViewImageSagittal->RequestAddObject( m_SagittalCrossHairRepresentation );
		m_ViewImageCoronal->RequestAddObject( m_CoronalCrossHairRepresentation );
		m_ViewMPR->RequestAddObject( m_3DViewCrossHairRepresentation );


		//todo
		m_ViewImageAxial->RequestAddObject( m_PickedPointRepresentation );
		m_ViewImageSagittal->RequestAddObject( m_PickedPointRepresentation->Copy() );
		m_ViewImageCoronal->RequestAddObject( m_PickedPointRepresentation->Copy() );
		m_ViewMPR->RequestAddObject( m_PickedPointRepresentation->Copy() );


		/**
		*  Request information about the slice bounds. The answer will be
		*  received in the form of an event. This will be used to initialize
		*  the reslicing sliders and set initial slice position
		*/
		ImageExtentObserver::Pointer extentObserver = ImageExtentObserver::New();
		unsigned int extentObserverID;
		extentObserverID = m_ImageSpatialObject->AddObserver( igstk::ImageExtentEvent(), extentObserver );
		m_ImageSpatialObject->RequestGetImageExtent();


		if( extentObserver->GotImageExtent() )
		{
			//const igstk::EventHelperType::ImageExtentType& m_ImageExtent
			m_ImageExtent = extentObserver->GetImageExtent();

			const unsigned int zmin = m_ImageExtent.zmin;
			const unsigned int zmax = m_ImageExtent.zmax;
			const unsigned int zslice = static_cast< unsigned int > ( (zmin + zmax) / 2.0 );
			m_mainFrameNavigationViewGUI.scrollBarAxial->setEnabled(true);
			m_mainFrameNavigationViewGUI.scrollBarAxial->setMinimum(zmin);
			m_mainFrameNavigationViewGUI.scrollBarAxial->setMaximum(zmax);
			m_mainFrameNavigationViewGUI.scrollBarAxial->setValue(zslice);

			const unsigned int ymin = m_ImageExtent.ymin;
			const unsigned int ymax = m_ImageExtent.ymax;
			const unsigned int yslice = static_cast< unsigned int > ( (ymin + ymax) / 2.0 );
			m_mainFrameNavigationViewGUI.scrollBarSagittal->setEnabled(true);
			m_mainFrameNavigationViewGUI.scrollBarSagittal->setMinimum(ymin);
			m_mainFrameNavigationViewGUI.scrollBarSagittal->setMaximum(ymax);
			m_mainFrameNavigationViewGUI.scrollBarSagittal->setValue(yslice);

			const unsigned int xmin = m_ImageExtent.xmin;
			const unsigned int xmax = m_ImageExtent.xmax;
			const unsigned int xslice = static_cast< unsigned int > ( (xmin + xmax) / 2.0 );
			m_mainFrameNavigationViewGUI.scrollBarCoronal->setEnabled(true);
			m_mainFrameNavigationViewGUI.scrollBarCoronal->setMinimum(xmin);
			m_mainFrameNavigationViewGUI.scrollBarCoronal->setMaximum(xmax);
			m_mainFrameNavigationViewGUI.scrollBarCoronal->setValue(xslice);

		}
		m_ImageSpatialObject->RemoveObserver( extentObserverID );

		// set transform and parent to the image spatial object
		m_ImageSpatialObject->RequestSetTransformAndParent( identity, m_WorldReference );

		// set transform and parent to the image plane reslice spatial objects
		m_AxialPlaneSpatialObject->RequestSetTransformAndParent( identity, m_WorldReference );
		m_SagittalPlaneSpatialObject->RequestSetTransformAndParent( identity, m_WorldReference );
		m_CoronalPlaneSpatialObject->RequestSetTransformAndParent( identity, m_WorldReference );

		m_ViewImageAxial->RequestSetTransformAndParent( identity, m_AxialPlaneSpatialObject );
		m_ViewImageSagittal->RequestSetTransformAndParent( identity, m_SagittalPlaneSpatialObject );
		m_ViewImageCoronal->RequestSetTransformAndParent( identity, m_CoronalPlaneSpatialObject );
		m_ViewMPR->RequestSetTransformAndParent( identity, m_WorldReference );

		// set transform and parent to the cross hair object
		m_CrossHair->RequestSetTransformAndParent( identity, m_WorldReference );

		// add reslice plane representations to the orthogonal views
		m_ViewImageAxial->RequestAddObject( m_AxialPlaneRepresentation );
		m_ViewImageSagittal->RequestAddObject( m_SagittalPlaneRepresentation );
		m_ViewImageCoronal->RequestAddObject( m_CoronalPlaneRepresentation );

		// add reslice plane representations to the 3D views
		m_AxialPlaneRepresentation2 = m_AxialPlaneRepresentation->Copy();
		m_SagittalPlaneRepresentation2 = m_SagittalPlaneRepresentation->Copy();
		m_CoronalPlaneRepresentation2 = m_CoronalPlaneRepresentation->Copy();

		m_ViewMPR->RequestAddObject( m_AxialPlaneRepresentation2 );
		m_ViewMPR->RequestAddObject( m_SagittalPlaneRepresentation2 );
		m_ViewMPR->RequestAddObject( m_CoronalPlaneRepresentation2 );

		// set parallel projection in the 2D views
		m_ViewImageAxial->SetCameraParallelProjection(true);
		m_ViewImageSagittal->SetCameraParallelProjection(true);
		m_ViewImageCoronal->SetCameraParallelProjection(true);

		// Set up view parameters
		m_ViewImageAxial->SetRefreshRate(VIEW_2D_REFRESH_RATE);
		m_ViewImageSagittal->SetRefreshRate(VIEW_2D_REFRESH_RATE);
		m_ViewImageCoronal->SetRefreshRate(VIEW_2D_REFRESH_RATE);
		m_ViewMPR->SetRefreshRate(VIEW_3D_REFRESH_RATE);

		// Set annotation-texts on the corners on views
		m_AnnotationImageAxial = Annotation2DImageType::New();
		m_AxialAnnotation = "Axial";

		m_AxialAnnotationPatientName = "Patient Name: ";
		//m_AxialAnnotationPatientName.append(m_ImageReader->GetPatientName().c_str());
		m_AxialAnnotationPatientName.append("empty");
		m_AnnotationImageAxial->RequestSetAnnotationText(0, m_AxialAnnotation);
		m_AnnotationImageAxial->RequestSetAnnotationText(2, m_AxialAnnotationPatientName);
		m_AnnotationImageAxial->RequestSetFontSize(0, 12);
		m_AnnotationImageAxial->RequestSetFontSize(2, 11);
		m_AnnotationImageAxial->RequestAddAnnotations();
		m_ViewImageAxial->RequestAddAnnotation2D(m_AnnotationImageAxial);

		m_AnnotationImageSagittal = Annotation2DImageType::New();
		m_SagittalAnnotation = "Sagittal";
		m_AnnotationImageSagittal->RequestSetAnnotationText(0, m_SagittalAnnotation);
		m_AnnotationImageSagittal->RequestSetFontSize(0, 12);
		m_AnnotationImageSagittal->RequestAddAnnotations();
		m_ViewImageSagittal->RequestAddAnnotation2D( m_AnnotationImageSagittal);

		m_AnnotationImageCoronal = Annotation2DImageType::New();
		m_CoronalAnnotation = "Coronal";
		m_AnnotationImageCoronal->RequestSetAnnotationText(0, m_CoronalAnnotation);
		m_AnnotationImageCoronal->RequestSetFontSize(0, 12);
		m_AnnotationImageCoronal->RequestAddAnnotations();
		m_ViewImageCoronal->RequestAddAnnotation2D(m_AnnotationImageCoronal);

		m_AnnotationImageMPR = Annotation2DImageType::New();
		m_MPRand3DAnnotation = "MPR";
		m_AnnotationImageMPR->RequestSetAnnotationText(0, m_MPRand3DAnnotation);
		m_AnnotationImageMPR->RequestSetFontSize(0, 12);
		m_AnnotationImageMPR->RequestAddAnnotations();
		m_ViewMPR->RequestAddAnnotation2D(m_AnnotationImageMPR);

		m_ViewImageAxial->SetCameraZoomFactor(m_DefaultZoomImageAxial);
		m_ViewImageSagittal->SetCameraZoomFactor(m_DefaultZoomImageSagittal);
		m_ViewImageCoronal->SetCameraZoomFactor(m_DefaultZoomImageCoronal);
		m_ViewMPR->SetCameraZoomFactor(m_DefaultZoomImageMPR);

		/** Add observer for picking events in the Axial view */
		m_AxialViewPickerObserver = LoadedObserverType::New();
		m_AxialViewPickerObserver->SetCallbackFunction( this, &trackerNavigationPlugin::imagePickingProcessingAxial );

		m_ViewImageAxial->AddObserver(igstk::CoordinateSystemTransformToEvent(), m_AxialViewPickerObserver );

		/** Add observer for picking events in the Axial view */
		m_SagittalViewPickerObserver = LoadedObserverType::New();
		m_SagittalViewPickerObserver->SetCallbackFunction( this, &trackerNavigationPlugin::imagePickingProcessingSagittal );

		m_ViewImageSagittal->AddObserver(igstk::CoordinateSystemTransformToEvent(), m_SagittalViewPickerObserver );

		/** Add observer for picking events in the Axial view */
		m_CoronalViewPickerObserver = LoadedObserverType::New();
		m_CoronalViewPickerObserver->SetCallbackFunction( this, &trackerNavigationPlugin::imagePickingProcessingCoronal );

		m_ViewImageCoronal->AddObserver(igstk::CoordinateSystemTransformToEvent(), m_CoronalViewPickerObserver );

		//this->getImageBounds(m_ImageSpatialObject);
/*
		m_vtkUpdateCommand = vtkCallbackCommand::New();
		m_vtkUpdateCommand->SetCallback(&trackerNavigationPlugin::onVTKUpdateEvent);
		m_vtkUpdateCommand->SetClientData (this);
*/

		/*
		m_ViewMPR->RequestSetUpdateCallback( m_vtkUpdateCommand );

		m_ViewMPR->SetupOrientationBox();

		m_ViewImageAxial->SetupColoredRectangle(1,0,0);
		m_ViewImageSagittal->SetupColoredRectangle(0,1,0);
		m_ViewImageCoronal->SetupColoredRectangle(0,0,1);
		m_ViewMPR->SetupColoredRectangle(1,1,0);
		 */

		// setup a tool projection spatial object
		/*
		m_ToolProjection = ToolProjectionType::New();
		m_ToolProjection->SetSize(150);
		m_ToolProjection->RequestSetTransformAndParent( identity, m_WorldReference );

		// setup axial tool projection
		m_AxialToolProjectionRepresentation = ToolProjectionRepresentationType::New();
		m_AxialToolProjectionRepresentation->RequestSetToolProjectionObject( m_ToolProjection );
		m_AxialToolProjectionRepresentation->RequestSetReslicePlaneSpatialObject( m_AxialPlaneSpatialObject );
		m_AxialToolProjectionRepresentation->SetColor( 1,1,0 );

		// setup sagittal tool projection
		m_SagittalToolProjectionRepresentation = ToolProjectionRepresentationType::New();
		m_SagittalToolProjectionRepresentation->RequestSetToolProjectionObject( m_ToolProjection );
		m_SagittalToolProjectionRepresentation->RequestSetReslicePlaneSpatialObject( m_SagittalPlaneSpatialObject );
		m_SagittalToolProjectionRepresentation->SetColor( 1,1,0 );

		// setup coronal tool projection
		m_CoronalToolProjectionRepresentation = ToolProjectionRepresentationType::New();
		m_CoronalToolProjectionRepresentation->RequestSetToolProjectionObject( m_ToolProjection );
		m_CoronalToolProjectionRepresentation->RequestSetReslicePlaneSpatialObject( m_CoronalPlaneSpatialObject );
		m_CoronalToolProjectionRepresentation->SetColor( 1,1,0 );
*/


		m_ImageLoaded = true;

		// reset the cameras in the different views
		m_ViewMPR->RequestResetCamera();
		m_ViewImageAxial->RequestResetCamera();
		m_ViewImageSagittal->RequestResetCamera();
		m_ViewImageCoronal->RequestResetCamera();

		m_mainFrameNavigationViewGUI.sliceNumberLabelAxial->setEnabled(true);
		m_mainFrameNavigationViewGUI.imagePointNumberLabelAxial->setEnabled(true);

		m_mainFrameNavigationViewGUI.sliceNumberLabelSagittal->setEnabled(true);
		m_mainFrameNavigationViewGUI.imagePointNumberLabelSagittal->setEnabled(true);

		m_mainFrameNavigationViewGUI.sliceNumberLabelCoronal->setEnabled(true);
		m_mainFrameNavigationViewGUI.imagePointNumberLabelCoronal->setEnabled(true);


		PointType empty;
		empty.Fill(0.0);

		for(int i=m_Plan->m_FiducialPoints.size(); i < m_maxFiducialNumber; i++)
		{
			m_Plan->m_FiducialPoints.push_back( empty );
		}

		m_LandmarksContainer.clear();
		m_AcceptedLandmarksContainer.clear();

		for( unsigned int i = 0; i < m_Plan->m_FiducialPoints.size(); i++ )
		{
			if(m_Plan->m_FiducialPoints[i] == empty)
				continue;

			pushButton->setStyleSheet(m_BackgroundBlueStyle);

			switch(i)
			{
				case 0: if(pushButton->text()== "1") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 1: if(pushButton->text()== "2") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 2: if(pushButton->text()== "3") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 3: if(pushButton->text()== "4") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 4: if(pushButton->text()== "5") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 5: if(pushButton->text()== "6") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 6: if(pushButton->text()== "7") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 7: if(pushButton->text()== "8") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 8: if(pushButton->text()== "9") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 9: if(pushButton->text()== "10") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 10: if(pushButton->text()== "11") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 11: if(pushButton->text()== "12") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 12: if(pushButton->text()== "13") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 13: if(pushButton->text()== "14") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 14: if(pushButton->text()== "15") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 15: if(pushButton->text()== "16") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 16: if(pushButton->text()== "17") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 17: if(pushButton->text()== "18") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 18: if(pushButton->text()== "19") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 19: if(pushButton->text()== "20") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 20: if(pushButton->text()== "21") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 21: if(pushButton->text()== "22") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 22: if(pushButton->text()== "23") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 23: if(pushButton->text()== "24") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 24: if(pushButton->text()== "25") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			}

			m_toolFrameTrackerNavigationGUI.gridLayout1->update();
			//RegistrationType::LandmarkTrackerPointType p;
			//m_LandmarksContainer.push_back(p);
			//m_AcceptedLandmarksContainer.insert( std::pair<unsigned int,bool>(i,false) );
		}


		m_ViewMPR->RequestStart();
		m_ViewImageAxial->RequestStart();
		m_ViewImageSagittal->RequestStart();
		m_ViewImageCoronal->RequestStart();

	   refreshImageViews();


}


void trackerNavigationPlugin::onVTKUpdateEvent(vtkObject *caller, unsigned long vtkNotUsed(eid), void * clientdata, void *av)
{

	double bounds[6];


	itk::Point<double,3> p0;
	p0[0] = bounds[0];
	p0[1] = bounds[2];
	p0[2] = bounds[4];

	itk::Point<double,3> p1;
	p1[0] = bounds[1];
	p1[1] = bounds[3];
	p1[2] = bounds[5];

	IndexType index1;
	instance->m_ImageSpatialObject->TransformPhysicalPointToIndex(p0, index1);
	//cout << index1[0] << " " << index1[1] << " " << index1[2] << endl;
	IndexType index2;
	instance->m_ImageSpatialObject->TransformPhysicalPointToIndex(p1, index2);
	//cout << index2[0] << " " << index2[1] << " " << index2[2] << endl;
}





void trackerNavigationPlugin::resliceImage(IndexType index)
{

	m_mainFrameNavigationViewGUI.scrollBarAxial->setValue(index[2]);
	m_mainFrameNavigationViewGUI.scrollBarSagittal->setValue(index[0]);
	m_mainFrameNavigationViewGUI.scrollBarCoronal->setValue(index[1]);

}


/** Reslice image Axial */
void trackerNavigationPlugin::resliceImageViews(int value)
{
	IndexType index;
	index[2]= m_mainFrameNavigationViewGUI.scrollBarAxial->value();
	index[0]= m_mainFrameNavigationViewGUI.scrollBarSagittal->value();
	index[1]= m_mainFrameNavigationViewGUI.scrollBarCoronal->value();

	QString str1;
	QString str2;
	QString str3;
	str1.append(QString("%1").arg(index[2]));
	str2.append(QString("%1").arg(index[0]));
	str3.append(QString("%1").arg(index[1]));
	m_mainFrameNavigationViewGUI.sliceNumberLabelAxial->setText(str1);
	m_mainFrameNavigationViewGUI.sliceNumberLabelSagittal->setText(str2);
	m_mainFrameNavigationViewGUI.sliceNumberLabelCoronal->setText(str3);

	PointType point;
	m_ImageSpatialObject->TransformIndexToPhysicalPoint( index, point );


	const double *data = point.GetVnlVector().data_block();


	m_CrossHair->RequestSetCursorPosition( data );
	m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
	m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
	m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );



}



/** Set window level and witdth */
/*
void trackerNavigationPlugin::changeImageWindowing(int windowing)
{
	int curLevel = m_toolFrameLoadDicomGUI.mainViewSliderWindowLevel->value();
	int curWidth = m_toolFrameLoadDicomGUI.mainViewSliderWindowWidht->value();

	m_AxialPlaneRepresentation->SetWindowLevel(curWidth, curLevel);
	m_SagittalPlaneRepresentation->SetWindowLevel(curWidth, curLevel);
	m_CoronalPlaneRepresentation->SetWindowLevel(curWidth, curLevel);

	m_AxialPlaneRepresentation2->SetWindowLevel(curWidth, curLevel);
	m_SagittalPlaneRepresentation2->SetWindowLevel(curWidth, curLevel);
	m_CoronalPlaneRepresentation2->SetWindowLevel(curWidth, curLevel);


	char width[100];
	sprintf(width, "%d", (int) curWidth);
	m_toolFrameLoadDicomGUI.mainViewLabelValueWindowWidht->setText(width);

	char level[100];
	sprintf(level, "%d", (int) curLevel);
	m_toolFrameLoadDicomGUI.mainViewLabelValueWindowLevel->setText(level);

	refreshImageViews();
}
*/


/** Reslices the views to the picked position */
void trackerNavigationPlugin::imagePickingProcessing()
{

	igstkLogMacro2( m_Logger, DEBUG, "imagePickingProcessing called...\n" )

	PointType point = TransformToPoint( m_PickingTransform );

	if ( m_ImageSpatialObject->IsInside( point ) )
	{
		//todo comment
		 m_PickedPoint->RequestSetTransformAndParent( m_PickingTransform , this->m_ViewMPR);

		IndexType index;
		m_ImageSpatialObject->TransformPhysicalPointToIndex( point, index);


		const double *data = point.GetVnlVector().data_block();
/*
		m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
		m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
		m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
		m_CrossHair->RequestSetCursorPosition( data );
*/
		this->resliceImage(index);

		QString str1;
		QString str2;
		QString str3;
		str1.append(QString("%1").arg(data[2]));
		str2.append(QString("%1").arg(data[0]));
		str3.append(QString("%1").arg(data[1]));
		m_mainFrameNavigationViewGUI.imagePointNumberLabelAxial->setText(str1);
		m_mainFrameNavigationViewGUI.imagePointNumberLabelSagittal->setText(str2);
		m_mainFrameNavigationViewGUI.imagePointNumberLabelCoronal->setText(str3);
	}
	else
	{
		igstkLogMacro( DEBUG,  "Picked point outside image...\n" )
	}
}




/** Reslices the views Sagittal and Coronal to the picked position on Axial */
void trackerNavigationPlugin::imagePickingProcessingAxial(const itk::EventObject & event)
{
	if ( igstk::CoordinateSystemTransformToEvent().CheckEvent( &event ) )
	{
		typedef igstk::CoordinateSystemTransformToEvent TransformEventType;
		const TransformEventType *tmevent= dynamic_cast< const TransformEventType *>( & event );

		// get the transform from the view to its parent (reslicer plane)
		igstk::CoordinateSystemTransformToResult transformCarrier = tmevent->Get();
		m_PickingTransform = transformCarrier.GetTransform();

		// get the transform from the reslicer plane to its parent (world reference)
		CoordinateSystemTransformObserver::Pointer coordinateObserver =
			CoordinateSystemTransformObserver::New();

		unsigned int obsId = m_AxialPlaneSpatialObject->AddObserver(igstk::CoordinateSystemTransformToEvent(), coordinateObserver);

		m_AxialPlaneSpatialObject->RequestComputeTransformTo(m_WorldReference);

		if( coordinateObserver->GotCoordinateSystemTransform())
		{
			igstk::CoordinateSystemTransformToResult transformToResult = coordinateObserver->GetCoordinateSystemTransform();
			igstk::Transform viewToWorldReferenceTransform = transformToResult.GetTransform();
			m_PickingTransform = igstk::Transform::TransformCompose( viewToWorldReferenceTransform, m_PickingTransform );
		}
		else
		{
			igstkLogMacro2( m_Logger, DEBUG, "imagePickingProcessingAxial could not get coordinate system transform...\n" )
				return;
		}

		m_AxialPlaneSpatialObject->RemoveObserver( obsId );

	}
	imagePickingProcessing();
	//resetImageCameraSettings();
	refreshImageViews();
}


/** Reslices the views Axial and Coronal to the picked position on Sagittal */
void trackerNavigationPlugin::imagePickingProcessingSagittal(const itk::EventObject & event)
{
	if (igstk::CoordinateSystemTransformToEvent().CheckEvent(&event))
		{
			typedef igstk::CoordinateSystemTransformToEvent TransformEventType;
			const TransformEventType *tmevent= dynamic_cast< const TransformEventType *>( & event );

			// get the transform from the view to its parent (reslicer plane)
			igstk::CoordinateSystemTransformToResult transformCarrier = tmevent->Get();
			m_PickingTransform = transformCarrier.GetTransform();

			// get the transform from the reslicer plane to its parent (world reference)
			CoordinateSystemTransformObserver::Pointer coordinateObserver= CoordinateSystemTransformObserver::New();

			unsigned int obsId = m_SagittalPlaneSpatialObject->AddObserver(igstk::CoordinateSystemTransformToEvent(), coordinateObserver);

			m_SagittalPlaneSpatialObject->RequestComputeTransformTo(m_WorldReference);

			if(coordinateObserver->GotCoordinateSystemTransform())
			{
				igstk::CoordinateSystemTransformToResult transformToResult = coordinateObserver->GetCoordinateSystemTransform();
				igstk::Transform viewToWorldReferenceTransform = transformToResult.GetTransform();
				m_PickingTransform = igstk::Transform::TransformCompose( viewToWorldReferenceTransform, m_PickingTransform );
			}
			else
			{
				igstkLogMacro2( m_Logger, DEBUG, "sagittalViewPickingCallback could not get coordinate system transform...\n" )
					return;
			}

			m_SagittalPlaneSpatialObject->RemoveObserver( obsId );

		}

	imagePickingProcessing();
	//resetImageCameraSettings();
	refreshImageViews();
}


/** Reslices the views Axial and Sagittal to the picked position on Coronal */
void trackerNavigationPlugin::imagePickingProcessingCoronal(const itk::EventObject & event)
{
	if ( igstk::CoordinateSystemTransformToEvent().CheckEvent(&event ))
		{
			typedef igstk::CoordinateSystemTransformToEvent TransformEventType;
			const TransformEventType *tmevent= dynamic_cast< const TransformEventType *>(&event);

			// get the transform from the view to its parent (reslicer plane)
			igstk::CoordinateSystemTransformToResult transformCarrier = tmevent->Get();
			m_PickingTransform = transformCarrier.GetTransform();

			// get the transform from the reslicer plane to its parent (world reference)
			CoordinateSystemTransformObserver::Pointer coordinateObserver=	CoordinateSystemTransformObserver::New();

			unsigned int obsId = m_CoronalPlaneSpatialObject->AddObserver(igstk::CoordinateSystemTransformToEvent(), coordinateObserver);

			m_CoronalPlaneSpatialObject->RequestComputeTransformTo(m_WorldReference);

			if( coordinateObserver->GotCoordinateSystemTransform())
			{
				igstk::CoordinateSystemTransformToResult transformToResult = coordinateObserver->GetCoordinateSystemTransform();
				igstk::Transform viewToWorldReferenceTransform = transformToResult.GetTransform();
				m_PickingTransform = igstk::Transform::TransformCompose( viewToWorldReferenceTransform, m_PickingTransform );
			}
			else
			{
				igstkLogMacro2( m_Logger, DEBUG, "coronalViewPickingCallback could not get coordinate system transform...\n" )
					return;
			}

			m_CoronalPlaneSpatialObject->RemoveObserver( obsId );

		}
	imagePickingProcessing();
	//resetImageCameraSettings();
	refreshImageViews();
}


//Sets fiducials on the image and reslices the views to that position
void trackerNavigationPlugin::setImageFiducialProcessing()
{
	igstkLogMacro2( m_Logger, DEBUG, "setImageFiducialProcessing called...\n" )

	PointType point = TransformToPoint( m_PickingTransform );

	if(m_ImageSpatialObject->IsInside(point ))
	{
		int choice = m_ActualFiducialNumber;

		m_FiducialPointVector[choice]->RequestSetTransformAndParent(m_PickingTransform, m_WorldReference );

		m_Plan->m_FiducialPoints[choice] = point;


		//pushButton->setStyleSheet(m_BackgroundBlueStyle);

		IndexType index;
		m_ImageSpatialObject->TransformPhysicalPointToIndex(point, index);

		const double *data = point.GetVnlVector().data_block();

		m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
		m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
		m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
		m_CrossHair->RequestSetCursorPosition( data );

		resliceImage(index);
	}
	else
	{
		igstkLogMacro( DEBUG,  "Picked point outside image...\n" )
	}
}





//Sets fiducials on the patient and reslices the views to that position
void trackerNavigationPlugin::setTrackerFiducialProcessing()
{
	igstkLogMacro2( m_Logger, DEBUG, "setTrackerFiducialProcessing called...\n" )



	/////////////////WAS TRACKER POINT MODIFY START requestChangeSelectedTrackerFiducial/////////////////


	cout<<"m_ActualFiducialNumberTracker :" << m_ActualFiducialNumberTracker <<endl;
	cout<<"m_LandmarksContainer:" << m_LandmarksContainer.size() <<endl;

	if ( m_ActualFiducialNumberTracker == -1)
	{
		igstkLogMacro2( m_Logger, DEBUG, "Error m_ActualFiducialNumberTracker == -1...\n" )
			return;
	}

	if(m_ActualFiducialNumberTracker > (m_LandmarksContainer.size()-1))
	{
		igstkLogMacro2( m_Logger, DEBUG, "Error m_ActualFiducialNumberTracker > (m_LandmarksContainer.size()-1)...\n" )
			return;
	}

	//get fiducial coordinates
	PointType point;

	point = m_LandmarksContainer[m_ActualFiducialNumberTracker];

	igstk::Transform trackerPointTransform = igstk::Transform::TransformCompose(m_RegistrationTransform, PointToTransform(point));

	m_TrackerFiducialPointVector[m_ActualFiducialNumberTracker]->RequestSetTransformAndParent( trackerPointTransform, m_WorldReference );

	for (int i=0; i<m_maxFiducialNumberTracker; i++)
	{
		m_ViewImageAxial->RequestRemoveObject( m_AxialTrackerFiducialRepresentationVector[i] );
		m_ViewImageSagittal->RequestRemoveObject( m_SagittalTrackerFiducialRepresentationVector[i] );
		m_ViewImageCoronal->RequestRemoveObject( m_CoronalTrackerFiducialRepresentationVector[i] );
		m_ViewMPR->RequestRemoveObject( m_3DViewTrackerFiducialRepresentationVector[i] );
	}

	m_ViewImageAxial->RequestAddObject( m_AxialTrackerFiducialRepresentationVector[m_ActualFiducialNumberTracker] );
	m_ViewImageSagittal->RequestAddObject(m_SagittalTrackerFiducialRepresentationVector[m_ActualFiducialNumberTracker] );
	m_ViewImageCoronal->RequestAddObject( m_CoronalTrackerFiducialRepresentationVector[m_ActualFiducialNumberTracker] );
	m_ViewMPR->RequestAddObject( m_3DViewTrackerFiducialRepresentationVector[m_ActualFiducialNumberTracker] );

	char buf2[50];
	sprintf( buf2, "[%.2f, %.2f, %.2f]", point[0], point[1], point[2]);

	// Reslice image to the selected point position
	if( m_ImageSpatialObject->IsInside(point))
	{
		IndexType index;
		m_ImageSpatialObject->TransformPhysicalPointToIndex( point	, index);

		const double *data2 = point.GetVnlVector().data_block();

		m_AxialPlaneSpatialObject->RequestSetCursorPosition( data2 );
		m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data2 );
		m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data2 );
		m_CrossHair->RequestSetCursorPosition( data2 );

		resliceImage(index);
	}
	else
	{
		igstkLogMacro2( m_Logger, DEBUG, "This tracker point is not defined in the image...\n" )
	}

	refreshImageViews();

	/////////////////WAS TRACKER POINT MODIFY END requestChangeSelectedTrackerFiducial/////////////////



	//PointType
	point = TransformToPoint( m_RegistrationTransform );


	if(m_ImageSpatialObject->IsInside(point ))
	{
		int choice = m_ActualFiducialNumberTracker;

		m_TrackerFiducialPointVector[choice]->RequestSetTransformAndParent(m_RegistrationTransform, m_WorldReference );

		m_LandmarksContainer[choice] = point;


		//pushButton->setStyleSheet(m_BackgroundYellowStyle);

		IndexType index;
		m_ImageSpatialObject->TransformPhysicalPointToIndex(point, index);

		const double *data = point.GetVnlVector().data_block();

		m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
		m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
		m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
		m_CrossHair->RequestSetCursorPosition( data );

		resliceImage(index);
	}
	else
	{
		igstkLogMacro( DEBUG,  "Tracked point outside image...\n" )
	}

}




//Switches the currently active image fiducial
void trackerNavigationPlugin::requestChangeSelectedFiducial()
{

	igstkLogMacro2( m_Logger, DEBUG, "requestChangeSelectedFiducial called...\n" )

		cout<<"m_ActualFiducialNumber :" << m_ActualFiducialNumber <<endl;
		cout<<"m_Plan->m_FiducialPoints:" << m_Plan->m_FiducialPoints.size() <<endl;

	if ( m_ActualFiducialNumber == -1)
	{
		igstkLogMacro2( m_Logger, DEBUG, "Error m_ActualFiducialNumber == -1...\n" )

		return;
	}

	if(m_ActualFiducialNumber > (m_Plan->m_FiducialPoints.size()-1))
	{
		igstkLogMacro2( m_Logger, DEBUG, "Error m_ActualFiducialNumber > (m_Plan->m_FiducialPoints.size()-1)...\n" )

		return;
	}

	// get fiducial coordinates
	PointType point;
	point = m_Plan->m_FiducialPoints[m_ActualFiducialNumber];

	m_FiducialPointVector[m_ActualFiducialNumber]->RequestSetTransformAndParent( PointToTransform(point), m_WorldReference );

	// MDD: Why for MAX_FIDUCIAL_NR? Why not just for # of fiducials existing?
	for (int i=0; i<m_maxFiducialNumber; i++)
	{
		m_ViewImageAxial->RequestRemoveObject( m_AxialFiducialRepresentationVector[i] );
		m_ViewImageSagittal->RequestRemoveObject( m_SagittalFiducialRepresentationVector[i] );
		m_ViewImageCoronal->RequestRemoveObject( m_CoronalFiducialRepresentationVector[i] );
		m_ViewMPR->RequestRemoveObject( m_3DViewFiducialRepresentationVector[i] );
	}

	m_ViewImageAxial->RequestAddObject( m_AxialFiducialRepresentationVector[m_ActualFiducialNumber] );
	m_ViewImageSagittal->RequestAddObject( m_SagittalFiducialRepresentationVector[m_ActualFiducialNumber] );
	m_ViewImageCoronal->RequestAddObject( m_CoronalFiducialRepresentationVector[m_ActualFiducialNumber] );
	m_ViewMPR->RequestAddObject( m_3DViewFiducialRepresentationVector[m_ActualFiducialNumber] );

	char buf[50];
	sprintf( buf, "[%.2f, %.2f, %.2f]", point[0], point[1], point[2]);

	// Reslice image to the selected point position
	if( m_ImageSpatialObject->IsInside( point ) )
	{
		IndexType index;
		m_ImageSpatialObject->TransformPhysicalPointToIndex( point, index);

		const double *data = point.GetVnlVector().data_block();

		m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
		m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
		m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
		m_CrossHair->RequestSetCursorPosition( data );

		printf("%f %f %f",data[0],data[1],data[2]);

		resliceImage(index);
	}
	else
	{
		igstkLogMacro2( m_Logger, DEBUG, "This point is not defined in the image...\n" )
	}


	refreshImageViews();

}



/*
//Switches the currently active tracker fiducial
void trackerNavigationPlugin::requestChangeSelectedTrackerFiducial()
{

	igstkLogMacro2( m_Logger, DEBUG, "requestChangeSelectedTrackerFiducial called...\n" )

		cout<<"m_ActualFiducialNumberTracker :" << m_ActualFiducialNumberTracker <<endl;
		cout<<"m_LandmarksContainer:" << m_LandmarksContainer.size() <<endl;

	if ( m_ActualFiducialNumberTracker == -1)
	{
		igstkLogMacro2( m_Logger, DEBUG, "Error m_ActualFiducialNumberTracker == -1...\n" )
		return;
	}

	if(m_ActualFiducialNumberTracker > (m_LandmarksContainer.size()-1))
	{
		igstkLogMacro2( m_Logger, DEBUG, "Error m_ActualFiducialNumberTracker > (m_LandmarksContainer.size()-1)...\n" )
		return;
	}

	//get fiducial coordinates
	PointType point;

	point = m_LandmarksContainer[m_ActualFiducialNumberTracker];

	igstk::Transform trackerPointTransform = igstk::Transform::TransformCompose(m_RegistrationTransform, PointToTransform(point));

	m_TrackerFiducialPointVector[m_ActualFiducialNumberTracker]->RequestSetTransformAndParent( trackerPointTransform, m_WorldReference );

	for (int i=0; i<m_maxFiducialNumberTracker; i++)
	{
		m_ViewImageAxial->RequestRemoveObject( m_AxialTrackerFiducialRepresentationVector[i] );
		m_ViewImageSagittal->RequestRemoveObject( m_SagittalTrackerFiducialRepresentationVector[i] );
		m_ViewImageCoronal->RequestRemoveObject( m_CoronalTrackerFiducialRepresentationVector[i] );
		m_ViewMPR->RequestRemoveObject( m_3DViewTrackerFiducialRepresentationVector[i] );
	}

	m_ViewImageAxial->RequestAddObject( m_AxialTrackerFiducialRepresentationVector[m_ActualFiducialNumberTracker] );
	m_ViewImageSagittal->RequestAddObject(m_SagittalTrackerFiducialRepresentationVector[m_ActualFiducialNumberTracker] );
	m_ViewImageCoronal->RequestAddObject( m_CoronalTrackerFiducialRepresentationVector[m_ActualFiducialNumberTracker] );
	m_ViewMPR->RequestAddObject( m_3DViewTrackerFiducialRepresentationVector[m_ActualFiducialNumberTracker] );

	char buf[50];
	sprintf( buf, "[%.2f, %.2f, %.2f]", point[0], point[1], point[2]);

	// Reslice image to the selected point position
	if( m_ImageSpatialObject->IsInside( point ) )
	{
		IndexType index;
		m_ImageSpatialObject->TransformPhysicalPointToIndex( point, index);

		const double *data = point.GetVnlVector().data_block();

		m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
		m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
		m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
		m_CrossHair->RequestSetCursorPosition( data );

		resliceImage(index);
	}
	else
	{
		igstkLogMacro2( m_Logger, DEBUG, "This tracker point is not defined in the image...\n" )
	}

	refreshImageViews();

}
*/


//Read fiducial points from file
void trackerNavigationPlugin::readFiducialsFromFile()
{
mutex.lock();
	deleteAllImageFiducials();

	igstkLogMacro2( m_Logger, DEBUG, "readFiducialsFromFile called...\n" )

	QFileDialog::Options options;
	QString selectedFilter;
	QString fName = QFileDialog::getOpenFileName(NULL, "Select Fiducial Plan", "*.igstk", "Fiducial Files (*);;Fiducials (*.igstk)", &selectedFilter, options);

	m_toolFrameTrackerNavigationGUI.fiducialSetNameLineEdit->setText(fName);
	//m_GUI.FiducialSetNameEdit2->setText(fName);

	std::string  fileName = fName.toStdString();

	if ( !fileName.c_str() )
	{
		igstkLogMacro2( m_Logger, DEBUG, "readFiducialsFromFile No file was selected\n" )
			return;
	}

	m_PlanFilename = fileName.c_str();

	fiducialReader = new igstk::FiducialsPlanIO;

	igstkLogMacro2( m_Logger, DEBUG, "Reading fiducials from " << m_PlanFilename << "\n")

	m_Plan = new igstk::FiducialsPlan;

	if (itksys::SystemTools::FileExists( m_PlanFilename.c_str()))
	{
		fiducialReader->SetFileName( m_PlanFilename );

		if(fiducialReader->RequestRead( ) )
		{
			m_Plan = fiducialReader->GetFiducialsPlan();
		}

		std::cout << "Fiducials loaded" << QString::number(m_Plan->m_FiducialPoints.size()).toStdString()<< std::endl;

		PointType empty;
		empty.Fill(0.0);

		for(int i=m_Plan->m_FiducialPoints.size(); i < m_maxFiducialNumber; i++)
		{
			m_Plan->m_FiducialPoints.push_back(empty);
		}
	}

	m_LandmarksContainer.clear();
	m_AcceptedLandmarksContainer.clear();


	// pushButton->setStyleSheet(m_BackgroundWhiteStyle);


	PointType empty;
	empty.Fill(0.0);

	for(unsigned int i = 0; i < m_Plan->m_FiducialPoints.size(); i++)
	{
		addImageFiducials(); //Add much fiducials, how much is it stored in file

		if(m_Plan->m_FiducialPoints[i] == empty)
			continue;

		pushButton->setStyleSheet(m_BackgroundBlueStyle);
		/*
		switch(i)
		{
			case 0: if(pushButton->text()== "1") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 1: if(pushButton->text()== "2") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 2: if(pushButton->text()== "3") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 3: if(pushButton->text()== "4") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 4: if(pushButton->text()== "5") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 5: if(pushButton->text()== "6") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 6: if(pushButton->text()== "7") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 7: if(pushButton->text()== "8") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 8: if(pushButton->text()== "9") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 9: if(pushButton->text()== "10") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 10: if(pushButton->text()== "11") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 11: if(pushButton->text()== "12") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 12: if(pushButton->text()== "13") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 13: if(pushButton->text()== "14") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 14: if(pushButton->text()== "15") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 15: if(pushButton->text()== "16") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 16: if(pushButton->text()== "17") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 17: if(pushButton->text()== "18") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 18: if(pushButton->text()== "19") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 19: if(pushButton->text()== "20") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 20: if(pushButton->text()== "21") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 21: if(pushButton->text()== "22") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 22: if(pushButton->text()== "23") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 23: if(pushButton->text()== "24") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 24: if(pushButton->text()== "25") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
		}
		*/

		m_toolFrameTrackerNavigationGUI.gridLayout1->update();
		//RegistrationType::LandmarkTrackerPointType p;
		//m_LandmarksContainer.push_back(p);
		m_AcceptedLandmarksContainer.insert( std::pair<unsigned int,bool>(i,false) );
	}

	m_toolFrameTrackerNavigationGUI.clearFiducialsPushButton->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.saveFiducialsPushButton->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.modifyFiducialsPushButton->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.addImageFiducialsPushButton->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.addTrackerFiducialsPushButton->setEnabled(true);

	requestChangeSelectedFiducial();

	mutex.unlock();
}




//Write the image fiducials to file
void trackerNavigationPlugin::saveFiducialsInFile()
{
	igstkLogMacro2( m_Logger, DEBUG, "saveFiducialsInFile called...\n" )

	QString fileName = QFileDialog::getSaveFileName(NULL, "Save Fiducial Set as...", QDir::currentPath(), "Fiducial Set (*.igstk)");

	m_PlanFilename = fileName.toStdString();

	fiducialWriter = new igstk::FiducialsPlanIO;
	fiducialWriter->SetFileName( m_PlanFilename );
	fiducialWriter->SetFiducialsPlan(m_Plan);
	fiducialWriter->RequestWrite();
}







//Start setting tracker fiducials processing
void trackerNavigationPlugin::addTrackerFiducialsProcessing()
{
	igstkLogMacro2( m_Logger, DEBUG, "addTrackerFiducials called...\n" )

		m_toolFrameTrackerNavigationGUI.clearTrackerFiducialsPushButton->setEnabled(true);

		std::cout << "Set fiducial event start" << std::endl;
		mutex.lock();

		m_TPoint = new RegistrationType::LandmarkTrackerPointType();
		m_TPoint->SetElement(0,toolPosition->GetElement(0));
		m_TPoint->SetElement(1,toolPosition->GetElement(1));
		m_TPoint->SetElement(2,toolPosition->GetElement(2));
		m_LandmarksContainer.push_back(*m_TPoint);
		std::cout << "Fiducial event" <<
						   " x:"<< QString::number(toolPosition->GetElement(0)).toStdString()
						<< " y:"<< QString::number(toolPosition->GetElement(1)).toStdString()
						<< " z:"<< QString::number(toolPosition->GetElement(2)).toStdString()
						<< std::endl;


		addTrackerFiducials();
		setTrackerFiducialProcessing();


/*

		PointType point = TransformToPoint( m_PickingTransform );

		if(m_ImageSpatialObject->IsInside(point ))
		{
			int choice = m_ActualFiducialNumberTracker;

			m_TrackerFiducialPointVector[choice]->RequestSetTransformAndParent(m_PickingTransform, m_WorldReference );

			m_LandmarksContainer[choice] = point;


			//pushButton->setStyleSheet(m_BackgroundBlueStyle);


			switch(choice)
			{
				case 0: if(pushButton->text()== "1") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 1: if(pushButton->text()== "2") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 2: if(pushButton->text()== "3") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 3: if(pushButton->text()== "4") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 4: if(pushButton->text()== "5") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 5: if(pushButton->text()== "6") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 6: if(pushButton->text()== "7") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 7: if(pushButton->text()== "8") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 8: if(pushButton->text()== "9") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 9: if(pushButton->text()== "10") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 10: if(pushButton->text()== "11") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 11: if(pushButton->text()== "12") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 12: if(pushButton->text()== "13") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 13: if(pushButton->text()== "14") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 14: if(pushButton->text()== "15") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 15: if(pushButton->text()== "16") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 16: if(pushButton->text()== "17") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 17: if(pushButton->text()== "18") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 18: if(pushButton->text()== "19") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 19: if(pushButton->text()== "20") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 20: if(pushButton->text()== "21") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 21: if(pushButton->text()== "22") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 22: if(pushButton->text()== "23") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 23: if(pushButton->text()== "24") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
				case 24: if(pushButton->text()== "25") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			}


			IndexType index;
			m_ImageSpatialObject->TransformPhysicalPointToIndex(point, index);

			const double *data = point.GetVnlVector().data_block();



			m_AxialPlaneSpatialObject->RequestSetCursorPosition( data );
			m_SagittalPlaneSpatialObject->RequestSetCursorPosition( data );
			m_CoronalPlaneSpatialObject->RequestSetCursorPosition( data );
			m_CrossHair->RequestSetCursorPosition( data );

			resliceImage(index);
		}
		else
		{
			igstkLogMacro( DEBUG,  "Picked point outside image...\n" )
		}

*/

		m_toolFrameTrackerNavigationGUI.modifyTrackerFiducialsPushButton->setEnabled(true);
		m_toolFrameTrackerNavigationGUI.clearTrackerFiducialsPushButton->setEnabled(true);
		m_toolFrameTrackerNavigationGUI.finishImageAndTrackerFiducialsPushButton->setEnabled(true);




		mutex.unlock();



		std::cout << "Set fiducial event end" << std::endl;







		/*

	//TODO Initialize the fiducial points (done in the observer)
	//m_TrackerController->RequestGetNonReferenceToolList();

	for (int i=0; i<m_maxFiducialNumber; i++)
	{
		m_AxialFiducialRepresentationVector[i]->SetColor( 0.0, 0.0, 1.0);
		m_SagittalFiducialRepresentationVector[i]->SetColor( 0.0, 0.0, 1.0);
		m_CoronalFiducialRepresentationVector[i]->SetColor( 0.0, 0.0, 1.0);
		m_3DViewFiducialRepresentationVector[i]->SetColor( 0.0, 0.0, 1.0);
	}

	PointType empty;
	empty.Fill(0.0);
	for(unsigned int i = 0; i < m_Plan->m_FiducialPoints.size(); i++)
	{
		if(m_Plan->m_FiducialPoints[i] == empty)
			continue;

		pushButton->setStyleSheet(m_BackgroundBlueStyle);


		switch(i)
		{
			case 0: if(pushButton->text()== "1") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 1: if(pushButton->text()== "2") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 2: if(pushButton->text()== "3") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 3: if(pushButton->text()== "4") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 4: if(pushButton->text()== "5") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 5: if(pushButton->text()== "6") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 6: if(pushButton->text()== "7") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 7: if(pushButton->text()== "8") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 8: if(pushButton->text()== "9") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 9: if(pushButton->text()== "10") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 10: if(pushButton->text()== "11") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 11: if(pushButton->text()== "12") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 12: if(pushButton->text()== "13") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 13: if(pushButton->text()== "14") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 14: if(pushButton->text()== "15") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 15: if(pushButton->text()== "16") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 16: if(pushButton->text()== "17") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 17: if(pushButton->text()== "18") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 18: if(pushButton->text()== "19") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 19: if(pushButton->text()== "20") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 20: if(pushButton->text()== "21") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 21: if(pushButton->text()== "22") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 22: if(pushButton->text()== "23") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 23: if(pushButton->text()== "24") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
			case 24: if(pushButton->text()== "25") { pushButton->setStyleSheet(m_BackgroundBlueStyle); } break;
		}

	}
	m_toolFrameTrackerNavigationGUI.clearTrackerFiducialsPushButton->setEnabled(true);
	//m_GUI.FiducialOpTab->setShown(true);
	//m_GUI.tabWidget->setCurrentIndex(1);

	if (!m_TrackingEnabled)
	{
		m_TrackingEnabled = !m_TrackingEnabled;
		m_TrackerController->RequestStartTracking();
		m_toolFrameTrackerNavigationGUI.startTrackingPushButton->setText("Stop Tracking");
		m_toolFrameTrackerNavigationGUI.startTrackingPushButton->setChecked(true);
	}


	m_AxialPlaneSpatialObject->SetAutomaticMode( false );
	m_SagittalPlaneSpatialObject->SetAutomaticMode( false );
	m_CoronalPlaneSpatialObject->SetAutomaticMode( false );
	m_CrossHair->SetAutomaticMode( false );


	m_ViewMPR->RequestRemoveObject(m_ToolRepresentation);
	m_3DViewCrossHairRepresentation->SetColor(1,0,0);

	// reset the cameras in the different views
	m_ViewImageAxial->RequestResetCamera();
	m_ViewImageSagittal->RequestResetCamera();
	m_ViewImageCoronal->RequestResetCamera();
	m_ViewMPR->RequestResetCamera();

	m_ViewImageAxial->SetCameraZoomFactor(m_DefaultZoomImageAxial);
	m_ViewImageSagittal->SetCameraZoomFactor(m_DefaultZoomImageSagittal);
	m_ViewImageCoronal->SetCameraZoomFactor(m_DefaultZoomImageCoronal);
	m_ViewMPR->SetCameraZoomFactor(m_DefaultZoomImageMPR);

	// first reset the reference tool
	igstk::Transform identity;
	identity.SetToIdentity(igstk::TimeStamp::GetLongestPossibleTime());

	if ( m_ReferenceTool.IsNotNull() )
	{
		m_ReferenceTool->RequestDetachFromParent();
		m_ReferenceTool->RequestSetTransformAndParent(identity, m_WorldReference);
	}

	m_LandmarksContainer.clear();
	m_AcceptedLandmarksContainer.clear();

	for ( unsigned int i=0; i<m_Plan->m_FiducialPoints.size(); i++)
	{
		RegistrationType::LandmarkTrackerPointType p;
		m_LandmarksContainer.push_back(p);
		m_AcceptedLandmarksContainer.insert( std::pair<unsigned int,bool>(i,false) );
	}

	requestChangeSelectedFiducial();
	*/
}




/*
//Finish setting tracker fiducials processing
void trackerNavigationPlugin::endSetTrackerFiducialsProcessing()
{
	igstkLogMacro2( m_Logger, DEBUG, "endSetTrackerFiducialsProcessing called...\n" )

	AcceptedLandmarkPointContainerType::const_iterator iter;
	iter = m_AcceptedLandmarksContainer.begin();

	unsigned int numberOfAcceptedLandmarks= 0;

	for (;iter != m_AcceptedLandmarksContainer.end(); iter++)
	{
		if (iter->second)
			numberOfAcceptedLandmarks++;
	}

	igstkLogMacro2( m_Logger, DEBUG, "numberOfAcceptedLandmarks " << numberOfAcceptedLandmarks << "\n" )

	if (numberOfAcceptedLandmarks >= 3)
	{
		igstkLogMacro2( m_Logger, DEBUG, "Success numberOfAcceptedLandmarks >= 3 \n" )
	}
	else
	{
		igstkLogMacro2( m_Logger, DEBUG, "Failure numberOfAcceptedLandmarks <= 3 \n" )
	}

}
*/



void trackerNavigationPlugin::finishImageAndTrackerFiducialsProcessing()
{

	igstkLogMacro2( m_Logger, DEBUG, "finishImageAndTrackerFiducialsProcessing called...\n" )


		cout<<"IMG SIZE= " << m_maxFiducials <<endl;
		cout<<"TRA SIZE= " << m_maxFiducialsTracker<<endl;

	if(m_maxFiducials <= 3 && m_maxFiducialsTracker <= 3)
	{

		m_messageDialog->warning(NULL, "Warning", "The number of image and patient fiducials should be at least 4 !", QMessageBox::Ok);

	}
	else
	{
		m_toolFrameTrackerNavigationGUI.startRegistrationNavigationPushButton->setEnabled(true);
	}

	if(m_maxFiducials != m_maxFiducialsTracker)
	{
		m_messageDialog->warning(NULL, "Warning", "The number of image and patient fiducials are not equal!", QMessageBox::Ok);
	}
	else
	{
		m_toolFrameTrackerNavigationGUI.startRegistrationNavigationPushButton->setEnabled(true);
	}

	m_toolFrameTrackerNavigationGUI.finishImageAndTrackerFiducialsPushButton->setEnabled(false);


}




//Registers the tracker to the working image
void trackerNavigationPlugin::startRegistrationNavigationProcessing()
{

	igstkLogMacro2( m_Logger, DEBUG, "startRegistrationNavigationProcessing called...\n" )


	registration  = RegistrationType::New();
	registration->RequestResetRegistration();


	for(unsigned int i=0; i< m_LandmarksContainer.size(); i++)
	{
		// use only those fiducial points that were accepted by the user
		//if ( m_AcceptedLandmarksContainer[i] )
		//{
		printf("Fiducial: %f %f %f\n",m_Plan->m_FiducialPoints[i].GetElement(0),m_Plan->m_FiducialPoints[i].GetElement(1),m_Plan->m_FiducialPoints[i].GetElement(2));
		printf("Landmark: %f %f %f\n",m_LandmarksContainer[i].GetElement(0),m_LandmarksContainer[i].GetElement(1),m_LandmarksContainer[i].GetElement(2));

		registration->RequestAddImageLandmarkPoint(m_Plan->m_FiducialPoints[i] );
		registration->RequestAddTrackerLandmarkPoint(m_LandmarksContainer[i] );

		//}
	}

	registration->RequestComputeTransform();

	lrtcb = igstk::TransformObserver::New();
	lrtcb->ObserveTransformEventsFrom(registration);
	lrtcb->Clear();

	registration->RequestGetTransformFromTrackerToImage();

	if(lrtcb->GotTransform())
	{


		RegistrationErrorObserver::Pointer lRmscb =	RegistrationErrorObserver::New();

		registration->AddObserver( igstk::DoubleTypeEvent(), lRmscb);
		registration->RequestGetRMSError();

		if(lRmscb->GotRegistrationError())
		{
			igstkLogMacro2(m_Logger, DEBUG, "Tracker to Image Registration Error " << lRmscb->GetRegistrationError() << "\n")
			m_TrackerRMS = lRmscb->GetRegistrationError();


			QString rmsValue = QString::number(m_TrackerRMS, 'f', 2);

				QMessageBox::StandardButton reply;
			    reply = QMessageBox::question(NULL, "Registration Result", "Accept registration with " + rmsValue + " RMS ?",
			                                  QMessageBox::Yes | QMessageBox::No );
			     if (reply == QMessageBox::Yes)
			     {

				 }
			     else if (reply == QMessageBox::No)
				 {

				 }

		}
		else
		{
			igstkLogMacro2(m_Logger, DEBUG, "Tracker Registration Error: could not retrieve RMS error \n")
			return;
		}

		// set new transformation
		m_RegistrationTransform = lrtcb->GetTransform();

		//m_toolFrameTrackerNavigationGUI.setFiducialsOnImageFrame->setEnabled(false);
		m_toolFrameTrackerNavigationGUI.setFiducialsOnImageReadSaveFrame->setEnabled(false);
		//m_toolFrameTrackerNavigationGUI.setFiducialsOnPatientFrame->setEnabled(false);

		m_toolFrameTrackerNavigationGUI.finishRegistrationNavigationPushButton->setEnabled(true);
		m_toolFrameTrackerNavigationGUI.startRegistrationNavigationPushButton->setEnabled(false);
		m_toolFrameTrackerNavigationGUI.modifyFiducialsPushButton->setEnabled(false);
		m_toolFrameTrackerNavigationGUI.clearFiducialsPushButton->setEnabled(false);
		m_toolFrameTrackerNavigationGUI.modifyTrackerFiducialsPushButton->setEnabled(false);
		m_toolFrameTrackerNavigationGUI.clearTrackerFiducialsPushButton->setEnabled(false);

/*
		if (m_ReferenceTool.IsNotNull())
		{
			m_ReferenceTool->RequestDetachFromParent();
			m_ReferenceTool->RequestSetTransformAndParent(m_RegistrationTransform, m_WorldReference);
		}
*/



		mutex.lock();
		isRegistered=true;

		mutex.unlock();
	}
	else
	{
		igstkLogMacro2(m_Logger, DEBUG, "Tracker Registration Error \n")
	}


	return;
}



//End the registration and navigation
void trackerNavigationPlugin::finishRegistrationNavigationProcessing()
{

	igstkLogMacro2( m_Logger, DEBUG, "finishRegistrationNavigationProcessing called...\n" )

	m_toolFrameTrackerNavigationGUI.setFiducialsOnImageFrame->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.setFiducialsOnImageReadSaveFrame->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.setFiducialsOnPatientFrame->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.finishImageAndTrackerFiducialsPushButton->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.finishRegistrationNavigationPushButton->setEnabled(false);
	m_toolFrameTrackerNavigationGUI.clearFiducialsPushButton->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.clearTrackerFiducialsPushButton->setEnabled(true);
	m_toolFrameTrackerNavigationGUI.addImageFiducialsPushButton->setEnabled(true);


}


/*

//Accept current tracker fiducial processing
void trackerNavigationPlugin::acceptTrackerFiducialProcessing()
{
	igstkLogMacro2( m_Logger, DEBUG, "acceptTrackerFiducialProcessing called...\n");

	typedef igstk::TransformObserver ObserverType;
	ObserverType::Pointer transformObserver = ObserverType::New();
	transformObserver->ObserveTransformEventsFrom( m_TrackerTool );
	transformObserver->Clear();
	m_TrackerTool->RequestComputeTransformTo( m_WorldReference );

	if ( !transformObserver->GotTransform() )
	{
		igstkLogMacro2( m_Logger, DEBUG, "AcceptTrackerFiducialProcessing: No transform received!\n");
		return;
	}
	acceptFiducial(m_ActualFiducialNumber, TransformToPoint( transformObserver->GetTransform() ));


	 (cp) moved this code to AcceptFiducial

	int n = m_ActualFiducialNumber;
	//int m = MAX_FIDUCIAL_NR;

	m_LandmarksContainer[n] = TransformToPoint( transformObserver->GetTransform() );
	m_AcceptedLandmarksContainer[n] = true;

	m_AxialFiducialRepresentationVector[n]->SetColor( 0.0, 1.0, 0.0 );
	m_SagittalFiducialRepresentationVector[n]->SetColor( 0.0, 1.0, 0.0 );
	m_CoronalFiducialRepresentationVector[n]->SetColor( 0.0, 1.0, 0.0 );
	m_3DViewFiducialRepresentationVector[n]->SetColor( 0.0, 1.0, 0.0 );

	this->RequestChangeSelectedFiducial();

	AcceptedLandmarkPointContainerType::const_iterator iter;
	iter = m_AcceptedLandmarksContainer.begin();

	unsigned int numberOfAcceptedLandmarks = 0;
	for (;iter != m_AcceptedLandmarksContainer.end(); iter++)
	{
		if (iter->second)
			numberOfAcceptedLandmarks++;
	}

	if ( numberOfAcceptedLandmarks >= 3 )
	{
		m_GUI.FinishFiducialPushButton->setEnabled(true);
	}

	switch(n)
	{
		// MDD: PushButtonCode
		case 0: m_GUI.pushButton_1->setStyleSheet(m_BackgroundGreenStyle);break;
		case 1: m_GUI.pushButton_2->setStyleSheet(m_BackgroundGreenStyle);break;
		case 2: m_GUI.pushButton_3->setStyleSheet(m_BackgroundGreenStyle);break;
		case 3: m_GUI.pushButton_4->setStyleSheet(m_BackgroundGreenStyle);break;
		case 4: m_GUI.pushButton_5->setStyleSheet(m_BackgroundGreenStyle);break;
		case 5: m_GUI.pushButton_6->setStyleSheet(m_BackgroundGreenStyle);break;
		case 6: m_GUI.pushButton_7->setStyleSheet(m_BackgroundGreenStyle);break;
		case 7: m_GUI.pushButton_8->setStyleSheet(m_BackgroundGreenStyle);break;
		case 8: m_GUI.pushButton_9->setStyleSheet(m_BackgroundGreenStyle);break;
		case 9: m_GUI.pushButton_10->setStyleSheet(m_BackgroundGreenStyle);break;
	}

}
*/
/*
// this code was in acceptTrackerFiducialProcessing
// moved here for re-using in automated mapping
void trackerNavigationPlugin::acceptFiducial(int fiducialID, PointType point)
{
	std::cerr << "acceptFiducial(" << fiducialID << ", " << point << ") called...\n";

	m_LandmarksContainer[fiducialID] = point;
	m_AcceptedLandmarksContainer[fiducialID] = true;

	m_AxialFiducialRepresentationVector[fiducialID]->SetColor( 0.0, 1.0, 0.0 );
	m_SagittalFiducialRepresentationVector[fiducialID]->SetColor( 0.0, 1.0, 0.0 );
	m_CoronalFiducialRepresentationVector[fiducialID]->SetColor( 0.0, 1.0, 0.0 );
	m_3DViewFiducialRepresentationVector[fiducialID]->SetColor( 0.0, 1.0, 0.0 );

	this->requestChangeSelectedFiducial();

	AcceptedLandmarkPointContainerType::const_iterator iter;
	iter = m_AcceptedLandmarksContainer.begin();

	unsigned int numberOfAcceptedLandmarks = 0;

	for (;iter != m_AcceptedLandmarksContainer.end(); iter++)
	{
		if (iter->second)
			numberOfAcceptedLandmarks++;
	}

	if ( numberOfAcceptedLandmarks >= 3 )
	{
		m_toolFrameTrackerNavigationGUI.finishFiducialsPushButton->setEnabled(true);
	}

	pushButton->setStyleSheet(m_BackgroundGreenStyle);


	switch(fiducialID)
	{
		case 0: if(pushButton->text()== "1") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 1: if(pushButton->text()== "2") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 2: if(pushButton->text()== "3") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 3: if(pushButton->text()== "4") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 4: if(pushButton->text()== "5") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 5: if(pushButton->text()== "6") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 6: if(pushButton->text()== "7") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 7: if(pushButton->text()== "8") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 8: if(pushButton->text()== "9") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 9: if(pushButton->text()== "10") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 10: if(pushButton->text()== "11") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 11: if(pushButton->text()== "12") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 12: if(pushButton->text()== "13") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 13: if(pushButton->text()== "14") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 14: if(pushButton->text()== "15") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 15: if(pushButton->text()== "16") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 16: if(pushButton->text()== "17") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 17: if(pushButton->text()== "18") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 18: if(pushButton->text()== "19") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 19: if(pushButton->text()== "20") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 20: if(pushButton->text()== "21") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 21: if(pushButton->text()== "22") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 22: if(pushButton->text()== "23") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 23: if(pushButton->text()== "24") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
		case 24: if(pushButton->text()== "25") { pushButton->setStyleSheet(m_BackgroundGreenStyle); } break;
	}


}
*/



//Begin new fiducial plan
void trackerNavigationPlugin::newFiducialPlan()
{
	igstkLogMacro2( m_Logger, DEBUG, "newFiducialPlan called...\n" )

	m_toolFrameTrackerNavigationGUI.fiducialSetNameLineEdit->setText("New Fiducial Set");
	//m_GUI.FiducialSetNameEdit2->setText("New Fiducial Set");
	m_Plan = new igstk::FiducialsPlan;

	PointType empty;
	empty.Fill(0.0);

	for(int i=0; i < m_maxFiducialNumber;i++)
	{
		m_Plan->m_FiducialPoints.push_back( empty );
	}

	m_LandmarksContainer.clear();
	m_AcceptedLandmarksContainer.clear();


	//pushButton->setStyleSheet(m_BackgroundWhiteStyle);

	deleteAllImageFiducials();

/*
	for(int i=0; i < m_maxFiducialNumber; i++)
	{
		RegistrationType::LandmarkTrackerPointType p;
		m_LandmarksContainer.push_back(p);
		m_AcceptedLandmarksContainer.insert( std::pair<unsigned int,bool>(i,false) );
	}
*/
	requestChangeSelectedFiducial();
}





void trackerNavigationPlugin::refreshImageViews()
{
	igstkLogMacro2(m_Logger, DEBUG,	"refreshImageViews start\n");



	m_mainFrameNavigationViewGUI.navigationViewDisplayAxial->repaint();
	m_mainFrameNavigationViewGUI.navigationViewDisplaySagittal->repaint();
	m_mainFrameNavigationViewGUI.navigationViewDisplayCoronal->repaint();
	m_mainFrameNavigationViewGUI.navigationViewDisplayMPR->repaint();




	igstkLogMacro2(m_Logger, DEBUG,	"refreshImageViews called\n");

}





void trackerNavigationPlugin::resetImageCameraSettings()
{
	m_ViewImageAxial->RequestResetCamera();
	m_ViewImageSagittal->RequestResetCamera();
	m_ViewImageCoronal->RequestResetCamera();
	m_ViewMPR->RequestResetCamera();

	m_ViewImageAxial->SetCameraZoomFactor(m_DefaultZoomImageAxial);
	m_ViewImageSagittal->SetCameraZoomFactor(m_DefaultZoomImageSagittal);
	m_ViewImageCoronal->SetCameraZoomFactor(m_DefaultZoomImageCoronal);

	//igstkLogMacro2(m_Logger, DEBUG,	"resetImageCameraSettings called\n");
}




/**
 * Request start/stop tracking


void trackerNavigationPlugin::requestToggleTracking()
{
	igstkLogMacro2(m_Logger, DEBUG, " requestToggleTracking called...\n" )

	m_TrackingEnabled = !m_TrackingEnabled;



	if (m_TrackingEnabled)
	{
		//m_TrackerController->RequestStartTracking();
		m_toolFrameTrackerNavigationGUI.startTrackingPushButton->setText("Stop Tracking");
		m_toolFrameTrackerNavigationGUI.startTrackingPushButton->setEnabled(true);
	}
	else
	{
		//m_TrackerController->RequestStopTracking();
		m_toolFrameTrackerNavigationGUI.startTrackingPushButton->setText("Start Tracking");
	}

	//check that start was successful
	if( m_TrackerControllerObserver->Error() )
	{
		std::string errorMessage;
		m_TrackerControllerObserver->GetErrorMessage( errorMessage );
		m_TrackerControllerObserver->ClearError();
		igstkLogMacro2( m_Logger, DEBUG, "Tracker start error\n" )

		//handleError("Tracker start error\n");
		//m_StateMachine.PushInput( m_FailureInput );
		//m_StateMachine.ProcessInputs();
		return;

	}
}

*/

/**
 * Request start/stop navigation
 */
/*
void trackerNavigationPlugin::requestToggleNavigate()
{
	igstkLogMacro2( m_Logger, DEBUG, "RequestToggleNavigate called...\n" )

	// MDD: This flag should be set after RequestStartTracking or
	// RequestStopTracking have executed successfully.
	m_NavigationEnabled = !m_NavigationEnabled;

	if (m_NavigationEnabled)
	{
		startTrackingProcessing();
	}
	else
	{
		stopTrackingProcessing();
	}
}
*/
/*
//Starts tracking provided it is initialized and connected to the communication port
void trackerNavigationPlugin::startTrackingProcessing()
{
	igstkLogMacro2( m_Logger, DEBUG, "startTrackingProcessing called...\n" )

	if (!m_TrackerConfiguration)
	{
		std::string errorMessage;
		igstkLogMacro2( m_Logger, DEBUG, "Tracker Starting error\n" )

		//handleError("Tracker Starting error\n");
		//m_StateMachine.PushInput( m_FailureInput );
		//m_StateMachine.ProcessInputs();
		return;
	}

	m_TrackerController->RequestStartTracking();

	//check that start was successful
	if( m_TrackerControllerObserver->Error() )
	{
		std::string errorMessage;
		m_TrackerControllerObserver->GetErrorMessage( errorMessage );
		m_TrackerControllerObserver->ClearError();
		igstkLogMacro2( m_Logger, DEBUG, "Tracker start error\n" )

		//handleError("Tracker start error\n");
		//m_StateMachine.PushInput( m_FailureInput );
		//m_StateMachine.ProcessInputs();
		return;
	}

	//m_StateMachine.PushInput( m_SuccessInput );
	//m_StateMachine.ProcessInputs();
	return;
}
*/
/*
//Stops tracking but keeps the tracker connected to the communication port
void trackerNavigationPlugin::stopTrackingProcessing()
{
	igstkLogMacro2( m_Logger, DEBUG, "stopTrackingProcessing called...\n" )

	m_TrackerController->RequestStopTracking( );

	//check that stop was successful
	if( m_TrackerControllerObserver->Error() )
	{
		std::string errorMessage;
		m_TrackerControllerObserver->GetErrorMessage( errorMessage );
		m_TrackerControllerObserver->ClearError();
		igstkLogMacro2( m_Logger, DEBUG, "Tracker stop error\n" )

		//handleError("Tracker stop error\n");
		//m_StateMachine.PushInput( m_FailureInput );
		//m_StateMachine.ProcessInputs();
		return;
	}

	//m_StateMachine.PushInput( m_SuccessInput );
	//m_StateMachine.ProcessInputs();
	return;
}
*/




Q_EXPORT_PLUGIN2(at_voxelmaster_trackerNavigationPlugin, trackerNavigationPlugin)


