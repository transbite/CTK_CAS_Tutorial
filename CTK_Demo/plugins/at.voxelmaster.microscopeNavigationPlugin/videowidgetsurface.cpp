 #include "videowidgetsurface.h"
#include <iostream>
#include <fstream>


 VideoWidgetSurface::VideoWidgetSurface(QWidget *widget, QObject *parent)
     : QAbstractVideoSurface(parent)
     , widget(widget)
     , imageFormat(QImage::Format_Invalid)
 {
	 this->encoder = NULL;
 }



 QList<QVideoFrame::PixelFormat> VideoWidgetSurface::supportedPixelFormats(
         QAbstractVideoBuffer::HandleType handleType) const
 {
     if (handleType == QAbstractVideoBuffer::NoHandle) {
         return QList<QVideoFrame::PixelFormat>()
                 << QVideoFrame::Format_RGB32
                 << QVideoFrame::Format_ARGB32
                 << QVideoFrame::Format_ARGB32_Premultiplied
                 << QVideoFrame::Format_RGB565
                 << QVideoFrame::Format_RGB555;
     } else {
         return QList<QVideoFrame::PixelFormat>();
     }
 }

 void VideoWidgetSurface::setEncoder(QVideoEncoder* encoder){
	 this->encoder = encoder;

 }

 void VideoWidgetSurface::finishEncoder(){
	 if(this->encoder!=NULL){
		 this->encoder->close();
		 this->encoder=NULL;
	 }
 }

 bool VideoWidgetSurface::isFormatSupported(
         const QVideoSurfaceFormat &format, QVideoSurfaceFormat *similar) const
 {
     Q_UNUSED(similar);



     const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
     const QSize size = format.frameSize();

     return imageFormat != QImage::Format_Invalid
             && !size.isEmpty()
             && format.handleType() == QAbstractVideoBuffer::NoHandle;
 }

 bool VideoWidgetSurface::start(const QVideoSurfaceFormat &format)
 {
     const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
     const QSize size = format.frameSize();

     if (imageFormat != QImage::Format_Invalid && !size.isEmpty()) {
         this->imageFormat = imageFormat;
         imageSize = size;

         sourceRect = format.viewport();

         QAbstractVideoSurface::start(format);
         std::cout << "start true" << std::endl;
         widget->updateGeometry();
         updateVideoRect();

         return true;
     } else {
    	 std::cout << "start false" << std::endl;
         return false;
     }

 }

 void VideoWidgetSurface::stop()
 {

     currentFrame = QVideoFrame();
     targetRect = QRect();

     QAbstractVideoSurface::stop();


     widget->update();


 }


 bool VideoWidgetSurface::present(const QVideoFrame &frame)
 {
     if ( surfaceFormat().frameSize() != frame.size()) {
         setError(IncorrectFormatError);
         stop();
         std::cout << "present false" << std::endl;
         return false;
     } else {
    	   std::cout << "present true" << std::endl;
         currentFrame = frame;

         widget->repaint(targetRect);

         return true;
     }
 }

 void VideoWidgetSurface::updateVideoRect()
 {
     QSize size = surfaceFormat().sizeHint();
     size.scale(widget->size().boundedTo(size), Qt::KeepAspectRatio);

     targetRect = QRect(QPoint(0, 0), size);
     targetRect.moveCenter(widget->rect().center());
 }

 void VideoWidgetSurface::paint(QPainter *painter)
 {
	 if (currentFrame.map(QAbstractVideoBuffer::ReadOnly)) {
	          const QTransform oldTransform = painter->transform();

	          if (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop) {
	             painter->scale(1, -1);
	             painter->translate(0, -widget->height());
	          }

	          QImage image(
	                  currentFrame.bits(),
	                  currentFrame.width(),
	                  currentFrame.height(),
	                  currentFrame.bytesPerLine(),
	                  imageFormat);

	          painter->drawImage(targetRect, image, sourceRect);

	          if(encoder!=NULL)
	        	  encoder->encodeImage(image);
	          painter->setTransform(oldTransform);

	          currentFrame.unmap();
	      }
 }
