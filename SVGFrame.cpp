#include "SVGFrame.h"
#include "x2goclientconfig.h"

#include <QPainter>
#include <QTimer>
#include "x2gologdebug.h"
#include <QResizeEvent>



SVGFrame::SVGFrame(QString fname,bool st,QWidget* parent, Qt::WFlags f) :QFrame(parent,f)
{
    empty=false;
    if(fname==QString::null)
        empty=true;
    if(!empty)
    {
        repaint=true;
        drawImg=st;
        renderer=new QSvgRenderer(this);
        renderer->load(fname);

        if(drawImg)
        {
            setAutoFillBackground(true);
            QPalette pal=palette();
            QImage img(renderer->defaultSize(),QImage::Format_ARGB32_Premultiplied);
            QPainter p(&img);
            renderer->render(&p);
            pal.setBrush(QPalette::Window,QBrush(QPixmap::fromImage(img)));
            setPalette(pal);
        }
        else
        {
            QTimer *timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(update()));
            if(renderer->animated())
	    {
                timer->start(1000/renderer->framesPerSecond());
		x2goDebug<<"Animated, fps:"<<renderer->framesPerSecond()<<endl;
	    }
        }
    }
}


void SVGFrame::paintEvent(QPaintEvent* event)
{
    if(repaint && !drawImg && !empty)
    {
        QPainter p(this);
        p.setViewport(0, 0, width(), height());
        p.eraseRect(0, 0, width(), height());
        renderer->render(&p);
    }
    QFrame::paintEvent(event);
}


void SVGFrame::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);
    emit resized(event->size());
    if(drawImg && event->size().width()>0 && event->size().height()>0 &&!empty)
    {
        QPalette pal=palette();
        QImage img(event->size(),QImage::Format_ARGB32_Premultiplied);
        QPainter p(&img);
        if(p.isActive())
            renderer->render(&p);
        pal.setBrush(QPalette::Window,QBrush(QPixmap::fromImage(img)));
        setPalette(pal);
    }
}


QSize SVGFrame::sizeHint() const
{
    if(!empty)
        return renderer->defaultSize();
    return QFrame::sizeHint();
}

void SVGFrame::loadBg(QString fl)
{
        renderer->load(fl);
	update();
}
