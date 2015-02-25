
#include <QWidget>


class trackerNavigationInterface : public QObject
  {
	 Q_OBJECT


public:

	QWidget															*toolFrameTrackerNavigationInterfaceWidget;
	QString 														toolFrameTrackerNavigationWidgetString;

	QWidget*														mainFrameVisualizationTrackerNavigationViewInterfaceWidget;
	QString															mainFrameVisualizationTrackerNavigationViewInterfaceWidgetString;



};

Q_DECLARE_INTERFACE(trackerNavigationInterface, "trackerNavigationInterface");
