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


#include "loadDicomPlugin.h"

#include <iostream>
#include <fstream>

#include <QtPlugin>

#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>

#include <unistd.h>


#define VIEW_2D_REFRESH_RATE 15
#define VIEW_3D_REFRESH_RATE 15



loadDicomPlugin* loadDicomPlugin::instance = 0;

loadDicomPlugin::loadDicomPlugin()
  : context(0)
{
	std::cout << "Load Dicom  plugin constructor" << std::endl;
}

loadDicomPlugin::~loadDicomPlugin()
{
  
}


void loadDicomPlugin::start(ctkPluginContext* context)
{
	std::cout << "Load Dicom plugin start" << std::endl;

	instance = this;
	this->context = context;


	QTimer *pulseTimer = new QTimer();
	connect(pulseTimer, SIGNAL(timeout()), this, SLOT(pulseTimerEvent()));
	pulseTimer->start(25);

	this->createWidgetsAndSignalSlotActions();
	this->startDicomDataService();

	m_ImageLoadedImage= false;

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

	m_toolFrameLoadDicomGUI.mainViewSliderWindowLevel->setMinimum(m_MinWindowLevelValue);
	m_toolFrameLoadDicomGUI.mainViewSliderWindowLevel->setMaximum(m_MaxWindowLevelValue);

	m_toolFrameLoadDicomGUI.mainViewSliderWindowWidht->setMinimum(m_MinWindowWidhtValue);
	m_toolFrameLoadDicomGUI.mainViewSliderWindowWidht->setMaximum(m_MaxWindowWidhtValue);


	//connect(m_toolFrameLoadDicomGUI.loadImageFilePushButton, SIGNAL(clicked()), this, SLOT(updateImageData()));
    connect(m_toolFrameLoadDicomGUI.loadImageFilePushButton, SIGNAL(clicked()),this, SLOT(readCTDataset()));

	connect(m_mainFrameDicomViewGUI.scrollBarAxial, SIGNAL(valueChanged(int)), this, SLOT(resliceImageViews(int)));
	connect(m_mainFrameDicomViewGUI.scrollBarSagittal, SIGNAL(valueChanged(int)), this, SLOT(resliceImageViews(int)));
	connect(m_mainFrameDicomViewGUI.scrollBarCoronal, SIGNAL(valueChanged(int)), this, SLOT(resliceImageViews(int)));

	connect(m_toolFrameLoadDicomGUI.mainViewSliderWindowLevel, SIGNAL(valueChanged(int)), this, SLOT(changeImageWindowing(int)));
	connect(m_toolFrameLoadDicomGUI.mainViewSliderWindowWidht, SIGNAL(valueChanged(int)), this, SLOT(changeImageWindowing(int)));

	m_toolFrameLoadDicomGUI.loadDicomProgressBar->setVisible(false);



	m_ImageWorldReference = AxesObjectImageType::New();
	std::cout<<"initializeWorldReference OK"<<std::endl;

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


	this->initializeQuadrantView();


}


void loadDicomPlugin::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)
}

loadDicomPlugin* loadDicomPlugin::getInstance()
{
	return instance;
}



ctkPluginContext* loadDicomPlugin::getPluginContext() const
{
	std::cout << "Load Dicom context" << std::endl;
	return context;
}



void loadDicomPlugin::startDicomDataService()
{
	ctkDictionary properties;
	properties["dicomDataServiceEventID"]=0;

	dicomDataInterfaceInstance = new dicomDataInterface();
	dicomDataServiceReference = context->registerService<dicomDataInterface>(dicomDataInterfaceInstance, properties);

	std::cout << "start dicom data service" << std::endl;
}


void loadDicomPlugin::pulseTimerEvent()
{
	igstk::PulseGenerator::CheckTimeouts();
}


void loadDicomPlugin::updateImageData(CTImageSpatialObjectType::Pointer dicomSpatialObject)
{
	std::cout << "Dicom Plugin updates image data..." << std::endl;


		ctkDictionary properties;

		bool* ok= new bool();

		dicomDataServiceReference.getReference();
		dicomDataServiceReference.getReference().getProperty("dicomDataServiceEventID");

		double eventID = dicomDataServiceReference.getReference().getProperty("dicomDataServiceEventID").toDouble(ok);

		properties["dicomDataServiceEventID"] = eventID+1;



		dicomDataInterfaceInstance->dicomSpatialObject= dicomSpatialObject;

		dicomDataServiceReference.setProperties(properties);


		std::cout << "Update dicom data: EventID " << dicomDataServiceReference.getReference().getProperty("dicomDataServiceEventID").toString().toStdString()<< std::endl;
		sleep(1);


	std::cout << "Dicom Plugin updates image data..." << std::endl;
}



