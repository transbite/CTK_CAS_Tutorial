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


#ifndef loadDicomPlugin_H
#define loadDicomPlugin_H

#include "loadDicomInterface.h"
#include "dicomDataInterface.h"

#include "ui_toolFrameLoadDicomGUI.h"
#include "ui_mainFrameDicomViewGUI.h"

#include <ctkPluginActivator.h>

/** QT header files */
#include <QString>
#include <QFileInfo>
#include <QErrorMessage>
#include <QFileDialog>
#include <QtGui>
#include <QObject>
#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QtCore/QTimer>
#include <QDesktopServices>
#include <QDialog>
#include <QFileDialog>
#include "QVTKWidget.h"
#include <QPushButton>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QWidget>




/** VTK header files */
#include "vtkCallbackCommand.h"


/** IGSTK header files */
#include "igstkConfigure.h"
#include "igstkView.h"
#include "igstkView2D.h"
#include "igstkView3D.h"
#include "igstkDICOMImageReader.h"  //warning on windows
#include "igstkCTImageReader.h"	//warning on windows
#include "igstkCTImageSpatialObject.h"
#include "igstkCTImageSpatialObjectRepresentation.h"
#include "igstkStateMachine.h"
#include "igstkMacros.h"
#include "igstkObject.h"
#include "igstkEvents.h"
#include "igstkReslicerPlaneSpatialObject.h"
#include "igstkImageResliceObjectRepresentation.h"
#include "igstkAxesObject.h"
#include "igstkAnnotation2D.h"
#include "igstkCoordinateSystemTransformToResult.h"
#include "igstkQTWidget.h"
#include "igstkEllipsoidObject.h"
#include "igstkEllipsoidObjectRepresentation.h"
#include "igstkToolProjectionSpatialObject.h"
#include "igstkToolProjectionObjectRepresentation.h"
#include "igstkCrossHairSpatialObject.h"
#include "igstkCrossHairObjectRepresentation.h"


/** ITK header files */
#include "itksys/SystemTools.hxx"
#include "itksys/Directory.hxx"
#include "itkLogger.h"
#include "itkLoggerBase.h"
#include "itkStdStreamLogOutput.h"
#include "itkCommand.h"
#include "itkEventObject.h"
#include "itkImageSeriesReader.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageDuplicator.h"


class loadDicomPlugin :  public QObject, public ctkPluginActivator
{
	Q_OBJECT
	Q_INTERFACES(ctkPluginActivator)

	//igstkStandardClassBasicTraitsMacro(loadDicomPlugin, QObject);



public:

	/** Typedefs */
		//igstkStandardClassBasicTraitsMacro(aroscopeOpenFileWindowGUI, QMainWindow);

		/** Declarations needed for the State Machine */
		//igstkStateMachineMacro();

	typedef igstk::AxesObject										AxesObjectImageType;

	typedef igstk::Object::LoggerType     							LoggerType;
	typedef itk::StdStreamLogOutput       							LogOutputType;

	igstk::Transform                                				m_ImagePickingTransform;
	igstk::Transform                                				m_RegistrationTransform;

	typedef igstk::View												ViewType;
	typedef igstk::View2D       									View2DType;
	typedef igstk::View3D       									View3DType;

	typedef igstk::CTImageReader                        			CTImageReaderType;
	typedef CTImageReaderType::ImageSpatialObjectType   			CTImageSpatialObjectType;

	typedef CTImageSpatialObjectType::IndexType           			ImageIndexType;
	typedef CTImageSpatialObjectType::PointType           			ImagePointType;


	// Cross hair spatial object and representation
	typedef igstk::CrossHairSpatialObject            				CrossHairType;
	typedef igstk::CrossHairObjectRepresentation     				CrossHairRepresentationType;

	// tool projection spatial object and representation
	typedef igstk::ToolProjectionSpatialObject             			ToolProjectionType;
	typedef igstk::ToolProjectionObjectRepresentation	   			ToolProjectionRepresentationType;


	typedef igstk::ImageResliceObjectRepresentation
			   <CTImageSpatialObjectType> 							CTImageRepresentationType;

	typedef igstk::ReslicerPlaneSpatialObject           			ReslicerImagePlaneType;

	typedef itk::ReceptorMemberCommand <loadDicomPlugin> 			LoadedObserverImageType;
	typedef itk::SimpleMemberCommand <loadDicomPlugin> 				CancelObserverImageType;

	typedef itk::MemberCommand<loadDicomPlugin>						ProgressCommandType;

	typedef igstk::Annotation2D 									Annotation2DImageType;



	igstkObserverMacro(CoordinateSystemTransform,
					   igstk::CoordinateSystemTransformToEvent,
					   igstk::CoordinateSystemTransformToResult)

	igstkObserverMacro(DICOMImageReaderInvalidDirectoryNameError,
			           igstk::DICOMImageDirectoryIsNotDirectoryErrorEvent,
			           std::string)

