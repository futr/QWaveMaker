#include "wavewidget.h"

WaveWidget::WaveWidget( QWidget *parent ) :
    QWidget( parent )
{
    // 初期化
    editable   = false;
    drawCenter = false;
    drawing    = false;
    lineStyle  = lsLine;
    inputMode  = imLiner;
    zeroMode   = zmCenter;
    yScaleMode = ymManual;
    style      = dsPlainRepeat;
    manualYScale    = 1.0;
    expandYLevel    = 1.0;
    customZeroLevel = 0;
    wave.resize( 0 );

    // マウストラッキング有効化
    setMouseTracking( true );
}

void WaveWidget::setWaveLine( const QPen &pen, const QColor &bgColor , LineStyle lineStyle )
{
    // 線の色と背景色を設定
    this->pen = pen;
    this->bgColor = bgColor;
    this->lineStyle = lineStyle;
}

void WaveWidget::setDrawStyle( WaveWidget::DrawStyle style )
{
    // 描画方法を指定
    this->style = style;
}

void WaveWidget::setInputMode( WaveWidget::InputMode mode )
{
    // 入力方法を指定
    inputMode = mode;
}

void WaveWidget::setZeroMode( WaveWidget::ZeroMode mode )
{
    // ゼロ位置の決定方法を指定
    zeroMode = mode;
}

void WaveWidget::setCustomZeroLevel( int level )
{
    // カスタムゼロレベル指定
    customZeroLevel = level;
}

void WaveWidget::setWave( const QVector<qreal> &wave )
{
    // 波を設定
    this->wave = wave;

    // 再描画
    repaint();

    // 変更を教える
    emit waveChanged();
}

void WaveWidget::setEditable( bool editable )
{
    // マウスで波形をいじれるか決める
    this->editable = editable;
}

void WaveWidget::setDrawCenter( bool drawCenter )
{
    // 中心線を書く
    this->drawCenter = drawCenter;
}

void WaveWidget::setManualYScale( qreal scale )
{
    // Yの拡大率
    manualYScale = scale;
}

void WaveWidget::setYScaleMode( WaveWidget::YScaleMode mode )
{
    // Yの拡大方法指定
    yScaleMode = mode;
}

void WaveWidget::setExpandYLevel( qreal level )
{
    // ウィジェットの高さと一致するレベル
    expandYLevel = level;
}

QVector<qreal> &WaveWidget::getWave()
{
    // 現在の波を返す
    return wave;
}

qreal WaveWidget::getWaveY( int widgetY )
{
    // Widget上座標からwave上のY座標に変換
    qreal yScale = getYScale();

    switch ( zeroMode ) {
    case zmCenter:
        return -( widgetY - height() / 2 ) / yScale;
    case zmBottom:
        return -( widgetY - height() ) / yScale;
    case zmCustom:
        return -( widgetY - ( height() - customZeroLevel ) ) / yScale;
    default:
        return 0;
    }
}

int WaveWidget::getWidgetY( qreal waveY )
{
    // wave上座標からWidget上のY座標に変換
    qreal yScale = getYScale();

    switch ( zeroMode ) {
    case zmCenter:
        return -waveY * yScale + height() / 2;
    case zmBottom:
        return -waveY * yScale + height();
    case zmCustom:
        return -waveY * yScale + height() - customZeroLevel;
    default:
        return 0;
    }
}

qreal WaveWidget::getYScale( void )
{
    // 現在のスケールモードからスケールを計算
    qreal scale;

    if ( yScaleMode == ymExpand ) {
        scale = height() / expandYLevel;
    } else {
        scale = manualYScale;
    }

    return scale;
}