void loadDicomPlugin::createWidgetsAndSignalSlotActions()
{
	loadDicomInterfaceInstance = new loadDicomInterface();

	toolFrameLoadDicomWidget = new QWidget();
	mainFrameVisualizationDicomViewWidget = new QWidget();

	m_toolFrameLoadDicomGUI.setupUi(toolFrameLoadDicomWidget);
	m_mainFrameDicomViewGUI.setupUi(mainFrameVisualizationDicomViewWidget);

	loadDicomInterfaceInstance->toolFrameLoadDicomInterfaceWidget= toolFrameLoadDicomWidget;
	loadDicomInterfaceInstance->toolFrameLoadDicomInterfaceWidgetString="1    Load DICOM";

	loadDicomInterfaceInstance->mainFrameVisualizationDicomViewInterfaceWidget= mainFrameVisualizationDicomViewWidget;
	loadDicomInterfaceInstance->mainFrameVisualizationDicomViewInterfaceWidgetString="DICOM View";


	ctkDictionary properties;
	properties["message"] = "toolFrameLoadDicomWidget";

	context->registerService<loadDicomInterface>(loadDicomInterfaceInstance, properties);

}


/*
QWidget* loadDicomPlugin::getLeftToolFrame()
{

	widget = new QWidget();
	widget->resize(250, 150);
	std::cout << "get widget 2"  << std::endl;

	QPushButton *button1 = new QPushButton("One");
	QPushButton *button2 = new QPushButton("Two");
	QPushButton *button3 = new QPushButton("Three");
	QPushButton *button4 = new QPushButton("Four");
	QPushButton *button5 = new QPushButton("Five");

	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(button1);
	layout->addWidget(button2);
	layout->addWidget(button3);
	layout->addWidget(button4);
	layout->addWidget(button5);

	widget->setLayout(layout);

////////////////////////////////////////////////////////////////

	std::cout << "get getLeftToolFrame 0"  << std::endl;
	QWidget* widget = new QWidget();
	std::cout << "get getLeftToolFrame 1"  << std::endl;
	Ui::toolFrameLoadDicomGUI			m_toolFrameLoadDicomGUI;
	m_toolFrameLoadDicomGUI->setupUi(widget);
	std::cout << "get getLeftToolFrame 2"  << std::endl;
	return widget;
}

*/




void loadDicomPlugin::readCTDataset() //std::string openFileWindowFilePath
{

	//logger object created for logging mouse activities
	m_Logger = LoggerType::New();
	m_LogOutput = LogOutputType::New();
	m_Logger->AddLogOutput(m_LogOutput);
	m_Logger->SetPriorityLevel(itk::Logger::DEBUG);
	m_LogOutput->SetStream(std::cout);

	igstkLogMacro2(m_Logger, DEBUG,	"LoadImageProcessing called\n");


	QFileDialog::Options options = QFileDialog::DontResolveSymlinks; options |= QFileDialog::DontUseNativeDialog;
	QString directory = QFileDialog::getExistingDirectory(toolFrameLoadDicomWidget, "Select DICOM directory!", "Select DICOM directory!", options);

	if (directory.isEmpty())
	{
		QErrorMessage *errorMessageDialog = new QErrorMessage(toolFrameLoadDicomWidget);
		errorMessageDialog->showMessage("No directory was selected\n");
		return;
	}


	m_CTImageDirectory= directory.toStdString();

	igstkLogMacro2(m_Logger, DEBUG,	"Set selected image-directory to readImageFiles\n");

	// Setup image reader
	m_CTImageReader = CTImageReaderType::New();

	// Build itk progress command to assess image load progress
	itk::SmartPointer<ProgressCommandType>            progressCommand;
	progressCommand = ProgressCommandType::New();

	// Set the callback to the itk progress command
	progressCommand->SetCallbackFunction(this, &loadDicomPlugin::onITKProgressEvent);

	// Provide a progress observer to the image reader
	m_CTImageReader->RequestSetProgressCallback(progressCommand);

	// Add observer for invalid directory
	DICOMImageReaderInvalidDirectoryNameErrorObserver::Pointer didcb= DICOMImageReaderInvalidDirectoryNameErrorObserver::New();

	m_CTImageReader->AddObserver(igstk::DICOMImageDirectoryIsNotDirectoryErrorEvent(), didcb);

	// Add observer for a non-existing directory
	DICOMImageReaderNonExistingDirectoryErrorObserver::Pointer dndcb= DICOMImageReaderNonExistingDirectoryErrorObserver::New();

	m_CTImageReader->AddObserver(igstk::DICOMImageDirectoryDoesNotExistErrorEvent(), dndcb);

	// Add observer for a an empty directory name (null string)
	DICOMImageReaderEmptyDirectoryErrorObserver::Pointer decb= DICOMImageReaderEmptyDirectoryErrorObserver::New();
	m_CTImageReader->AddObserver(igstk::DICOMImageDirectoryEmptyErrorEvent(), decb);

	// Add observer for a directory which does not have enough number of files
	DICOMImageDirectoryNameDoesNotHaveEnoughFilesErrorObserver::Pointer ddhefcb= DICOMImageDirectoryNameDoesNotHaveEnoughFilesErrorObserver::New();

	m_CTImageReader->AddObserver(igstk::DICOMImageDirectoryDoesNotHaveEnoughFilesErrorEvent(), ddhefcb);

	// Add observer for a directory containing non-DICOM files
	DICOMImageDirectoryDoesNotContainValidDICOMSeriesErrorObserver::Pointer disgcb=	DICOMImageDirectoryDoesNotContainValidDICOMSeriesErrorObserver::New();

	m_CTImageReader->AddObserver(igstk::DICOMImageSeriesFileNamesGeneratingErrorEvent(), disgcb);

	// Add observer for reading invalid/corrupted dicom files
	DICOMImageInvalidErrorObserver::Pointer dircb= DICOMImageInvalidErrorObserver::New();

	m_CTImageReader->AddObserver(igstk::DICOMImageReadingErrorEvent(), dircb);

	// Set directory
	m_CTImageReader->RequestSetDirectory(m_CTImageDirectory);

	if(didcb->GotDICOMImageReaderInvalidDirectoryNameError())
	{
		igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::DICOMImageReaderInvalidDirectoryNameError\n");
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

		igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::DICOMImageDirectoryNameDoesNotHaveEnoughFilesError\n");
		return;
	}

	if(disgcb->GotDICOMImageDirectoryDoesNotContainValidDICOMSeriesError())
	{
		m_messageDialog->warning(NULL, "Warning", "The selected directory does not contain valid DICOM files!", QMessageBox::Ok);

		igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::DICOMImageDirectoryDoesNotContainValidDICOMSeriesError\n");
		return;
	}

	// Read CT Image
	m_CTImageReader->RequestReadImage();




	m_CTImageObserver = CTImageObserver::New();
	m_CTImageReader->AddObserver(CTImageReaderType::ImageModifiedEvent(), m_CTImageObserver);

	m_CTImageReader->RequestGetImage();

	if(!m_CTImageObserver->GotCTImage())
	{
		m_messageDialog->warning(NULL, "Warning", "Could not read DICOM dataset!", QMessageBox::Ok);

		m_CTImageObserver->RemoveAllObservers();
		m_CTImageObserver = NULL;

		igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::Could not read DICOM images\n");
		return;
	}

	if (m_CTImageObserver.IsNotNull())
	{
		m_CTImageSpatialObject = m_CTImageObserver->GetCTImage();


		igstk::Transform identity;
		identity.SetToIdentity( igstk::TimeStamp::GetLongestPossibleTime() );

		// Set transform and parent to the image spatial object
		m_CTImageSpatialObject->RequestSetTransformAndParent(identity, m_ImageWorldReference);


		// Create a directory for logging files
		m_logDirectoryName= m_CTImageReader->GetPatientName()+"_Log_Files";
		m_logDirectoryNameDate= itksys::SystemTools::GetCurrentDateTime("%d%m%Y_%H%M%S");

		m_logDirectoryNameCurrentlyUsed=(m_logDirectoryName + "/" + m_logDirectoryNameDate);


		displayCTDataSet();

		resetImageCameraSettings();
		refreshImageViews();

		readCTDatasetForDicomInterace();

		igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::displayCTDataSet called\n");
	}


}


