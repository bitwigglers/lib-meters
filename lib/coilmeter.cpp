#include "coilmeter.h"

#include <QPainter>
#include <cmath>

CoilMeter::CoilMeter(QWidget *parent)
    : AlarmMeter(parent)
{
    init();
}

CoilMeter::CoilMeter(QWidget *parent, qreal min, qreal max)
    : AlarmMeter(parent, min, max)
{
    init();
}

CoilMeter::~CoilMeter()
{
}

void CoilMeter::init()
{
    QSizePolicy s(QSizePolicy::Expanding, QSizePolicy::Expanding);
    s.setHeightForWidth(true);
    setSizePolicy(s);

    _overlayEnabled = true;
    _precision = 1;
    _unit = "";
    _style = StyleBar;

    connect(&offsetFontTimer, SIGNAL(timeout()), this, SLOT(setOffsetFontColor()));
    offsetFontTimer.start(1000);
}

QSize CoilMeter::sizeHint() const
{
    return QSize(320, 240);
}

QSize CoilMeter::minimumSizeHint() const
{
    return QSize(120, 90);
}

QSize CoilMeter::minimumSize() const
{
    return QSize(80, 60);
}

int CoilMeter::heightForWidth(int w) const
{
    return (w * 3) / 4;
}

void CoilMeter::setNeedleStyle(enum NeedleStyle style)
{
    _style = style;
    update();
}

enum CoilMeter::NeedleStyle CoilMeter::needleStyle() const
{
    return _style;
}

void CoilMeter::setOverlayEnabled(bool enable)
{
    _overlayEnabled = enable;
    update();
}

void CoilMeter::setPrecision(int prec)
{
    if (prec > 0)
    _precision = prec;
    update();
}

int CoilMeter::precision()
{
    return _precision;
}

void CoilMeter::setUnit(const QString& string)
{
    _unit = string;
    _unit.prepend(' ');
}

bool CoilMeter::overlayEnabled() const
{
    return _overlayEnabled;
}

QRect CoilMeter::findRect(const QRect& r)
{
    int w = r.width();
    int h = r.height();

    // scale the rect to 4:3 aspect ratio
    h = qMin(h, (3 * w) / 4);
    w = (4 * h) / 3;

    // center the rect and return it
    return(QRect((r.width() - w) / 2, (r.height() - h) / 2, w, h));
}