	igstkObserverMacro(DICOMImageReaderNonExistingDirectoryError,
			           igstk::DICOMImageDirectoryDoesNotExistErrorEvent,
			           igstk::EventHelperType::StringType)

	igstkObserverMacro(DICOMImageReaderEmptyDirectoryError,
			           igstk::DICOMImageDirectoryEmptyErrorEvent,
			           igstk::EventHelperType::StringType)

	igstkObserverMacro(DICOMImageDirectoryNameDoesNotHaveEnoughFilesError,
			           igstk::DICOMImageDirectoryDoesNotHaveEnoughFilesErrorEvent,
			           igstk::EventHelperType::StringType)

	igstkObserverMacro(DICOMImageDirectoryDoesNotContainValidDICOMSeriesError,
					   igstk::DICOMImageSeriesFileNamesGeneratingErrorEvent,
					   igstk::EventHelperType::StringType)

	igstkObserverMacro(DICOMImageInvalidError,
					   igstk::DICOMImageReadingErrorEvent,
					   igstk::EventHelperType::StringType)

	igstkObserverObjectMacro(CTImage,
			                 ::igstk::CTImageReader::ImageModifiedEvent,
							 igstk::CTImageSpatialObject);

	igstkObserverMacro(ImageExtent,
			           igstk::ImageExtentEvent,
					   igstk::EventHelperType::ImageExtentType);

	igstkObserverMacro(VTKImage,
			           ::igstk::VTKImageModifiedEvent,
					   igstk::EventHelperType::VTKImagePointerType);

	igstkEventMacro(InvalidRequestErrorEvent,
			        igstk::IGSTKErrorEvent);

	igstkEventMacro(CloseCommunicationErrorEvent,
			        igstk::IGSTKErrorWithStringEvent);


	igstkObserverMacro(Registration,
			           igstk::CoordinateSystemTransformToEvent,
					   igstk::CoordinateSystemTransformToResult);

	igstkObserverMacro(RegistrationError,
			           igstk::DoubleTypeEvent,
			           double);


	igstkObserverMacro(TransformationDescription,
			           igstk::StringEvent,
					   std::string)



	loadDicomPlugin();
	~loadDicomPlugin();

	void start(ctkPluginContext* context);
	void stop(ctkPluginContext* context);

	static loadDicomPlugin* getInstance();

	ctkPluginContext* getPluginContext() const;


private:

	static loadDicomPlugin									*instance;
	ctkPluginContext										*context;

	Ui::toolFrameLoadDicomGUI								m_toolFrameLoadDicomGUI;
	Ui::mainFrameDicomViewGUI								m_mainFrameDicomViewGUI;

	loadDicomInterface										*loadDicomInterfaceInstance;

	QWidget													*toolFrameLoadDicomWidget;
	QWidget													*mainFrameVisualizationDicomViewWidget;

	ctkServiceRegistration dicomDataServiceReference;
	dicomDataInterface*	dicomDataInterfaceInstance;


	void createWidgetsAndSignalSlotActions();
	void startDicomDataService();

	void updateImageData(CTImageSpatialObjectType::Pointer);


	loadDicomPlugin(const loadDicomPlugin&); // purposely not implemented
	void operator=(const loadDicomPlugin&); // purposely not implemented

	AxesObjectImageType::Pointer								m_ImageWorldReference;

	LoggerType::Pointer   										m_Logger;
	LogOutputType::Pointer 										m_LogOutput;

	ReslicerImagePlaneType::Pointer								m_AxialImagePlaneSpatialObject;
	ReslicerImagePlaneType::Pointer								m_SagittalImagePlaneSpatialObject;
	ReslicerImagePlaneType::Pointer								m_CoronalImagePlaneSpatialObject;

	CTImageReaderType::Pointer									m_CTImageReader;
	CTImageObserver::Pointer                       				m_CTImageObserver;

	CTImageRepresentationType::Pointer							m_AxialPlaneRepresentation;
	CTImageRepresentationType::Pointer							m_SagittalPlaneRepresentation;
	CTImageRepresentationType::Pointer							m_CoronalPlaneRepresentation;

	CTImageRepresentationType::Pointer                  		m_AxialPlaneRepresentation2;
	CTImageRepresentationType::Pointer                  		m_SagittalPlaneRepresentation2;
	CTImageRepresentationType::Pointer                  		m_CoronalPlaneRepresentation2;

	View2DType::Pointer											m_ViewImageAxial;
	View2DType::Pointer											m_ViewImageSagittal;
	View2DType::Pointer											m_ViewImageCoronal;
	View3DType::Pointer											m_ViewMPR;

	CTImageSpatialObjectType::Pointer							m_CTImageSpatialObject;


	// Ellipsoid spatial object, used to represent the fiducial points
	typedef igstk::EllipsoidObject                  			EllipsoidType;
	typedef igstk::EllipsoidObjectRepresentation    			EllipsoidRepresentationType;

	EllipsoidType::Pointer                          			m_PickedPoint;
	EllipsoidRepresentationType::Pointer            			m_PickedPointRepresentation;