void loadDicomPlugin::readCTDatasetForDicomInterace() //std::string openFileWindowFilePath
{

	//logger object created for logging mouse activities
	m_Logger = LoggerType::New();
	m_LogOutput = LogOutputType::New();
	m_Logger->AddLogOutput(m_LogOutput);
	m_Logger->SetPriorityLevel(itk::Logger::DEBUG);
	m_LogOutput->SetStream(std::cout);

	CTImageReaderType::Pointer m_CTImageReaderForDicomInterface = CTImageReaderType::New();

	// Set directory
	m_CTImageReaderForDicomInterface->RequestSetDirectory(m_CTImageDirectory);



	// Read CT Image
	m_CTImageReaderForDicomInterface->RequestReadImage();

	CTImageObserver::Pointer m_CTImageObserverForDicomInterface = CTImageObserver::New();
	m_CTImageReaderForDicomInterface->AddObserver(CTImageReaderType::ImageModifiedEvent(), m_CTImageObserverForDicomInterface);

	m_CTImageReaderForDicomInterface->RequestGetImage();

	if(!m_CTImageObserverForDicomInterface->GotCTImage())
	{
		m_messageDialog->warning(NULL, "Warning", "Could not read DICOM dataset!", QMessageBox::Ok);

		m_CTImageObserverForDicomInterface->RemoveAllObservers();
		m_CTImageObserverForDicomInterface = NULL;

		igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::Could not read DICOM images\n");
		return;
	}

	if (m_CTImageObserverForDicomInterface.IsNotNull())
	{
		CTImageSpatialObjectType::Pointer dicomSpatialObject = CTImageSpatialObjectType::New();
		dicomSpatialObject = m_CTImageObserverForDicomInterface->GetCTImage();
		m_CTImageObserverForDicomInterface->RemoveAllObservers();
		m_CTImageObserverForDicomInterface = NULL;
		m_CTImageReaderForDicomInterface = NULL;


		updateImageData(dicomSpatialObject);

		igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::displayCTDataSet called\n");
	}


}





// Show progress bar while loading image dataset */
void loadDicomPlugin::onITKProgressEvent(itk::Object *source, const itk::EventObject &)
{
	// Get the value of the progress
	float progress = reinterpret_cast<itk::ProcessObject *>(source)->GetProgress();

	if (progress >= 0.0 && progress < 1.0)
	{
		m_toolFrameLoadDicomGUI.loadDicomProgressBar->setVisible(true);

		// Update the progress bar and value
		m_toolFrameLoadDicomGUI.loadDicomProgressBar->setValue( 100 * progress );
	}
	else if(progress >= 1.0)
	{
		m_toolFrameLoadDicomGUI.loadDicomProgressBar->setVisible(false);
	}
}





