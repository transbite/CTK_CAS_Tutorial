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


#include "microscopeNavigationPlugin.h"
#include "microscopeVideoServiceTracker.h"

#include <iostream>
#include <fstream>

#include <QtPlugin>
#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>
#include <unistd.h>
#include <QVideoFrame>
#include <QtMultimediaKit/qabstractvideosurface.h>
#include <QVideoSurfaceFormat>
#include <QtMultimediaKit/qmediaservice.h>
#include <QtMultimediaKit/qvideorenderercontrol.h>
#include <QtMultimediaKit/qcamera.h>

microscopeNavigationPlugin* microscopeNavigationPlugin::instance = 0;

microscopeNavigationPlugin::microscopeNavigationPlugin()
  : context(0)
{
	std::cout << "Microscope Navigation plugin constructor" << std::endl;
}



microscopeNavigationPlugin::~microscopeNavigationPlugin()
{
  
}



void microscopeNavigationPlugin::start(ctkPluginContext* context)
{
	std::cout << "Microscope Navigation plugin start" << std::endl;

	instance = this;
	this->context = context;

	this->createWidgetsAndSignalSlotActions();
	initMicroscopeVideoTracker();

/*
	QStandardItemModel* m_leftCameraComboBoxItemModel= qobject_cast<QStandardItemModel*>(m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->model());
	if(m_leftCameraComboBoxItemModel)
	{
	    QModelIndex m_leftCameraComboBoxIndex = m_leftCameraComboBoxItemModel->index(0 ,m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->modelColumn(), m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->rootModelIndex());
	    QStandardItem* m_leftCameraComboBoxItem = m_leftCameraComboBoxItemModel->itemFromIndex(m_leftCameraComboBoxIndex);
	    if(m_leftCameraComboBoxItem)
	    {
	    	m_leftCameraComboBoxItem->setEnabled(false);
	    }
	}


	QStandardItemModel* m_rightCameraComboBoxItemModel= qobject_cast<QStandardItemModel*>(m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->model());
	if(m_rightCameraComboBoxItemModel)
	{
	    QModelIndex m_rightCameraComboBoxIndex = m_rightCameraComboBoxItemModel->index(0 ,m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->modelColumn(), m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->rootModelIndex());
	    QStandardItem* m_rightCameraComboBoxItem = m_rightCameraComboBoxItemModel->itemFromIndex(m_rightCameraComboBoxIndex);
	    if(m_rightCameraComboBoxItem)
	    {
	    	m_rightCameraComboBoxItem->setEnabled(false);
	    }
	}
*/

	connect(m_toolFrameMicroscopeNavigationGUI.microscopeStartCaptureVideoPushButton, SIGNAL(clicked()), this, SLOT(startCaptureVideo()));
	connect(m_toolFrameMicroscopeNavigationGUI.microscopeStopCaptureVideoPushButton, SIGNAL(clicked()), this, SLOT(stopCaptureVideo()));

	connect(m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox, SIGNAL(currentIndexChanged(QString)), m_mainFrameMicroscopeViewGUI.displayedCameraOnLeftWidgetTextLabel, SLOT(setText(QString)));
	connect(m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox, SIGNAL(currentIndexChanged(QString)), m_mainFrameMicroscopeViewGUI.displayedCameraOnRightWidgetTextLabel, SLOT(setText(QString)));

	connect(m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeDisplayedViewOnLeftWidget(int)));
	connect(m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeDisplayedViewOnRightWidget(int)));

	connect(m_toolFrameMicroscopeNavigationGUI.switchMicroscopeViewsPushButton,SIGNAL(clicked()), this, SLOT(switchMicroscopeViewsChangedState()));

}



void microscopeNavigationPlugin::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)
}



microscopeNavigationPlugin* microscopeNavigationPlugin::getInstance()
{
	return instance;
}



ctkPluginContext* microscopeNavigationPlugin::getPluginContext() const
{
	std::cout << "Microscope Navigation context" << std::endl;
	return context;
}