void WaveWidget::paintEvent( QPaintEvent */*event*/ )
{
    // 描画
    QPainter painter( this );
    QVector<QPointF> dwave( width() );
    int i;

    // 波がなければ何もしない
    if ( !wave.size() ) {
        return;
    }

    // 色を設定
    QBrush brush( bgColor );
    painter.setPen( pen );
    painter.setBrush( brush );

    // クリア
    painter.fillRect( rect(), brush );

    // 描画用の波を作る
    if ( style == dsPlain ) {
        // そのまま
        for ( i = 0; i < dwave.size(); i++ ) {
            if ( i < wave.size() ) {
                dwave[i].setX( i );
                dwave[i].setY( getWidgetY( wave[i] ) );
            } else {
                dwave[i].setX( i );
                dwave[i].setY( 0 );
            }
        }
    } else if ( style == dsScale ) {
        // X方向に拡大
        dwave.resize( wave.size() );

        for ( i = 0; i < wave.size(); i++ ) {
            // ceilで丸めないとlsBarのときにバー間に隙間が生まれてしまう
            dwave[i].setX( ceil( (qreal)i / wave.size() * width() ) );
            dwave[i].setY( getWidgetY( wave[i] ) );
        }
    } else if ( style == dsPlainRepeat ) {
        // そのままだけど繰り返す
        for ( i = 0; i < dwave.size(); i++ ) {
            dwave[i].setX( i );
            dwave[i].setY( getWidgetY( wave[i % wave.size()] ) );
        }
    }

    // 必要なら中心線を書く
    if ( drawCenter ) {
        QPen npen = painter.pen();

        painter.save();

        npen.setWidth( 1 );
        npen.setStyle( Qt::DashLine );
        painter.setPen( npen );

        painter.drawLine( 0, getWidgetY( 0 ), width(), getWidgetY( 0 ) );

        painter.restore();
    }

    // 波を書く
    if ( lineStyle == lsLine ) {
        painter.drawPolyline( dwave );
    } else if ( lineStyle == lsDot ) {
        painter.drawPoints( dwave );
    } else if ( lineStyle == lsBar ) {
        QRectF barRect;
        int barWidth;

        // ２点以上なければ書けない
        if ( dwave.size() < 2 ) {
            return;
        }

        for ( i = 0; i < dwave.size(); i++ ) {
            // バーの幅を計算
            if ( i < dwave.size() - 1 ) {
                barWidth = dwave[i + 1].x() - dwave[i].x();
            } else {
                barWidth = dwave[i].x() - dwave[i - 1].x();
            }

            if ( getWidgetY( 0 ) > dwave[i].y() ) {
                barRect.setRect( dwave[i].x(), dwave[i].y(), barWidth, qAbs( getWidgetY( 0 ) - dwave[i].y() ) );
            } else {
                barRect.setRect( dwave[i].x(), getWidgetY( 0 ), barWidth, qAbs( getWidgetY( 0 ) - dwave[i].y() ) );
            }

            // 描画
            painter.fillRect( barRect, pen.color() );
        }
    }
}

void WaveWidget::mouseMoveEvent( QMouseEvent *event )
{
    // マウスが移動した
    int posX;
    int bposX;
    int i;
    qreal grad;
    QPointF left;
    QPointF right;

    // エディット禁止なら終わり
    if ( !editable ) {
        return;
    }

    // 左がクリックされてなければ終わり
    if ( !( event->buttons() & Qt::LeftButton ) ) {
        // お絵かき中ではない
        drawing = false;

        // 今の点を保存
        beforePoint = event->pos();

        return;
    }

    // 描画モードにより座標を計算
    if ( style == dsPlain || style == dsPlainRepeat ) {
        // そのまま
        bposX  = (int)beforePoint.x()  % wave.size();
        posX   = (int)event->pos().x() % wave.size();
    } else if ( style == dsScale ) {
        // スケーリング
        bposX  = (qreal)wave.size() / width() * beforePoint.x();
        posX   = (qreal)wave.size() / width() * event->pos().x();
    }

    // データー入力
    if ( drawing == true && inputMode == imLiner && qAbs( bposX - posX ) > 0 ) {
        // 線形補完モード

        // 左右座標を作る
        if ( bposX > posX ) {
            right.setX( bposX );
            right.setY( getWaveY( beforePoint.y() ) );
            left.setX( posX );
            left.setY( getWaveY( event->pos().y() ) );
        } else {
            left.setX( bposX );
            left.setY( getWaveY( beforePoint.y() ) );
            right.setX( posX );
            right.setY( getWaveY( event->pos().y() ) );
        }

        // 傾きを計算
        grad = ( right.y() - left.y() ) / ( right.x() - left.x() );

        // 前回の点から今回の点までを更新
        for ( i = left.x(); i <= right.x(); i++ ) {
            if ( i >= 0 && i < wave.size() ) {
                wave[i] = left.y() + grad * ( i - left.x() );
            }
        }
    } else {
        // 直接書く
        if ( posX >= 0 && posX < wave.size() ) {
            wave[posX] = getWaveY( event->pos().y() );
        }
    }

    // マウスでお絵かき中
    drawing = true;

    // 今の点を保存
    beforePoint = event->pos();

    // 更新
    repaint();

    // 変更を教える
    emit waveChanged();
    emit waveChangedByHand();
}

void WaveWidget::resizeEvent( QResizeEvent * )
{
}