/** Initalize Axial, Sagittal, Coronal and MPR Views */
void loadDicomPlugin::initializeQuadrantView()
{

	m_ViewImageAxial= View2DType::New();
	m_ViewImageSagittal= View2DType::New();
	m_ViewImageCoronal= View2DType::New();
	m_ViewMPR= View3DType::New();

	m_mainFrameDicomViewGUI.dicomViewDisplayAxial->RequestSetView(m_ViewImageAxial);
	m_mainFrameDicomViewGUI.dicomViewDisplaySagittal->RequestSetView(m_ViewImageSagittal);
	m_mainFrameDicomViewGUI.dicomViewDisplayCoronal->RequestSetView(m_ViewImageCoronal);
	m_mainFrameDicomViewGUI.dicomViewDisplayMPR->RequestSetView(m_ViewMPR);

	m_mainFrameDicomViewGUI.dicomViewDisplayAxial->RequestEnableInteractions();
	m_mainFrameDicomViewGUI.dicomViewDisplaySagittal->RequestEnableInteractions();
	m_mainFrameDicomViewGUI.dicomViewDisplayCoronal->RequestEnableInteractions();
	m_mainFrameDicomViewGUI.dicomViewDisplayMPR->RequestEnableInteractions();

	m_ViewImageAxial->SetRendererBackgroundColor(0, 0, 0);
	m_ViewImageSagittal->SetRendererBackgroundColor(0, 0, 0);
	m_ViewImageCoronal->SetRendererBackgroundColor(0, 0, 0);
	m_ViewMPR->SetRendererBackgroundColor(0, 0, 0);


	std::cout<<"initializeQuadrantView OK"<<std::endl;

}





