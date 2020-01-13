/*
    Copyright © 2015-2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "screenshotgrabber.hpp"
#include "screengrabberchooserrectitem.hpp"
#include "screengrabberoverlayitem.hpp"
#include "toolboxgraphicsitem.hpp"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QScreen>
#include <QTimer>
#include <obs-frontend-api.h>
#include <obs-module.h>

ScreenshotGrabber::ScreenshotGrabber(screenshot_callback_t* callback)
    : QObject()
    , mKeysBlocked(false)
    , scene(nullptr)
    , mcallback(callback)
{
    window = new QGraphicsView(scene); // Top-level widget
    window->setWindowFlags(Qt::FramelessWindowHint | Qt::BypassWindowManagerHint);
    window->setContentsMargins(0, 0, 0, 0);
    window->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    window->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    window->setFrameShape(QFrame::NoFrame);
    window->installEventFilter(this);
    pixRatio = QApplication::primaryScreen()->devicePixelRatio();

    setupScene();
}

ScreenshotGrabber::ScreenshotGrabber(screenshot_callback_t* callback,
    QRect& rect)
    : QObject()
    , mKeysBlocked(false)
    , scene(nullptr)
    , mcallback(callback)
    , window(nullptr)
{
    screenGrab = grabScreen();
    grabRegion(rect);
}

void ScreenshotGrabber::reInit()
{
    window->resetCachedContent();
    setupScene();
    showGrabber();
    mKeysBlocked = false;
}

ScreenshotGrabber::~ScreenshotGrabber()
{
    delete scene;
    delete window;
}

bool ScreenshotGrabber::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
        return handleKeyPress(static_cast<QKeyEvent*>(event));

    return QObject::eventFilter(object, event);
}

void ScreenshotGrabber::showGrabber()
{
    this->screenGrab = grabScreen();
    this->screenGrabDisplay->setPixmap(this->screenGrab);
    this->window->show();
    this->window->setFocus();
    this->window->grabKeyboard();

    QRect fullGrabbedRect = screenGrab.rect();
    QRect rec = QApplication::primaryScreen()->virtualGeometry();

    this->window->setGeometry(rec);
    this->scene->setSceneRect(fullGrabbedRect);
    this->overlay->setRect(fullGrabbedRect);

    adjustTooltipPosition();
}

bool ScreenshotGrabber::handleKeyPress(QKeyEvent* event)
{
    if (mKeysBlocked)
        return false;

    if (event->key() == Qt::Key_Escape)
        reject();
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        acceptRegion();
    else if (event->key() == Qt::Key_Space) {
        mKeysBlocked = true;

        window->hide();
        QWidget* b = static_cast<QWidget*>(obs_frontend_get_main_window());
        b->setVisible(!b->isVisible());
        QTimer::singleShot(350, this, SLOT(reInit()));
    } else
        return false;

    return true;
}

void ScreenshotGrabber::acceptRegion()
{
    QRect rect = this->chooserRect->chosenRect();
    if (rect.width() < 1 || rect.height() < 1)
        return;
    grabRegion(rect);
}

void ScreenshotGrabber::grabRegion(QRect& rect)
{

    // Scale the accepted region from DIPs to actual pixels
    rect.setRect(rect.x() * pixRatio, rect.y() * pixRatio, rect.width() * pixRatio,
        rect.height() * pixRatio);

    blog(LOG_DEBUG, "Screenshot accepted, chosen region: x: %i, y: %i, w: %i, h: %i",
        rect.x(), rect.y(), rect.width(), rect.height());
    QPixmap pixmap = this->screenGrab.copy(rect);
    mcallback(!pixmap.size().isNull(), &pixmap, rect);
    QWidget* b = static_cast<QWidget*>(obs_frontend_get_main_window());
    b->show();
    deleteLater();
}

void ScreenshotGrabber::setupScene()
{
    delete scene;
    scene = new QGraphicsScene;
    window->setScene(scene);

    this->overlay = new ScreenGrabberOverlayItem(this);
    this->helperToolbox = new ToolBoxGraphicsItem;

    this->screenGrabDisplay = scene->addPixmap(this->screenGrab);
    this->helperTooltip = scene->addText(QString());

    scene->addItem(this->overlay);
    this->chooserRect = new ScreenGrabberChooserRectItem(scene);
    scene->addItem(this->helperToolbox);

    this->helperToolbox->addToGroup(this->helperTooltip);
    this->helperTooltip->setDefaultTextColor(Qt::black);
    useNothingSelectedTooltip();

    connect(this->chooserRect, &ScreenGrabberChooserRectItem::doubleClicked, this,
        &ScreenshotGrabber::acceptRegion);
    connect(this->chooserRect, &ScreenGrabberChooserRectItem::regionChosen, this,
        &ScreenshotGrabber::chooseHelperTooltipText);
    connect(this->chooserRect, &ScreenGrabberChooserRectItem::regionChosen, this->overlay,
        &ScreenGrabberOverlayItem::setChosenRect);
}

void ScreenshotGrabber::useNothingSelectedTooltip()
{
    helperTooltip->setHtml(obs_module_text("scrab.gui.tooltip.default"));
    adjustTooltipPosition();
}

void ScreenshotGrabber::useRegionSelectedTooltip()
{
    helperTooltip->setHtml(obs_module_text("scrab.gui.tooltip.selected"));
    adjustTooltipPosition();
}

void ScreenshotGrabber::chooseHelperTooltipText(QRect rect)
{
    if (rect.size().isNull())
        useNothingSelectedTooltip();
    else
        useRegionSelectedTooltip();
}

/**
 * @internal
 * @brief Align the tooltip centered at top of screen with the mouse cursor.
 */
void ScreenshotGrabber::adjustTooltipPosition()
{
    QRect recGL = QGuiApplication::primaryScreen()->virtualGeometry();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    const auto rec = QGuiApplication::screenAt(QCursor::pos())->geometry();
#else
    const auto rec = qApp->desktop()->screenGeometry(QCursor::pos());
#endif
    const QRectF ttRect = this->helperToolbox->childrenBoundingRect();
    int x = qAbs(recGL.x()) + rec.x() + ((rec.width() - ttRect.width()) / 2);
    int y = qAbs(recGL.y()) + rec.y();

    helperToolbox->setX(x);
    helperToolbox->setY(y);
}

void ScreenshotGrabber::reject()
{
    deleteLater();
    QWidget* b = static_cast<QWidget*>(obs_frontend_get_main_window());
    b->show();
}

QPixmap ScreenshotGrabber::grabScreen()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect rec = screen->virtualGeometry();

    // Multiply by devicePixelRatio to get actual desktop size
    return screen->grabWindow(QApplication::desktop()->winId(), rec.x() * pixRatio,
        rec.y() * pixRatio, rec.width() * pixRatio, rec.height() * pixRatio);
}

void ScreenshotGrabber::beginRectChooser(QGraphicsSceneMouseEvent* event)
{
    QPointF pos = event->scenePos();
    this->chooserRect->setX(pos.x());
    this->chooserRect->setY(pos.y());
    this->chooserRect->beginResize(event->scenePos());
}
