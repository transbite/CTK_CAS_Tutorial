
#include <QObject>
#include <QString>


/** IGSTK header files */
#include "igstkDICOMImageReader.h"  //warning on windows
#include "igstkCTImageReader.h"	//warning on windows
#include "igstkCTImageSpatialObject.h"


class dicomDataInterface : public QObject
  {
	 Q_OBJECT


public:

	 	 typedef igstk::CTImageReader                        			DicomDataReaderType;
	 	 typedef DicomDataReaderType::ImageSpatialObjectType   			DicomDataSpatialObjectType;


		 DicomDataSpatialObjectType::Pointer							dicomSpatialObject;





};

Q_DECLARE_INTERFACE(dicomDataInterface, "dicomDataInterface");