	// tool projection spatial object
	ToolProjectionType::Pointer                           		m_ToolProjection;

	// cross hair spatial object
	CrossHairType::Pointer                               		m_CrossHair;

	// tool projection representations
	ToolProjectionRepresentationType::Pointer					m_AxialToolProjectionRepresentation;
	ToolProjectionRepresentationType::Pointer					m_SagittalToolProjectionRepresentation;
	ToolProjectionRepresentationType::Pointer					m_CoronalToolProjectionRepresentation;

	// cross hair representation
	CrossHairRepresentationType::Pointer        				m_AxialCrossHairRepresentation;
	CrossHairRepresentationType::Pointer        				m_SagittalCrossHairRepresentation;
	CrossHairRepresentationType::Pointer        				m_CoronalCrossHairRepresentation;
	CrossHairRepresentationType::Pointer        				m_3DViewCrossHairRepresentation;


	LoadedObserverImageType::Pointer							m_AxialViewImagePickerObserver;
	LoadedObserverImageType::Pointer							m_SagittalViewImagePickerObserver;
	LoadedObserverImageType::Pointer							m_CoronalViewImagePickerObserver;

	vtkCallbackCommand											*m_vtkUpdateCommand;

	Annotation2DImageType::Pointer 								m_AnnotationImageAxial;
	Annotation2DImageType::Pointer 								m_AnnotationImageSagittal;
	Annotation2DImageType::Pointer 								m_AnnotationImageCoronal;
	Annotation2DImageType::Pointer								m_AnnotationImageMPR;
	Annotation2DImageType::Pointer								m_AnnotationImage3DObject;

	igstk::EventHelperType::ImageExtentType						m_ImageExtentImage;


	QMessageBox 												*m_messageDialog;

	std::string													m_CTImageDirectory;

	std::string													m_AxialAnnotation;
	std::string													m_AxialAnnotationPatientName;
	std::string													m_SagittalAnnotation;
	std::string													m_CoronalAnnotation;
	std::string													m_MPRand3DAnnotation;

	std::string   												m_logDirectoryName;
	std::string   												m_logDirectoryNameDate;
	std::string													m_logDirectoryNameCurrentlyUsed;

	std::string													m_ReadPointSetFile;
	std::string													m_ReadVtkPolyDataFile;
	std::string													m_ReadFixedPointSetFile;
	std::string													m_ReadMovingPointSetFile;


	double														m_WindowLevelImage;
	double														m_WindowWidthImage;

	double														m_DefaultZoomImageAxial;
	double														m_DefaultZoomImageCoronal;
	double														m_DefaultZoomImageSagittal;
	double														m_DefaultZoomImageMPR;

	double														m_CTImageBounds[6];

	double														m_VoxelSpacingImage[3];

	int 														m_MinWindowLevelValue;
	int 														m_MaxWindowLevelValue;

	int 														m_MinWindowWidhtValue;
	int 														m_MaxWindowWidhtValue;

	int															m_VoxelDimensionImage[3];
	int 														m_ImageExtentCT[6];

	bool														m_ImageLoadedImage;
	float														m_ReductionInPercent;
	int															m_NumberOfIterationsIcp;
	int															m_NumberOfLandmarksIcp;
	float														m_NumberOfMeanDistanceIcp;



	/** Utility functions, conversion between points and transform */
	inline
	igstk::Transform
	PointToTransformImage(ImagePointType point)
	{
		igstk::Transform transform;
		igstk::Transform::VectorType translation;
		for (int i=0; i<3; i++)
		{
			translation[i] = point[i];
		}
		transform.SetTranslation(translation, 0.1,
		igstk::TimeStamp::GetLongestPossibleTime());

		return transform;
	}

	inline
	ImagePointType
	TransformToPointImage(igstk::Transform transform)
	{
	ImagePointType point;
	for (int i=0; i<3; i++)
	{
		point[i] = transform.GetTranslation()[i];
	}
	return point;
	}




public slots:

	void initializeQuadrantView();

	void readCTDataset();
	void readCTDatasetForDicomInterace();
	void displayCTDataSet();

	void changeImageWindowing(int windowing);

	void getCTImageBounds(CTImageSpatialObjectType::Pointer);

	void refreshImageViews();
	void resetImageCameraSettings();

	void resliceImage(ImageIndexType index);
	void resliceImageViews(int value);


	void imagePickingProcessing();
	void imagePickingProcessingAxial(const itk::EventObject & event);
	void imagePickingProcessingSagittal(const itk::EventObject & event);
	void imagePickingProcessingCoronal(const itk::EventObject & event);

	static void onVTKUpdateEvent(vtkObject *caller, unsigned long vtkNotUsed(eid), void *, void *av);
	void onITKProgressEvent(itk::Object *source, const itk::EventObject &);
	void pulseTimerEvent();


}; // loadDicomPlugin


#endif // loadDicomPlugin_H