void microscopeNavigationPlugin::initMicroscopeVideoTracker(){



	microscopeVideoServiceTracker* mVst = new microscopeVideoServiceTracker(context,this);

	ctkServiceTracker<microscopeVideoInterface*,microscopeVideoInterface*>* tracker = new ctkServiceTracker<microscopeVideoInterface*,microscopeVideoInterface*>(context,mVst);
	tracker->open();

}



void microscopeNavigationPlugin::visualizeVideo(std::vector<QAbstractVideoBuffer*> microscopeVideoInterfaceBuffers, QSize size,QVideoFrame::PixelFormat pixelFormat)
{
	std::cout << "Call Microscope Navigation visualizeVideo..." << std::endl;



	this->microscopeVideoInterfaceBuffers = microscopeVideoInterfaceBuffers;
	this->buffersize = new QSize(size);
	this->bufferpixelFormat=&pixelFormat;

	videoWidgetThread=new RefreshWidgetThread();
	videoWidgetThread->start();

	QVideoSurfaceFormat format(*(new QSize(size)),pixelFormat);

	if(microscopeVideoInterfaceBuffers.size()>0){
		leftVideoWidget->videoSurface()->start(format);
		this->setQVideoFrameToVideoWidgetThread(true, new QVideoFrame(microscopeVideoInterfaceBuffers.data()[0],*(new QSize(size)),pixelFormat));
		videoWidgetThread->addWidget(leftVideoWidget);
	}
	if(microscopeVideoInterfaceBuffers.size()>1){
		rightVideoWidget->videoSurface()->start(format);
		this->setQVideoFrameToVideoWidgetThread(false, new QVideoFrame(microscopeVideoInterfaceBuffers.data()[1],*(new QSize(size)),pixelFormat));
		videoWidgetThread->addWidget(rightVideoWidget);
	}
	std::cout << "Microscope Navigation visualizeVideo called" << std::endl;


	m_toolFrameMicroscopeNavigationGUI.noCameraFoundTextLabel->setVisible(false);
	m_toolFrameMicroscopeNavigationGUI.numbeOfFoundCamerasTextLabel->setVisible(true);
	m_toolFrameMicroscopeNavigationGUI.numbeOfFoundCamerasNumberLabel->setVisible(true);

	m_toolFrameMicroscopeNavigationGUI.switchMicroscopeViewsPushButton->setEnabled(true);


	QString numberOfFoundCameras =QString("").append(QString::number(microscopeVideoInterfaceBuffers.size()));
	m_toolFrameMicroscopeNavigationGUI.numbeOfFoundCamerasNumberLabel->setText(numberOfFoundCameras);

	if(microscopeVideoInterfaceBuffers.size()<=0)
	{
		m_toolFrameMicroscopeNavigationGUI.noCameraFoundTextLabel->setVisible(true);

		m_toolFrameMicroscopeNavigationGUI.numbeOfFoundCamerasTextLabel->setVisible(false);
		m_toolFrameMicroscopeNavigationGUI.numbeOfFoundCamerasNumberLabel->setVisible(false);

	}

	if(microscopeVideoInterfaceBuffers.size()<=1)
	{
		m_toolFrameMicroscopeNavigationGUI.switchMicroscopeViewsPushButton->setEnabled(false);

	}

	showFoundCamerasInLeftComboBox();
	showFoundCamerasInRightComboBox();


}

//setQVideoFrameToVideoWidgetThread(true,new QVideoFrame(microscopeVideoInterfaceBuffers.data()[x],*buffersize,*bufferpixelFormat))
void microscopeNavigationPlugin::setQVideoFrameToVideoWidgetThread(bool isLeft,QVideoFrame* frame){
	if(isLeft){
		leftVideoWidget->videoSurface()->present(*frame);


	 }else{
		 rightVideoWidget->videoSurface()->present(*frame);

	 }
}