void loadDicomPlugin::displayCTDataSet()
{


	// Once a dataset is loaded remove all objects from all views
	if(m_ImageLoadedImage)
	{
		m_ViewImageAxial->RequestRemoveObject(m_AxialPlaneRepresentation);
		m_ViewImageSagittal->RequestRemoveObject(m_SagittalPlaneRepresentation);
		m_ViewImageCoronal->RequestRemoveObject(m_CoronalPlaneRepresentation);

		m_ViewMPR->RequestRemoveObject(m_AxialPlaneRepresentation2);
		m_ViewMPR->RequestRemoveObject(m_SagittalPlaneRepresentation2);
		m_ViewMPR->RequestRemoveObject(m_CoronalPlaneRepresentation2);

		m_ViewImageAxial->RequestRemoveObject( m_AxialCrossHairRepresentation );
		m_ViewImageSagittal->RequestRemoveObject( m_SagittalCrossHairRepresentation );
		m_ViewImageCoronal->RequestRemoveObject( m_CoronalCrossHairRepresentation );
		m_ViewMPR->RequestRemoveObject( m_3DViewCrossHairRepresentation );

	}

	std::cout<<"TIME STAMP: "<< m_CTImageSpatialObject->GetMTime()<<std::endl;

	igstk::Transform identity;
	identity.SetToIdentity(igstk::TimeStamp::GetLongestPossibleTime());

	// Create reslice plane spatial object for axial view
	m_AxialImagePlaneSpatialObject = ReslicerImagePlaneType::New();
	m_AxialImagePlaneSpatialObject->RequestSetReslicingMode(ReslicerImagePlaneType::Orthogonal);
	m_AxialImagePlaneSpatialObject->RequestSetOrientationType(ReslicerImagePlaneType::Axial);
	m_AxialImagePlaneSpatialObject->RequestSetBoundingBoxProviderSpatialObject(m_CTImageSpatialObject);

	// Create reslice plane spatial object for sagittal view
	m_SagittalImagePlaneSpatialObject = ReslicerImagePlaneType::New();
	m_SagittalImagePlaneSpatialObject->RequestSetReslicingMode(ReslicerImagePlaneType::Orthogonal);
	m_SagittalImagePlaneSpatialObject->RequestSetOrientationType(ReslicerImagePlaneType::Sagittal);
	m_SagittalImagePlaneSpatialObject->RequestSetBoundingBoxProviderSpatialObject(m_CTImageSpatialObject);

	// Create reslice plane spatial object for coronal view
	m_CoronalImagePlaneSpatialObject = ReslicerImagePlaneType::New();
	m_CoronalImagePlaneSpatialObject->RequestSetReslicingMode(ReslicerImagePlaneType::Orthogonal);
	m_CoronalImagePlaneSpatialObject->RequestSetOrientationType(ReslicerImagePlaneType::Coronal);
	m_CoronalImagePlaneSpatialObject->RequestSetBoundingBoxProviderSpatialObject(m_CTImageSpatialObject);

	// Create reslice plane representation for axial view
	m_AxialPlaneRepresentation = CTImageRepresentationType::New();
	m_AxialPlaneRepresentation->SetFrameColor(1,0,0);
	m_AxialPlaneRepresentation->SetWindowLevel(m_WindowWidthImage, m_WindowLevelImage);
	m_AxialPlaneRepresentation->RequestSetImageSpatialObject(m_CTImageSpatialObject);
	m_AxialPlaneRepresentation->RequestSetReslicePlaneSpatialObject(m_AxialImagePlaneSpatialObject);

	// Create reslice plane representation for sagittal view
	m_SagittalPlaneRepresentation = CTImageRepresentationType::New();
	m_SagittalPlaneRepresentation->SetFrameColor(0,1,0);
	m_SagittalPlaneRepresentation->SetWindowLevel(m_WindowWidthImage, m_WindowLevelImage);
	m_SagittalPlaneRepresentation->RequestSetImageSpatialObject(m_CTImageSpatialObject);
	m_SagittalPlaneRepresentation->RequestSetReslicePlaneSpatialObject(m_SagittalImagePlaneSpatialObject);

	// Create reslice plane representation for coronal view
	m_CoronalPlaneRepresentation = CTImageRepresentationType::New();
	m_CoronalPlaneRepresentation->SetFrameColor(0,0,1);
	m_CoronalPlaneRepresentation->SetWindowLevel(m_WindowWidthImage, m_WindowLevelImage);
	m_CoronalPlaneRepresentation->RequestSetImageSpatialObject(m_CTImageSpatialObject);
	m_CoronalPlaneRepresentation->RequestSetReslicePlaneSpatialObject(m_CoronalImagePlaneSpatialObject);


	// Set up cross hairs
	m_CrossHair = CrossHairType::New();
	m_CrossHair->RequestSetBoundingBoxProviderSpatialObject(m_CTImageSpatialObject);


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
	extentObserverID = m_CTImageSpatialObject->AddObserver(igstk::ImageExtentEvent(), extentObserver);
	m_CTImageSpatialObject->RequestGetImageExtent();

	if(extentObserver->GotImageExtent())
	{
		//igstk::EventHelperType::ImageExtentType m_ImageExtentImage
		m_ImageExtentImage = extentObserver->GetImageExtent();

		const unsigned int zmin = m_ImageExtentImage.zmin;
		const unsigned int zmax = m_ImageExtentImage.zmax;
		const unsigned int zslice = static_cast< unsigned int > ((zmin + zmax) / 2.0);

		m_mainFrameDicomViewGUI.scrollBarAxial->setEnabled(true);
		m_mainFrameDicomViewGUI.scrollBarAxial->setMinimum(zmin);
		m_mainFrameDicomViewGUI.scrollBarAxial->setMaximum(zmax);
		m_mainFrameDicomViewGUI.scrollBarAxial->setValue(zslice);


		const unsigned int ymin = m_ImageExtentImage.ymin;
		const unsigned int ymax = m_ImageExtentImage.ymax;
		const unsigned int yslice = static_cast< unsigned int > ((ymin + ymax) / 2.0);

		m_mainFrameDicomViewGUI.scrollBarSagittal->setEnabled(true);
		m_mainFrameDicomViewGUI.scrollBarSagittal->setMinimum(ymin);
		m_mainFrameDicomViewGUI.scrollBarSagittal->setMaximum(ymax);
		m_mainFrameDicomViewGUI.scrollBarSagittal->setValue(yslice);

		const unsigned int xmin = m_ImageExtentImage.xmin;
		const unsigned int xmax = m_ImageExtentImage.xmax;
		const unsigned int xslice = static_cast< unsigned int > ((xmin + xmax) / 2.0);

		m_mainFrameDicomViewGUI.scrollBarCoronal->setEnabled(true);
		m_mainFrameDicomViewGUI.scrollBarCoronal->setMinimum(xmin);
		m_mainFrameDicomViewGUI.scrollBarCoronal->setMaximum(xmax);
		m_mainFrameDicomViewGUI.scrollBarCoronal->setValue(xslice);

	}

	m_CTImageSpatialObject->RemoveObserver(extentObserverID);


	// set transform and parent to the image spatial object
	m_CTImageSpatialObject->RequestSetTransformAndParent( identity, m_ImageWorldReference );

	// Set transform and parent to the image plane reslice spatial objects
	m_AxialImagePlaneSpatialObject->RequestSetTransformAndParent(identity, m_ImageWorldReference);
	m_SagittalImagePlaneSpatialObject->RequestSetTransformAndParent(identity, m_ImageWorldReference);
	m_CoronalImagePlaneSpatialObject->RequestSetTransformAndParent(identity, m_ImageWorldReference);

	m_ViewImageAxial->RequestSetTransformAndParent(identity, m_AxialImagePlaneSpatialObject);
	m_ViewImageSagittal->RequestSetTransformAndParent(identity, m_SagittalImagePlaneSpatialObject);
	m_ViewImageCoronal->RequestSetTransformAndParent(identity, m_CoronalImagePlaneSpatialObject);
	m_ViewMPR->RequestSetTransformAndParent(identity, m_ImageWorldReference);

	// set transform and parent to the cross hair object
	m_CrossHair->RequestSetTransformAndParent(identity, m_ImageWorldReference);

	// Add reslice plane representations to the orthogonal view
	m_ViewImageAxial->RequestAddObject(m_AxialPlaneRepresentation);
	m_ViewImageSagittal->RequestAddObject(m_SagittalPlaneRepresentation);
	m_ViewImageCoronal->RequestAddObject(m_CoronalPlaneRepresentation);


	// Add reslice plane representations to the MPR view
	m_AxialPlaneRepresentation2 = m_AxialPlaneRepresentation->Copy();
	m_SagittalPlaneRepresentation2 = m_SagittalPlaneRepresentation->Copy();
	m_CoronalPlaneRepresentation2 = m_CoronalPlaneRepresentation->Copy();

	m_ViewMPR->RequestAddObject(m_AxialPlaneRepresentation2);
	m_ViewMPR->RequestAddObject(m_SagittalPlaneRepresentation2);
	m_ViewMPR->RequestAddObject(m_CoronalPlaneRepresentation2);

	// Set parallel projection in the 2D views
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
	m_AxialAnnotationPatientName.append(m_CTImageReader->GetPatientName().c_str());

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

	// Add observer for picking events in the Axial view
	m_AxialViewImagePickerObserver = LoadedObserverImageType::New();
	m_AxialViewImagePickerObserver->SetCallbackFunction(this, &loadDicomPlugin::imagePickingProcessingAxial);

	m_ViewImageAxial->AddObserver(igstk::CoordinateSystemTransformToEvent(), m_AxialViewImagePickerObserver);

	// Add observer for picking events in the Axial view
	m_SagittalViewImagePickerObserver = LoadedObserverImageType::New();
	m_SagittalViewImagePickerObserver->SetCallbackFunction(this, &loadDicomPlugin::imagePickingProcessingSagittal);

	m_ViewImageSagittal->AddObserver(igstk::CoordinateSystemTransformToEvent(), m_SagittalViewImagePickerObserver);

	// Add observer for picking events in the Axial view
	m_CoronalViewImagePickerObserver = LoadedObserverImageType::New();
	m_CoronalViewImagePickerObserver->SetCallbackFunction(this, &loadDicomPlugin::imagePickingProcessingCoronal);

	m_ViewImageCoronal->AddObserver(igstk::CoordinateSystemTransformToEvent(), m_CoronalViewImagePickerObserver);

	//m_vtkUpdateCommand = vtkCallbackCommand::New();
	//m_vtkUpdateCommand->SetCallback(&loadDicomPlugin::onVTKUpdateEvent);
	//m_vtkUpdateCommand->SetClientData(this);

	//m_ViewMPR->SetupOrientationBox();
	//m_ViewMPR->RequestSetUpdateCallback(m_vtkUpdateCommand);

	m_ImageLoadedImage= true;

	// Reset the cameras in the different views
	m_ViewImageAxial->RequestResetCamera();
	m_ViewImageSagittal->RequestResetCamera();
	m_ViewImageCoronal->RequestResetCamera();
	m_ViewMPR->RequestResetCamera();

	m_mainFrameDicomViewGUI.dicomViewDisplayAxial->setEnabled(true);
	m_mainFrameDicomViewGUI.dicomViewDisplaySagittal->setEnabled(true);
	m_mainFrameDicomViewGUI.dicomViewDisplayCoronal->setEnabled(true);
	m_mainFrameDicomViewGUI.dicomViewDisplayMPR->setEnabled(true);

	m_mainFrameDicomViewGUI.scrollBarAxial->setEnabled(true);
	m_mainFrameDicomViewGUI.scrollBarSagittal->setEnabled(true);
	m_mainFrameDicomViewGUI.scrollBarCoronal->setEnabled(true);

	m_mainFrameDicomViewGUI.sliceNumberLabelAxial->setEnabled(true);
	m_mainFrameDicomViewGUI.imagePointNumberLabelAxial->setEnabled(true);

	m_mainFrameDicomViewGUI.sliceNumberLabelSagittal->setEnabled(true);
	m_mainFrameDicomViewGUI.imagePointNumberLabelSagittal->setEnabled(true);

	m_mainFrameDicomViewGUI.sliceNumberLabelCoronal->setEnabled(true);
	m_mainFrameDicomViewGUI.imagePointNumberLabelCoronal->setEnabled(true);


	m_toolFrameLoadDicomGUI.mainViewSliderWindowLevel->setEnabled(true);
	m_toolFrameLoadDicomGUI.mainViewSliderWindowWidht->setEnabled(true);

	m_mainFrameDicomViewGUI.dicomViewDisplayMPR->setEnabled(true);

	m_toolFrameLoadDicomGUI.loadImageLevelFrame->setEnabled(true);
	m_toolFrameLoadDicomGUI.loadImageWidhtFrame->setEnabled(true);

	m_ViewImageAxial->RequestStart();
	m_ViewImageSagittal->RequestStart();
	m_ViewImageCoronal->RequestStart();
	m_ViewMPR->RequestStart();

	refreshImageViews();


}