void CoilMeter::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QColor cv = valueColor();
    QColor cw = warnColor();
    QColor ca = alarmColor();

    if (_style != StyleNeedle) {
        cv.setAlpha(127);
        cw.setAlpha(127);
        ca.setAlpha(127);
    }

    // find the biggest possible centered rect with an aspect ratio of 4:3
    // that fits within current width and height
    QRect r = findRect(QRect(0, 0, width(), height()));
    painter.setViewport(r);
    painter.setWindow(-205, -155, 410, 310);

    int cornerRadius = 10;
    // draw housing
    painter.setBrush(palette().alternateBase());
    painter.drawRoundRect(-200, -150, 400, 300, cornerRadius, cornerRadius);

    // draw scale background
    painter.setBrush(palette().base());
    QRect scaleBackgroundRect(-180, -130, 360, 190);
    painter.drawRoundRect(scaleBackgroundRect, cornerRadius, cornerRadius);
    qreal vcenter = 55;

    // draw user string on the housing
    painter.setFont(_housingFont);
    painter.drawText(-180, 60, 360, 90, Qt::AlignLeft | Qt::AlignVCenter, _housingText);

    // the rest (except the overlay) is drawn a bit larger to match the proportions of an analog meter
    painter.save();
    painter.translate(0, 60);
    painter.scale(4.0/3.0, 4.0/3.0);

    // draw minor ticks
    painter.save();
    painter.setBrush(palette().windowText());
    painter.rotate(-60);
    qreal x = minimum();
    qreal angle = (maximum() - minimum()) / minorTicks();
    angle = 120 / angle;
    while (x <= maximum()) {
        painter.drawLine(0, -130, 0 , -120);
        painter.rotate(angle);
        x += minorTicks();
    }
    painter.restore();

    // draw major ticks
    painter.save();
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    painter.setBrush(palette().windowText());
    painter.rotate(-60);
    x = minimum();
    angle = (maximum() - minimum()) / majorTicks();
    angle = 120 / angle;
    while (x <= maximum()) {
        painter.drawLine(0, -130, 0 , -110);
        painter.drawText(-15, -105, 30, 20, Qt::AlignHCenter | Qt::AlignTop, QString::number(x));
        painter.rotate(angle);
        x += majorTicks();
    }
    painter.restore();

    // draw value
    if (_style == StyleNeedle) {
        painter.save();

        angle = ((120 * (value() - minimum())) / (maximum() - minimum())) - 60;
        painter.rotate(angle);

        painter.setClipRect(-200, -150, 400, 120);

        if (value() < warnValue())
        painter.setBrush(QBrush(cv));
        else if (value() >= warnValue() && value() < alarmValue())
        painter.setBrush(QBrush(cw));
        else
        painter.setBrush(QBrush(ca));

        painter.setPen(Qt::PenStyle());
        static const int needle[3][2] = {{-10, (int)vcenter}, {0, -130}, {10, (int)vcenter}};
        painter.drawPolygon(QPolygon(3, &needle[0][0]));
        painter.restore();
    } else {
        painter.save();

        angle = ((120 * (value() - minimum())) / (maximum() - minimum()));

        if (_style == StyleBar) {
            QPen pen;
            if (value() < warnValue())
            pen.setColor(cv);
            else if (value() >= warnValue() && value() < alarmValue())
            pen.setColor(cw);
            else
            pen.setColor(ca);

            pen.setWidth(15);
            pen.setCapStyle(Qt::FlatCap);
            painter.setPen(pen);
            painter.drawArc(QRectF(-sqrtf(-123 * -123), -123, 2 * sqrtf(-123 * -123), 246), 150 * 16, (int)(-angle * 16));

            if (value() < warnValue())
            pen.setColor(valueColor());
            else if (value() >= warnValue() && value() < alarmValue())
            pen.setColor(warnColor());
            else
            pen.setColor(alarmColor());
            painter.setPen(pen);

            QFont font = painter.font();
            font.setPixelSize(18);
            font.setBold(true);
            painter.setFont(font);
            QString text = QString::number(value(), 'f', _precision);
            text += _unit;
            painter.drawText(-70, -55, 140, 50, Qt::AlignCenter, text);
            painter.restore();

        } else if (_style == StyleFixedGradient || _style == StyleFlowGradient) {
            QConicalGradient grad(0, 0, 30);
            // calculate the gradient stops for warn and alarm level
            qreal alarm = (alarmValue() - minimum()) / (maximum() - minimum());
            alarm = 0.333 - alarm * 0.333;
            qreal warn = (warnValue() - minimum()) / (maximum() - minimum());
            warn = 0.333 - warn * 0.333;

            if (_style == StyleFlowGradient) {
                grad.setColorAt(0, ca);
                grad.setColorAt(alarm, ca);
                grad.setColorAt(warn, cw);
                grad.setColorAt(0.333, cv);
            } else {
                grad.setColorAt(0, ca);
                grad.setColorAt(alarm - 0.000000000001, ca);
                grad.setColorAt(alarm, cw);
                grad.setColorAt(warn - 0.0000000000001, cw);
                grad.setColorAt(warn, cv);
                grad.setColorAt(0.333, cv);
            }

            QPen pen;
            pen.setWidth(15);
            pen.setCapStyle(Qt::FlatCap);
            pen.setBrush(QBrush(grad));painter.setPen(pen);
            painter.drawArc(QRectF(-sqrtf(-123 * -123), -123, 2 * sqrtf(-123 * -123), 246), 150 * 16, (int)(-angle * 16));

            if (value() < warnValue())
            pen.setColor(valueColor());
            else if (value() >= warnValue() && value() < alarmValue())
            pen.setColor(warnColor());
            else
            pen.setColor(alarmColor());
            painter.setPen(pen);

            QFont font = painter.font();
            font.setPixelSize(18);
            font.setBold(true);
            painter.setFont(font);
            QString text = QString::number(value(), 'f', _precision);
            text += _unit;
            painter.drawText(-70, -55, 140, 50, Qt::AlignCenter, text);
            painter.restore();
        }
    }

    // draw needle origin
    if (_style == StyleNeedle) {
        painter.save();
        painter.setBrush(palette().alternateBase());
        painter.setClipRect(-80, -80, 160, 80);

        //painter.setBrush(QBrush());
        painter.drawRoundedRect(-70, -30, 140, 30 + cornerRadius,  + cornerRadius,  + cornerRadius);
        QPen pen;
        if (value() < warnValue())
        pen.setColor(cv);
        else if (value() >= warnValue() && value() < alarmValue())
        pen.setColor(cw);
        else
        pen.setColor(ca);
        painter.setPen(pen);

        QFont font = painter.font();
        font.setPixelSize(16);
        font.setBold(true);
        painter.setFont(font);
        QString text = QString::number(value(), 'f', _precision);
        text += _unit;
        painter.drawText(-70, -30, 140, 30, Qt::AlignCenter, text);

        painter.restore();
    }

    // paint offset value
    if (!qFuzzyCompare(1 + offset(), 1 + 0.0)) {
        QFont f = painter.font();
        f.setPixelSize(10);
        f.setBold(true);
        painter.setFont(f);
        QPen pen = painter.pen();
        pen.setColor(_offsetColor);
        painter.setPen(pen);
        QRect r2(70, -30, 65, 30);
        painter.drawText(r2, Qt::AlignCenter, "Offset:\n" + QString::number(offset(), 'f', _precision));
    }

    // paint reference value if enabled
    if (refEnabled()) {
        QFont f = painter.font();
        f.setPixelSize(10);
        f.setBold(true);
        painter.setFont(f);
        QPen pen = painter.pen();
        pen.setColor(Qt::black);
        painter.setPen(pen);
        QRect r2(-135, -30, 65, 30);
        painter.drawText(r2, Qt::AlignCenter, "Ref:\n" + QString::number(refValue() + offset(), 'f', _precision) + " dB");
    }

    painter.restore();
    // draw overlay
    if (_overlayEnabled) {
        QRegion region = constructOverlayRegion(scaleBackgroundRect, cornerRadius);
        painter.setClipRegion(region);

        QLinearGradient g(0, 0, 1, 1);
        g.setCoordinateMode(QGradient::ObjectBoundingMode);
        g.setColorAt(0, QColor(255, 255, 255, 127));
        g.setColorAt(0.5, QColor(255, 255, 255, 0));
        QBrush b(g);
        painter.fillRect(scaleBackgroundRect, b);
    }
}

