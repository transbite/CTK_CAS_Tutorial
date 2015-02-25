

#ifndef __mainWindowGUI_h
#define __mainWindowGUI_h


#include "ui_mainWindowGUI.h"

#include <sys/stat.h>
#include <string>
#include <sstream>
#include <cstring>
#include <stdio.h>


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
#include "QVTKWidget.h"
#include <QPushButton>
#include <QAbstractButton>
#include <QButtonGroup>



class mainWindowGUI : public QMainWindow
{
	Q_OBJECT


public:

	mainWindowGUI(QWidget *parent= 0);

	~mainWindowGUI();

	void createSignalSlotActions();
	bool hasQuitted();


	Ui::MainWindow* getMainWindowGUI();


public slots:

	void addFiducial();
	void test(QAbstractButton *pushButton);
	void toolBoxCurrentChanged(const int index);
	void openVOXELMASTERSite();
	void requestPrepareToQuit();
	void closeEvent(QCloseEvent* event);



protected:



private:


	Ui::MainWindow												m_MainWindowGUI;


	bool														m_MainWindowGUIQuit;
	QPushButton 												*pushButton;
	QButtonGroup 												*buttongroup;

	int															maxFiducials;
	int															numberOfFiducials;

	int 														firstRow;
	int															secondRow;
	int															thirdRow;
	int 														fourthRow;
	int 														fifthRow;

	int 														firstColumn;
	int 														secondColumn;
	int 														thirdColumn;
	int 														fourthColumn;
	int 														fifthColumn;
	int 														sixthColumn;

	QMessageBox 												*m_messageDialog;

};

#endif
