#ifndef MAIN_H
#define MAIN_H

#include <QVBoxLayout>
#include <QTimer>
#include <QWidget>

#include <coilmeter.h>

class Widget : public QWidget
{
	Q_OBJECT
public:
	Widget(QWidget *parent);
private:
	CoilMeter *needle, *bar, *gradBar, *flowBar;
//	HorizontalScale *hneedle, *hbar, *hgradBar, *hflowBar;
	QGridLayout * layout;
	QVBoxLayout * vlayout;
	QHBoxLayout *hlayout;
	QTimer *timer;
private slots:
	void updateScales();
};

#endif // MAIN_H
