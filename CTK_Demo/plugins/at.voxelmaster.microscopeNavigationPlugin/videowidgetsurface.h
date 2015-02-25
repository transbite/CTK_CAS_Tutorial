#ifndef VIDEOWIDGETSURFACE_H
 #define VIDEOWIDGETSURFACE_H

 #include <QtCore/QRect>
 #include <QtGui/QImage>
 #include <QtMultimediaKit/qabstractvideosurface.h>
 #include <QtMultimediaKit/qvideoframe.h>

 #include <QtMultimediaKit/qvideosurfaceformat.h>
 #include <qwidget.h>
 #include <qpainter.h>
#include "QVideoEncoder.h"

 class VideoWidgetSurface : public QAbstractVideoSurface
 {
     Q_OBJECT
 public:
     VideoWidgetSurface(QWidget *widget, QObject *parent = 0);

     QList<QVideoFrame::PixelFormat> supportedPixelFormats(
             QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
     bool isFormatSupported(const QVideoSurfaceFormat &format, QVideoSurfaceFormat *similar) const;

     bool start(const QVideoSurfaceFormat &format);
     void stop();

     bool present(const QVideoFrame &frame);

     QRect videoRect() const { return targetRect; }
     void updateVideoRect();

     void paint(QPainter *painter);

     void setEncoder(QVideoEncoder* encoder);
     void finishEncoder();

 private:
     QWidget *widget;
     QImage::Format imageFormat;
     QRect targetRect;
     QSize imageSize;
     QRect sourceRect;
     QVideoFrame currentFrame;
     QVideoEncoder* encoder;

 };

 #endif