/*
void loadDicomPlugin::onVTKUpdateEvent(vtkObject *caller, unsigned long vtkNotUsed(eid), void * clientdata, void *av)
{
	loadDicomPlugin* client = reinterpret_cast<loadDicomPlugin*> (clientdata);
	double bounds[6];

	itk::Point<double,3> p0;
	p0[0] = bounds[0];
	p0[1] = bounds[2];
	p0[2] = bounds[4];

	itk::Point<double,3> p1;
	p1[0] = bounds[1];
	p1[1] = bounds[3];
	p1[2] = bounds[5];

	ImageIndexType index1;
	client->m_CTImageSpatialObject->TransformPhysicalPointToIndex(p0, index1);
	//cout << index1[0] << " " << index1[1] << " " << index1[2] << endl;
	ImageIndexType index2;
	client->m_CTImageSpatialObject->TransformPhysicalPointToIndex(p1, index2);
	//cout << index2[0] << " " << index2[1] << " " << index2[2] << endl;
}
*/




void loadDicomPlugin::resliceImage(ImageIndexType index)
{
	m_mainFrameDicomViewGUI.scrollBarAxial->setValue(index[2]);
	m_mainFrameDicomViewGUI.scrollBarSagittal->setValue(index[0]);
	m_mainFrameDicomViewGUI.scrollBarCoronal->setValue(index[1]);

	//resetImageCameraSettings();
	//refreshImageViews();

}


