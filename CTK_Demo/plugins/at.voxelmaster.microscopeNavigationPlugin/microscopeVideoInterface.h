#include <QObject>
#include <QString>

#include <QtGui>
#include <vector>
#include <QtMultimediaKit/qabstractvideobuffer.h>
#include <QtMultimediaKit/qvideoframe.h>
#include <stdlib.h>
class microscopeVideoInterface : public QObject
  {
	 Q_OBJECT


public:

	 //QWidget								*microscopeVideoInterfaceWidget;
	 std::vector<QAbstractVideoBuffer*> microscopeVideoInterfaceBuffers;

	 QSize									*size;
	 QVideoFrame::PixelFormat				pixelFormat;
};


Q_DECLARE_INTERFACE(microscopeVideoInterface, "microscopeVideoInterface");