void microscopeNavigationPlugin::createWidgetsAndSignalSlotActions()
{
	microscopeNavigationInterfaceInstance = new microscopeNavigationInterface();

	toolFrameMicroscopeNavigationWidget= new QWidget();
	mainFrameVisualizationMicroscopeViewWidget= new QWidget();
	
	m_toolFrameMicroscopeNavigationGUI.setupUi(toolFrameMicroscopeNavigationWidget);
	m_mainFrameMicroscopeViewGUI.setupUi(mainFrameVisualizationMicroscopeViewWidget);

	leftVideoWidget = new VideoWidget();
	m_mainFrameMicroscopeViewGUI.microscopeViewDisplayLeftEyepiece->addWidget(leftVideoWidget);

	m_mainFrameMicroscopeViewGUI.microscopeViewDisplayLeftEyepiece->repaint();

	rightVideoWidget = new VideoWidget();
	m_mainFrameMicroscopeViewGUI.microscopeViewDisplayRightEyepiece->addWidget(rightVideoWidget);

	m_mainFrameMicroscopeViewGUI.microscopeViewDisplayRightEyepiece->repaint();


	microscopeNavigationInterfaceInstance->toolFrameMicroscopeNavigationInterfaceWidget= toolFrameMicroscopeNavigationWidget;
	microscopeNavigationInterfaceInstance->toolFrameMicroscopeNavigationInterfaceWidgetString="3    Microscope";


	microscopeNavigationInterfaceInstance->mainFrameVisualizationMicroscopeViewInterfaceWidget= mainFrameVisualizationMicroscopeViewWidget;
	microscopeNavigationInterfaceInstance->mainFrameVisualizationMicroscopeViewInterfaceWidgetString="Microscope View";


	ctkDictionary properties;
	properties["message"] = "toolFrameMicroscopeNavigationWidget";

	context->registerService<microscopeNavigationInterface>(microscopeNavigationInterfaceInstance, properties);
}


void microscopeNavigationPlugin::showFoundCamerasInLeftComboBox()
{
/*
	int i=1;
	for(std::vector<QAbstractVideoBuffer*>::iterator it= microscopeVideoInterfaceBuffers.begin(); it != microscopeVideoInterfaceBuffers.end(); ++it){}
*/
	int i;
	for(i= 1; i<= microscopeVideoInterfaceBuffers.size(); i++)
	{
		foundCameraNameLeft =QString("Camera : ").append(QString::number(i));

		m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->addItem(foundCameraNameLeft);
	}

	m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->setCurrentIndex(0);

	m_mainFrameMicroscopeViewGUI.displayedCameraOnLeftWidgetTextLabel->setText(QString("Camera : ").append(QString::number(i-2)));

}



void microscopeNavigationPlugin::showFoundCamerasInRightComboBox()
{
	int j;
	for(j= 1; j<= microscopeVideoInterfaceBuffers.size(); j++)
	{
		foundCameraNameRight =QString("Camera : ").append(QString::number(j));

		m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->addItem(foundCameraNameRight);
	}

	m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->setCurrentIndex(1);

	m_mainFrameMicroscopeViewGUI.displayedCameraOnRightWidgetTextLabel->setText(foundCameraNameRight);

}



void microscopeNavigationPlugin::switchMicroscopeViewsChangedState()
{

	if(m_toolFrameMicroscopeNavigationGUI.switchMicroscopeViewsPushButton->isChecked())
	{
		//m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->currentIndex();

		changeDisplayedViewOnLeftWidget(0);
		changeDisplayedViewOnRightWidget(1);

	}
	else
	{

		changeDisplayedViewOnLeftWidget(1);
		changeDisplayedViewOnRightWidget(0);
	}
}


