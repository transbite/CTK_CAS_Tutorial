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


#ifndef MicroscopeNavigationPluginActivator_H
#define MicroscopeNavigationPluginActivator_H

#include "ui_toolFrameMicroscopeNavigationGUI.h"
#include "ui_mainFrameMicroscopeViewGUI.h"


#include "microscopeNavigationInterface.h"

#include "microscopeVideoInterface.h"


/** QT header files */
#include <QString>
#include <QFileInfo>
#include <QtGui>
#include <QObject>
#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QtCore/QTimer>
#include <QDesktopServices>
#include <QDialog>
#include <QFileDialog>
//#include "QVTKWidget.h"
#include <QPushButton>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QWidget>
#include <QMediaPlayer>
#include <QtMultimediaKit/QMediaRecorder>
#include <QAudioCaptureSource>
#include <QModelIndex>

#include <ctkPluginActivator.h>
#include <ctkServiceTracker.h>
#include <service/event/ctkEventHandler.h>
#include <service/event/ctkEvent.h>
#include <VideoWidget.h>
#include <QThread>
#include <iostream>
#include <fstream>

#include "QVideoEncoder.h"

#define FRAME_RATE 25



class RefreshWidgetThread : public QThread{
	Q_OBJECT
	public:
		RefreshWidgetThread(){

		}

		void addWidget(QWidget* refreshWidget){
			connect(this, SIGNAL(refresh()),
								refreshWidget, SLOT(repaint()));
		}

	private:



	 void run(){
	 		 std::cout<<"Thread RUN RefreshWidgetThread.h"<<std::endl;

	 		 while(true){


	 			emit refresh();

	 			usleep( 1000000/FRAME_RATE );
	 		 }
	 }

	 signals:
	     void refresh();

};

class microscopeNavigationPlugin :  public QObject,  public ctkPluginActivator
{
	Q_OBJECT
	Q_INTERFACES(ctkPluginActivator)

public:

	microscopeNavigationPlugin();
	~microscopeNavigationPlugin();

	void start(ctkPluginContext* context);
	void stop(ctkPluginContext* context);

	static microscopeNavigationPlugin* getInstance();

	ctkPluginContext* getPluginContext() const;


	void createWidgetsAndSignalSlotActions();
	void visualizeVideo(std::vector<QAbstractVideoBuffer*> microscopeVideoInterfaceBuffers, QSize size,QVideoFrame::PixelFormat pixelFormat);


	Ui::toolFrameMicroscopeNavigationGUI				m_toolFrameMicroscopeNavigationGUI;
	Ui::mainFrameMicroscopeViewGUI						m_mainFrameMicroscopeViewGUI;


public slots:

	void startCaptureVideo();
	void stopCaptureVideo();
	void showFoundCamerasInLeftComboBox();
	void showFoundCamerasInRightComboBox();
	void changeDisplayedViewOnLeftWidget(int index);
	void changeDisplayedViewOnRightWidget(int index);
	void switchMicroscopeViewsChangedState();

private:


	static microscopeNavigationPlugin					*instance;
	ctkPluginContext									*context;

	microscopeNavigationInterface						*microscopeNavigationInterfaceInstance;
	
	QWidget												*toolFrameMicroscopeNavigationWidget;
	QWidget												*mainFrameVisualizationMicroscopeViewWidget;

	VideoWidget											*leftVideoWidget;
	VideoWidget											*rightVideoWidget;

	RefreshWidgetThread									*videoWidgetThread;


	std::vector<QAbstractVideoBuffer*> 					microscopeVideoInterfaceBuffers;
	QSize												*buffersize;
	QVideoFrame::PixelFormat							*bufferpixelFormat;

	QString 											foundCameraNameLeft;
	QString 											foundCameraNameRight;


	QModelIndex 										modelIndexLeftComboBox;
	QModelIndex 										modelIndexRightComboBox;
/*
	QStandardItemModel									*m_leftCameraComboBoxItemModel;
	QModelIndex 										m_leftCameraComboBoxIndex;
	QStandardItem										*m_leftCameraComboBoxItem;

	QStandardItemModel									*m_rightCameraComboBoxItemModel;
	QModelIndex 										m_rightCameraComboBoxIndex;
	QStandardItem										*m_rightCameraComboBoxItem;
*/

	void initMicroscopeVideoTracker();
	void setQVideoFrameToVideoWidgetThread(bool isLeft,QVideoFrame* frame);

}; // MicroscopeNavigationPluginActivator

#endif // MicroscopeNavigationPluginActivator_H
