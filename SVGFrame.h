#ifndef SVGFRAME_H
#define SVGFRAME_H
#include "x2goclientconfig.h"


#include <QFrame>
#include <QtSvg/QSvgRenderer>

class SVGFrame: public QFrame
{

    Q_OBJECT
public:
    SVGFrame(QString fname, bool st, QWidget* parent=0, Qt::WFlags f=0);
    void setRepaintable(bool val)
    {
        repaint=val;
    }
    void loadBg(QString fl);
    virtual QSize sizeHint() const;
private:
    QSvgRenderer* renderer;
    bool repaint;
    bool drawImg;
    bool empty;
protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
signals:
    void resized (const QSize);
};

#endif

