/*
 * WaveWidget2 : 単純な波形を入出力
 *
 * Fourieで使ってるのより高性能
 *
 */

#ifndef WAVEWIDGET_H
#define WAVEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QVector>
#include <QPointF>
#include <QMouseEvent>
#include <math.h>

class WaveWidget : public QWidget
{
    Q_OBJECT
public:
    enum DrawStyle {
        dsPlain,
        dsScale,
        dsPlainRepeat,
    };

    enum LineStyle {
        lsLine,
        lsDot,
        lsBar,
    };

    enum YScaleMode {
        ymExpand,
        ymManual,
    };

    enum InputMode {
        imDirect,
        imLiner,
    };

    enum ZeroMode {
        zmBottom,
        zmCenter,
        zmCustom,
    };

public:
    explicit WaveWidget(QWidget *parent = 0);

    void setWaveLine( const QPen &pen, const QColor &bgColor, LineStyle lineStyle );
    void setDrawStyle( DrawStyle style );
    void setInputMode( InputMode mode );
    void setZeroMode( ZeroMode mode );
    void setCustomZeroLevel( int level );
    void setWave( const QVector<qreal> &wave );
    void setEditable( bool editable = true );
    void setDrawCenter( bool drawCenter = true );
    void setManualYScale( qreal scale = 1.0 );
    void setYScaleMode( YScaleMode mode );
    void setExpandYLevel( qreal level );
    QVector<qreal> &getWave( void );

    qreal getWaveY( int widgetY );
    int getWidgetY( qreal waveY );
    qreal getYScale( void );

protected:
    virtual void paintEvent( QPaintEvent * );
    virtual void mouseMoveEvent( QMouseEvent * event );
    virtual void resizeEvent( QResizeEvent * );

private:
    QPen pen;
    QColor bgColor;
    DrawStyle style;
    LineStyle lineStyle;
    InputMode inputMode;
    YScaleMode yScaleMode;
    ZeroMode zeroMode;
    QVector<qreal> wave;
    QPoint beforePoint;
    bool editable;
    bool drawCenter;
    bool drawing;
    int customZeroLevel;
    qreal manualYScale;
    qreal expandYLevel;

signals:
    void waveChanged();
    void waveChangedByHand();

public slots:

};

#endif // WAVEWIDGET_H