void microscopeNavigationPlugin::changeDisplayedViewOnLeftWidget(int index)
{
/*
	//To disable an item
	modelIndexLeftComboBox = m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->model()->index(1,0); //diable item 1
	QVariant v3(0);
	m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->model()->setData(modelIndexLeftComboBox, v3, Qt::UserRole -1);
	m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->setCurrentIndex(1);
*/

	m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->setCurrentIndex(index);

	if(m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->currentIndex()== 0)
	{
		// Get the index of the value to enable
		modelIndexRightComboBox = m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->model()->index(1,0);
		QVariant v1(1);
		m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->model()->setData(modelIndexRightComboBox, v1, Qt::UserRole +1);
		m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->setCurrentIndex(1);
	}
	else
	{
		// Get the index of the value to enable
		modelIndexRightComboBox = m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->model()->index(0,0);
		QVariant v2(1);
		m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->model()->setData(modelIndexRightComboBox, v2, Qt::UserRole +1);
		m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->setCurrentIndex(0);

	}

	std::cout<<"left combobox current index: " << buffersize->width()<<" "<< buffersize->height()<<" " <<m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->currentIndex()<<std::endl;;

	this->setQVideoFrameToVideoWidgetThread(true, new QVideoFrame(microscopeVideoInterfaceBuffers.data()[index], *(new QSize(*buffersize)), *bufferpixelFormat));

	std::cout<<"cam "<<index<<" on left widget"<<std::endl;


}



void microscopeNavigationPlugin::changeDisplayedViewOnRightWidget(int index)
{

	m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->setCurrentIndex(index);


	if(m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->currentIndex()== 0)
	{
		// Get the index of the value to enable
		modelIndexLeftComboBox = m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->model()->index(1,0);
		QVariant v3(1);
		m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->model()->setData(modelIndexLeftComboBox, v3, Qt::UserRole +1);
		m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->setCurrentIndex(1);
	}
	else
	{
		// Get the index of the value to enable
		modelIndexLeftComboBox = m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->model()->index(0,0);
		QVariant v4(1);
		m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->model()->setData(modelIndexLeftComboBox, v4, Qt::UserRole +1);
		m_toolFrameMicroscopeNavigationGUI.microscopeLeftEyepieceComboBox->setCurrentIndex(0);
	}


   std::cout<<"right combobox current index: " <<m_toolFrameMicroscopeNavigationGUI.microscopeRightEyepieceComboBox->currentIndex()<<std::endl;;

   this->setQVideoFrameToVideoWidgetThread(false, new QVideoFrame(microscopeVideoInterfaceBuffers.data()[index], *(new QSize(*buffersize)), *bufferpixelFormat));

   std::cout<<"cam "<<index<<" on right widget"<<std::endl;

}


void microscopeNavigationPlugin::stopCaptureVideo()
{
	leftVideoWidget->finishEncoder();
	rightVideoWidget->finishEncoder();
}

void microscopeNavigationPlugin::startCaptureVideo()
{

	QString fileName = QFileDialog::getSaveFileName(NULL, "Save Video as...", QDir::currentPath(), "Video (*.avi)");
	QFileInfo fileInfo(fileName);
	QString secondPart = fileInfo.completeSuffix();
	QString firstPart;
	    if (!secondPart.isEmpty()) {
	        secondPart = "." + secondPart;
	        firstPart = fileName.left(fileName.size() - secondPart.size());
	    } else {
	        firstPart = fileName;
	        secondPart = ".avi";
	    }
	    QString leftFilename= QString(firstPart +  "_left" +secondPart);
	    QString rightFilename= QString(firstPart + "_right" +secondPart);

    std::cout<<"start recording video to "<<leftFilename.toStdString()<<" and "<<rightFilename.toStdString()<<std::endl;

    std::cout<<buffersize->width()<<" "<<buffersize->height()<<std::endl;
	QVideoEncoder* leftEncoder = new QVideoEncoder();
	leftEncoder->createFile(leftFilename,buffersize->width(),buffersize->height(),1000000,20,FRAME_RATE);
	leftVideoWidget->setEncoder(leftEncoder);

	QVideoEncoder* rightEncoder = new QVideoEncoder();
	rightEncoder->createFile(rightFilename,buffersize->width(),buffersize->height(),1000000,20,FRAME_RATE);
	rightVideoWidget->setEncoder(rightEncoder);

}

Q_EXPORT_PLUGIN2(at_voxelmaster_microscopeNavigationPlugin, microscopeNavigationPlugin)