/** Reslice image Axial */
void loadDicomPlugin::resliceImageViews(int value)
{
	ImageIndexType index;
	index[2]= m_mainFrameDicomViewGUI.scrollBarAxial->value();
	index[0]= m_mainFrameDicomViewGUI.scrollBarSagittal->value();
	index[1]= m_mainFrameDicomViewGUI.scrollBarCoronal->value();

	QString str1;
	QString str2;
	QString str3;
	str1.append(QString("%1").arg(index[2]));
	str2.append(QString("%1").arg(index[0]));
	str3.append(QString("%1").arg(index[1]));
	m_mainFrameDicomViewGUI.sliceNumberLabelAxial->setText(str1);
	m_mainFrameDicomViewGUI.sliceNumberLabelSagittal->setText(str2);
	m_mainFrameDicomViewGUI.sliceNumberLabelCoronal->setText(str3);

	ImagePointType point;
	m_CTImageSpatialObject->TransformIndexToPhysicalPoint( index, point );


	const double *data = point.GetVnlVector().data_block();


	m_CrossHair->RequestSetCursorPosition( data );
	m_AxialImagePlaneSpatialObject->RequestSetCursorPosition( data );
	m_SagittalImagePlaneSpatialObject->RequestSetCursorPosition( data );
	m_CoronalImagePlaneSpatialObject->RequestSetCursorPosition( data );

	//resetImageCameraSettings();
	//refreshImageViews();



}


/** Set window level and witdth */
void loadDicomPlugin::changeImageWindowing(int windowing)
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



/** Reslices the views to the picked position */
void loadDicomPlugin::imagePickingProcessing()
{
	igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::imagePickingProcessing called...! \n");

	ImagePointType point = TransformToPointImage(m_ImagePickingTransform);

	if (m_CTImageSpatialObject->IsInside(point))
	{
		ImageIndexType index;
		m_CTImageSpatialObject->TransformPhysicalPointToIndex(point, index);

		const double *data = point.GetVnlVector().data_block();

		m_AxialImagePlaneSpatialObject->RequestSetCursorPosition(data);
		m_SagittalImagePlaneSpatialObject->RequestSetCursorPosition(data);
		m_CoronalImagePlaneSpatialObject->RequestSetCursorPosition(data);

		this->resliceImage(index);

		QString str1;
		QString str2;
		QString str3;
		str1.append(QString("%1").arg(data[0]));
		str2.append(QString("%1").arg(data[1]));
		str3.append(QString("%1").arg(data[2]));

		m_mainFrameDicomViewGUI.imagePointNumberLabelAxial->setText(str1);
		m_mainFrameDicomViewGUI.imagePointNumberLabelSagittal->setText(str2);
		m_mainFrameDicomViewGUI.imagePointNumberLabelCoronal->setText(str3);
	}
	else
	{
		igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::Picked point outside image\n");
	}
}

/** Reslices the views Sagittal and Coronal to the picked position on Axial */
void loadDicomPlugin::imagePickingProcessingAxial(const itk::EventObject & event)
{
	if (igstk::CoordinateSystemTransformToEvent().CheckEvent(&event))
	{
		typedef igstk::CoordinateSystemTransformToEvent TransformEventType;
		const TransformEventType * tmevent = dynamic_cast< const TransformEventType *>(& event);

		// Get the transform from the view to its parent (reslicer plane)
		igstk::CoordinateSystemTransformToResult transformCarrier = tmevent->Get();
		m_ImagePickingTransform = transformCarrier.GetTransform();

		// Get the transform from the reslicer plane to its parent (world reference)
		CoordinateSystemTransformObserver::Pointer coordinateObserver =	CoordinateSystemTransformObserver::New();

		unsigned int obsId = m_AxialImagePlaneSpatialObject->AddObserver( igstk::CoordinateSystemTransformToEvent(), coordinateObserver);

		m_AxialImagePlaneSpatialObject->RequestComputeTransformTo(m_ImageWorldReference);

			if(coordinateObserver->GotCoordinateSystemTransform())
			{
				igstk::CoordinateSystemTransformToResult transformToResult = coordinateObserver->GetCoordinateSystemTransform();
				igstk::Transform viewToWorldReferenceTransform = transformToResult.GetTransform();
				m_ImagePickingTransform = igstk::Transform::TransformCompose(viewToWorldReferenceTransform, m_ImagePickingTransform);
			}
			else
			{
				igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::imagePickingProcessingAxial could not get coordinate system transform \n");
				return;
			}

		m_AxialImagePlaneSpatialObject->RemoveObserver(obsId);
	}

	imagePickingProcessing();
	//resetImageCameraSettings();
	refreshImageViews();
}


