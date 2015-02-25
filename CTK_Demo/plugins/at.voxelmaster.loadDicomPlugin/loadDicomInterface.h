
#include <QWidget>


class loadDicomInterface : public QObject
  {
	 Q_OBJECT


public:

	QWidget*		toolFrameLoadDicomInterfaceWidget;
	QString			toolFrameLoadDicomInterfaceWidgetString;

	QWidget*		mainFrameVisualizationDicomViewInterfaceWidget;
	QString			mainFrameVisualizationDicomViewInterfaceWidgetString;

};

Q_DECLARE_INTERFACE(loadDicomInterface, "loadDicomInterface");
