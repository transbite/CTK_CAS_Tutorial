
#include "mainWindowGUI.moc"
#include "mainWindowGUI.h"

#ifdef _WIN32

#include <direct.h>

#endif




/** Constructor */
mainWindowGUI::mainWindowGUI(QWidget *parent) : QMainWindow(parent)
{
	m_MainWindowGUI.setupUi(this);

	createSignalSlotActions();


	m_MainWindowGUIQuit= false;
/*
	maxFiducials= 0;

	firstRow= 0;
	secondRow= 0;
	thirdRow= 0;
	fourthRow= 0;
	fifthRow= 0;

	firstColumn= 0;
	secondColumn= 1;
	thirdColumn= 2;
	fourthColumn= 3;
	fifthColumn= 4;
	sixthColumn= 5;
*/

	//m_MainWindowGUI.documentTreeToolBox->setCurrentIndex(0);


	m_MainWindowGUI.mainViewProgressBar->setVisible(false);
	m_MainWindowGUI.mainWindowStatusBar->addPermanentWidget(m_MainWindowGUI.mainViewProgressBar, 1);

	// Remove initial pages from ToolBox and StackedWidget, that created via QtDesigner
	m_MainWindowGUI.documentTreeToolBox->removeItem(0);
	m_MainWindowGUI.mainViewStackedWidget->removeWidget(0);


/*
	m_MainWindowGUI.pb1->setVisible(false);
	m_MainWindowGUI.pb2->setVisible(false);
	m_MainWindowGUI.pb3->setVisible(false);
	m_MainWindowGUI.pb4->setVisible(false);
	m_MainWindowGUI.pb5->setVisible(false);
	m_MainWindowGUI.pb6->setVisible(false);
	m_MainWindowGUI.pb11->setVisible(false);
	m_MainWindowGUI.pb16->setVisible(false);
	m_MainWindowGUI.pb21->setVisible(false);
*/

}

/** Destructor */
mainWindowGUI::~mainWindowGUI()
{

}

Ui::MainWindow* mainWindowGUI::getMainWindowGUI(){
	return &m_MainWindowGUI;
}

/** Connect GUI elements with actions */
void mainWindowGUI::createSignalSlotActions()
{
	//connect(m_MainWindowGUI.addFiducialPushButton, SIGNAL(clicked()), this, SLOT(addFiducial()));

	connect(m_MainWindowGUI.pushButtonLogo, SIGNAL(clicked()), this, SLOT(openVOXELMASTERSite()));
	connect(m_MainWindowGUI.documentTreeToolBox, SIGNAL(currentChanged(int)), this, SLOT(toolBoxCurrentChanged(const int)));

}



void mainWindowGUI::toolBoxCurrentChanged(const int index)
{
	if(m_MainWindowGUI.documentTreeToolBox->currentIndex()== 0)
	{
		m_MainWindowGUI.mainViewStackedWidget->setCurrentIndex(1);
	}

	if(m_MainWindowGUI.documentTreeToolBox->currentIndex()== 1)
	{

		m_MainWindowGUI.mainViewStackedWidget->setCurrentIndex(2);

	}

	if(m_MainWindowGUI.documentTreeToolBox->currentIndex()== 2)
	{
		m_MainWindowGUI.mainViewStackedWidget->setCurrentIndex(3);
	}

}


void mainWindowGUI::addFiducial()
{

/*
	pushButton = new QPushButton();
	//pushButton->setText("1");
	pushButton->setFixedSize(30, 30);

	QFont font = pushButton->font();
	font.setPointSize(8);
	pushButton->setFont(font);


	if(maxFiducials!=25)
	{

		if(firstRow< 6)
		{
			m_MainWindowGUI.gridLayout_3->addWidget(pushButton, firstRow, firstColumn);
			//setLayout(m_MainWindowGUI.gridLayout_3);
			firstRow++;
			cout<<"firstRow :"<< firstRow<<endl;
		}

		if(firstRow>= 6)
		{
			m_MainWindowGUI.gridLayout_3->addWidget(pushButton, secondRow, secondColumn);
			secondRow++;
			cout<<"secondRow :"<< secondRow<<endl;
		}

		if(secondRow>= 6)
		{
			m_MainWindowGUI.gridLayout_3->addWidget(pushButton, thirdRow, thirdColumn);
			thirdRow++;
			cout<<"thirdRow :"<< thirdRow<<endl;
		}

		if(thirdRow>= 6)
		{
			m_MainWindowGUI.gridLayout_3->addWidget(pushButton, fourthRow, fourthColumn);
			fourthRow++;
			cout<<"fourthRow :"<< fourthRow<<endl;
		}

		if(fourthRow>=6 && fifthRow!=5)
		{
			m_MainWindowGUI.gridLayout_3->addWidget(pushButton, fifthRow, fifthColumn);
			fifthRow++;
			cout<<"fifthRow :"<< fifthRow<<endl;
		}

	maxFiducials++;

	cout<<"maxFiducials: "<<maxFiducials<<endl;

	pushButton->setText(QString::number(maxFiducials));

	buttongroup = new QButtonGroup(this);
	buttongroup->addButton(pushButton);
	buttongroup->setId(pushButton, maxFiducials);


	}
	else
	{
		m_messageDialog->information(NULL, "Information", "You have reached maximum fiducial number! \nPlease click on finish button.",  QMessageBox::Ok);


	}

	//connect(pushButton , SIGNAL(clicked()), this, SLOT(test()));
	connect(buttongroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(test(QAbstractButton*)));
*/
}




void mainWindowGUI::test(QAbstractButton *pushButton)
{

	qDebug()<<"here"<<pushButton->text()<< endl;


	if(pushButton->text()== "1")
	{
		qDebug()<<"Clicked on Button 1, set number of fiducial to 1 and call other classes"<<pushButton->text()<< endl;
	}


	if(pushButton->text()== "2")
	{
		qDebug()<<"Clicked on Button 2, set number of fiducial to 2 and call other classes"<<pushButton->text()<< endl;
	}



}


/** Open http://www.voxelmaster.at in standard browser */
void mainWindowGUI::openVOXELMASTERSite()
{
	QDesktopServices::openUrl(QUrl("http://www.voxelmaster.at", QUrl::TolerantMode));
}


/** Show quit dialog */
void mainWindowGUI::requestPrepareToQuit()
{
	QMessageBox::StandardButton value= QMessageBox::question(this, "StoS", "Are you sure you want to quit?",
									   QMessageBox::Cancel | QMessageBox::Ok);
	if(value == QMessageBox::Ok)
	{
		m_MainWindowGUIQuit = true;

		QTimer::singleShot(0, QApplication::instance(), SLOT(quit()));
	}
}




void mainWindowGUI::closeEvent(QCloseEvent* event)
{
	QMessageBox::StandardButton value= QMessageBox::question(this, "CTK_Demo", "Are you sure you want to quit?",
									   QMessageBox::Cancel | QMessageBox::Ok );
	if(value == QMessageBox::Ok)
	{
		m_MainWindowGUIQuit = true;
		event->accept();
	}
	else
	{
		event->ignore();
	}
}




/** Ask if already quitted */
bool mainWindowGUI::hasQuitted()
{
	return m_MainWindowGUIQuit;
}