/** Reslices the views Axial and Coronal to the picked position on Sagittal */
void loadDicomPlugin::imagePickingProcessingSagittal(const itk::EventObject & event)
{
	if (igstk::CoordinateSystemTransformToEvent().CheckEvent(&event))
	{
		typedef igstk::CoordinateSystemTransformToEvent TransformEventType;
		const TransformEventType * tmevent = dynamic_cast< const TransformEventType *>(& event);

		// Get the transform from the view to its parent (reslicer plane)
		igstk::CoordinateSystemTransformToResult transformCarrier = tmevent->Get();
		m_ImagePickingTransform = transformCarrier.GetTransform();

		// Get the transform from the reslicer plane to its parent (world reference)
		CoordinateSystemTransformObserver::Pointer coordinateObserver =	CoordinateSystemTransformObserver::New();

		unsigned int obsId = m_SagittalImagePlaneSpatialObject->AddObserver(igstk::CoordinateSystemTransformToEvent(), coordinateObserver);

		m_SagittalImagePlaneSpatialObject->RequestComputeTransformTo(m_ImageWorldReference);

		if(coordinateObserver->GotCoordinateSystemTransform())
		{
			igstk::CoordinateSystemTransformToResult transformToResult = coordinateObserver->GetCoordinateSystemTransform();
			igstk::Transform viewToWorldReferenceTransform = transformToResult.GetTransform();
			m_ImagePickingTransform = igstk::Transform::TransformCompose(viewToWorldReferenceTransform, m_ImagePickingTransform);
		}
		else
		{
			igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::imagePickingProcessingSagittal could not get coordinate system transform \n");
			return;
		}

		m_SagittalImagePlaneSpatialObject->RemoveObserver(obsId);

	}

	imagePickingProcessing();
	//resetImageCameraSettings();
	refreshImageViews();
}


/** Reslices the views Axial and Sagittal to the picked position on Coronal */
void loadDicomPlugin::imagePickingProcessingCoronal(const itk::EventObject & event)
{
	if (igstk::CoordinateSystemTransformToEvent().CheckEvent(&event))
	{
		typedef igstk::CoordinateSystemTransformToEvent TransformEventType;
		const TransformEventType * tmevent = dynamic_cast< const TransformEventType *>(& event);

		// Get the transform from the view to its parent (reslicer plane)
		igstk::CoordinateSystemTransformToResult transformCarrier = tmevent->Get();
		m_ImagePickingTransform = transformCarrier.GetTransform();

		// Get the transform from the reslicer plane to its parent (world reference)
		CoordinateSystemTransformObserver::Pointer coordinateObserver = 	CoordinateSystemTransformObserver::New();

		unsigned int obsId = m_CoronalImagePlaneSpatialObject->AddObserver(
			igstk::CoordinateSystemTransformToEvent(), coordinateObserver);

		m_CoronalImagePlaneSpatialObject->RequestComputeTransformTo(m_ImageWorldReference);

		if(coordinateObserver->GotCoordinateSystemTransform())
		{
			igstk::CoordinateSystemTransformToResult transformToResult = coordinateObserver->GetCoordinateSystemTransform();
			igstk::Transform viewToWorldReferenceTransform = transformToResult.GetTransform();
			m_ImagePickingTransform = igstk::Transform::TransformCompose(viewToWorldReferenceTransform, m_ImagePickingTransform);
		}
		else
		{
			igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::imagePickingProcessingCoronal could not get coordinate system transform \n");
			return;
		}

		m_CoronalImagePlaneSpatialObject->RemoveObserver(obsId);
	}

	imagePickingProcessing();
	//resetImageCameraSettings();
	refreshImageViews();
}



void loadDicomPlugin::refreshImageViews()
{
	m_mainFrameDicomViewGUI.dicomViewDisplayAxial->repaint();
	m_mainFrameDicomViewGUI.dicomViewDisplaySagittal->repaint();
	m_mainFrameDicomViewGUI.dicomViewDisplayCoronal->repaint();
	m_mainFrameDicomViewGUI.dicomViewDisplayMPR->repaint();

	igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::refreshImageViews called\n");
}



void loadDicomPlugin::resetImageCameraSettings()
{
	m_ViewImageAxial->RequestResetCamera();
	m_ViewImageSagittal->RequestResetCamera();
	m_ViewImageCoronal->RequestResetCamera();
	m_ViewMPR->RequestResetCamera();

	m_ViewImageAxial->SetCameraZoomFactor(m_DefaultZoomImageAxial);
	m_ViewImageSagittal->SetCameraZoomFactor(m_DefaultZoomImageSagittal);
	m_ViewImageCoronal->SetCameraZoomFactor(m_DefaultZoomImageCoronal);

	igstkLogMacro2(m_Logger, DEBUG,	"loadDicomPlugin::resetImageCameraSettings called\n");
}








Q_EXPORT_PLUGIN2(at_voxelmaster_loadDicomPlugin, loadDicomPlugin)


