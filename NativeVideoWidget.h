#ifndef NATIVEVIDEOWIDGET_H
#define NATIVEVIDEOWIDGET_H

#include <QWidget>
#include <QQuickItem>

class NativeVideoWidget : public QWidget {
    Q_OBJECT
public:
    explicit NativeVideoWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_NativeWindow);
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_OpaquePaintEvent);
    }

    QPaintEngine* paintEngine() const override { return nullptr; }
};

#endif // NATIVEVIDEOWIDGET_H
