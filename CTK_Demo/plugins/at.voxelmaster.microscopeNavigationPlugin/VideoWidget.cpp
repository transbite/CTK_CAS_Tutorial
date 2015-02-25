 #include "VideoWidget.h"

 #include "videowidgetsurface.h"


 #include <QPaintEvent>
#include <iostream>
#include <fstream>

 VideoWidget::VideoWidget(QWidget *parent)
     : QWidget(parent)
     , surface(0)
 {
     setAutoFillBackground(true);


     QPalette palette = this->palette();
     palette.setColor(QPalette::Background, Qt::black);
     setPalette(palette);

     QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
     sizePolicy.setHorizontalStretch(0);
     sizePolicy.setVerticalStretch(0);
     sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
     this->setSizePolicy(sizePolicy);


     surface = new VideoWidgetSurface(this);


 }

 VideoWidget::~VideoWidget()
 {
     delete surface;
 }

 void VideoWidget::finishEncoder(){
	 surface->finishEncoder();
 }

 void VideoWidget::setEncoder(QVideoEncoder* encoder){
	 surface->setEncoder(encoder);
 }

 QSize VideoWidget::sizeHint() const
 {
	 return  QSize(0,0);

 }

 void VideoWidget::paintEvent(QPaintEvent *event)
 {

     QPainter painter(this);
     //painter.scale(0.5,0.5);
     surface->paint(&painter);

 }

 void VideoWidget::resizeEvent(QResizeEvent *event)
 {
     QWidget::resizeEvent(event);

     surface->updateVideoRect();
 }
