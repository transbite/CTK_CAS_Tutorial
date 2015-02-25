
#include <QWidget>


class microscopeNavigationInterface : public QObject
  {
	 Q_OBJECT


public:
	
	QWidget         *toolFrameMicroscopeNavigationInterfaceWidget;
	QString			toolFrameMicroscopeNavigationInterfaceWidgetString;

	QWidget			*mainFrameVisualizationMicroscopeViewInterfaceWidget;
	QString			mainFrameVisualizationMicroscopeViewInterfaceWidgetString;


};

Q_DECLARE_INTERFACE(microscopeNavigationInterface, "microscopeNavigationInterface");
