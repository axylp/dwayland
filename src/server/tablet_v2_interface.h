/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WAYLAND_SERVER_TABLET_INTERFACE_H
#define WAYLAND_SERVER_TABLET_INTERFACE_H

#include <KWaylandServer/kwaylandserver_export.h>

#include <QObject>
#include <QVector>

namespace KWaylandServer
{
class Display;
class SeatInterface;
class SurfaceInterface;
class TabletCursorV2;
class TabletCursorV2Private;
class TabletManagerV2InterfacePrivate;
class TabletSeatV2Interface;
class TabletSeatV2InterfacePrivate;
class TabletToolV2InterfacePrivate;
class TabletV2Interface;
class TabletV2InterfacePrivate;

/**
 * This is an implementation of wayland-protocols/unstable/tablet/tablet-unstable-v2.xml
 *
 * This class is just the means to get a @class TabletSeatInterface, which is
 * the class that will have all of the information we need.
 *
 * @since 5.69
 */

class KWAYLANDSERVER_EXPORT TabletManagerV2Interface : public QObject
{
    Q_OBJECT
public:
    virtual ~TabletManagerV2Interface();

    TabletSeatV2Interface *seat(SeatInterface *seat) const;

private:
    friend class Display;
    explicit TabletManagerV2Interface(Display *d, QObject *parent);
    QScopedPointer<TabletManagerV2InterfacePrivate> d;
};

class KWAYLANDSERVER_EXPORT TabletToolV2Interface : public QObject
{
    Q_OBJECT
public:
    virtual ~TabletToolV2Interface();

    enum Type {
        Pen = 0x140, ///< Pen
        Eraser = 0x141, ///< Eraser
        Brush = 0x142, ///< Brush
        Pencil = 0x143, ///< Pencil
        Airbrush = 0x144, ///< Airbrush
        Finger = 0x145, ///< Finger
        Mouse = 0x146, ///< Mouse
        Lens = 0x147, ///< Lens
        Totem
    };
    Q_ENUM(Type)

    enum Capability {
        Tilt = 1, ///< Tilt axeis
        Pressure = 2, ///< Pressure axis
        Distance = 3, ///< Distance axis
        Rotation = 4, ///< Z-rotation axis
        Slider = 5, ///< Slider axis
        Wheel = 6 ///< Wheel axis
    };
    Q_ENUM(Capability)

    /**
     * Sets the surface the events will be sent to.
     *
     * Make sure the surface supports being sent events to.
     *
     * @see TabletV2Interface::isSurfaceSupported
     */
    void setCurrentSurface(SurfaceInterface *surface);
    bool isClientSupported() const;

    void sendRemoved();
    void sendProximityIn(TabletV2Interface *tablet);
    void sendProximityOut();
    void sendUp();
    void sendDown();
    void sendPressure(quint32 pressure);
    void sendDistance(quint32 distance);
    void sendTilt(qreal degreesX, qreal degreesY);
    void sendRotation(qreal degrees);
    void sendSlider(qint32 position);
    void sendWheel(qint32 degrees, qint32 clicks);
    void sendButton(quint32 button, bool pressed);
    void sendFrame(quint32 time);
    void sendMotion(const QPointF &pos);

Q_SIGNALS:
    void cursorChanged(TabletCursorV2 *cursor) const;

private:
    friend class TabletSeatV2InterfacePrivate;
    friend class TabletSeatV2Interface;
    explicit TabletToolV2Interface(Display *display, Type type, quint32 hsh, quint32 hsl, quint32 hih, quint32 hil, const QVector<Capability> &capability, QObject *parent);
    QScopedPointer<TabletToolV2InterfacePrivate> d;
};

class KWAYLANDSERVER_EXPORT TabletCursorV2 : public QObject
{
    Q_OBJECT
public:
    ~TabletCursorV2() override;
    QPoint hotspot() const;
    quint32 enteredSerial() const;
    SurfaceInterface* surface() const;

Q_SIGNALS:
    void changed();

private:
    TabletCursorV2();
    const QScopedPointer<TabletCursorV2Private> d;
    friend class TabletToolV2InterfacePrivate;
};

class KWAYLANDSERVER_EXPORT TabletV2Interface : public QObject
{
    Q_OBJECT
public:
    virtual ~TabletV2Interface();

    /**
     * @returns true if the surface has been bound to the tablet.
     */
    bool isSurfaceSupported(SurfaceInterface *surface) const;

    void sendRemoved();

private:
    friend class TabletSeatV2Interface;
    friend class TabletSeatV2InterfacePrivate;
    friend class TabletToolV2Interface;
    explicit TabletV2Interface(quint32 vendorId, quint32 productId, const QString &name, const QStringList &paths, QObject *parent);
    QScopedPointer<TabletV2InterfacePrivate> d;
};

class KWAYLANDSERVER_EXPORT TabletSeatV2Interface : public QObject
{
    Q_OBJECT
public:
    virtual ~TabletSeatV2Interface();

    TabletV2Interface *addTablet(quint32 vendorId, quint32 productId, const QString &sysname, const QString &name, const QStringList &paths);
    TabletToolV2Interface *addTool(TabletToolV2Interface::Type type, quint64 hardwareSerial, quint64 hardwareId, const QVector<TabletToolV2Interface::Capability> &capabilities);

    TabletToolV2Interface *toolByHardwareId(quint64 hardwareId) const;
    TabletToolV2Interface *toolByHardwareSerial(quint64 hardwareSerial) const;
    TabletV2Interface *tabletByName(const QString &sysname) const;

    void removeTablet(const QString &sysname);

private:
    friend class TabletManagerV2InterfacePrivate;
    explicit TabletSeatV2Interface(Display *display, QObject *parent);
    QScopedPointer<TabletSeatV2InterfacePrivate> d;
};

}

Q_DECLARE_METATYPE(KWaylandServer::TabletSeatV2Interface *)

#endif