static QRegion roundedRectRegion(const QRect& rect, int r)
{
    QRegion region;
    // middle and borders
    region += rect.adjusted(r, 0, -r, 0);
    region += rect.adjusted(0, r, 0, -r);
    // top left
    QRect corner(rect.topLeft(), QSize(r*2, r*2));
    region += QRegion(corner, QRegion::Ellipse);
    // top right
    corner.moveTopRight(rect.topRight());
    region += QRegion(corner, QRegion::Ellipse);
    // bottom left
    corner.moveBottomLeft(rect.bottomLeft());
    region += QRegion(corner, QRegion::Ellipse);
    // bottom right
    corner.moveBottomRight(rect.bottomRight());
    region += QRegion(corner, QRegion::Ellipse);
    return region;
}

QRegion CoilMeter::constructOverlayRegion(const QRect& rect, int radius)
{
    QRegion region = roundedRectRegion(rect, radius);

    QRect ellipseRect = rect.adjusted(rect.width() / 20, rect.height() / 10, rect.width(), rect.height());
    QRegion ellipseRegion(ellipseRect,QRegion::Ellipse);
    return region.subtracted(ellipseRegion);
}

#include <QDebug>
void CoilMeter::setOffsetFontColor()
{
    _offsetOdd = _offsetOdd ? false : true;
    _offsetColor = _offsetOdd ? Qt::black : alarmColor();
    update();
}

