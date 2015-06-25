/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#ifndef WAYLAND_PLASMASHELL_H
#define WAYLAND_PLASMASHELL_H

#include <QObject>
#include <QSize>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_surface;
struct org_kde_plasma_shell;
struct org_kde_plasma_surface;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Surface;
class PlasmaShellSurface;

/**
 * @short Wrapper for the org_kde_plasma_shell interface.
 *
 * This class provides a convenient wrapper for the org_kde_plasma_shell interface.
 * It's main purpose is to create a PlasmaShellSurface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the Shell interface:
 * @code
 * PlasmaShell *s = registry->createPlasmaShell(name, version);
 * @endcode
 *
 * This creates the PlasmaShell and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * PlasmaShell *s = new PlasmaShell;
 * s->setup(registry->bindPlasmaShell(name, version));
 * @endcode
 *
 * The PlasmaShell can be used as a drop-in replacement for any org_kde_plasma_shell
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @see PlasmaShellSurface
 **/
class KWAYLANDCLIENT_EXPORT PlasmaShell : public QObject
{
    Q_OBJECT
public:
    explicit PlasmaShell(QObject *parent = nullptr);
    virtual ~PlasmaShell();

    /**
     * @returns @c true if managing a org_kde_plasma_shell.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_plasma_shell interface.
     * After the interface has been released the PlasmaShell instance is no
     * longer valid and can be setup with another org_kde_plasma_shell interface.
     *
     * Right before the interface is released the signal interfaceAboutToBeReleased is emitted.
     * @see interfaceAboutToBeReleased
     **/
    void release();
    /**
     * Destroys the data held by this PlasmaShell.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. Once the connection becomes invalid, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new org_kde_plasma_shell interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, shell, &PlasmaShell::destroyed);
     * @endcode
     *
     * Right before the data is destroyed, the signal interfaceAboutToBeDestroyed is emitted.
     *
     * @see release
     * @see interfaceAboutToBeDestroyed
     **/
    void destroy();
    /**
     * Setup this Shell to manage the @p shell.
     * When using Registry::createShell there is no need to call this
     * method.
     **/
    void setup(org_kde_plasma_shell *shell);

    /**
     * Sets the @p queue to use for creating a Surface.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Surface.
     **/
    EventQueue *eventQueue();

    /**
     * Creates a PlasmaShellSurface for the given @p surface and sets it up.
     *
     * @param surface The native surface to create the PlasmaShellSurface for
     * @param parent The parent to use for the PlasmaShellSurface
     * @returns created PlasmaShellSurface
     **/
    PlasmaShellSurface *createSurface(wl_surface *surface, QObject *parent = nullptr);
    /**
     * Creates a PlasmaShellSurface for the given @p surface and sets it up.
     *
     * @param surface The Surface to create the PlasmaShellSurface for
     * @param parent The parent to use for the PlasmaShellSurface
     * @returns created PlasmaShellSurface
     **/
    PlasmaShellSurface *createSurface(Surface *surface, QObject *parent = nullptr);

    operator org_kde_plasma_shell*();
    operator org_kde_plasma_shell*() const;

Q_SIGNALS:
    /**
     * This signal is emitted right before the interface is released.
     **/
    void interfaceAboutToBeReleased();
    /**
     * This signal is emitted right before the data is destroyed.
     **/
    void interfaceAboutToBeDestroyed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the org_kde_plasma_surface interface.
 *
 * This class is a convenient wrapper for the org_kde_plasma_surface interface.
 *
 * To create an instance use PlasmaShell::createSurface.
 *
 * @see PlasmaShell
 * @see Surface
 **/
class KWAYLANDCLIENT_EXPORT PlasmaShellSurface : public QObject
{
    Q_OBJECT
public:
    explicit PlasmaShellSurface(QObject *parent);
    virtual ~PlasmaShellSurface();

    /**
     * Releases the org_kde_plasma_surface interface.
     * After the interface has been released the PlasmaShellSurface instance is no
     * longer valid and can be setup with another org_kde_plasma_surface interface.
     *
     * This method is automatically invoked when the PlasmaShell which created this
     * PlasmaShellSurface gets released.
     **/
    void release();
    /**
     * Destroys the data hold by this PlasmaShellSurface.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new org_kde_plasma_surface interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the PlasmaShell which created this
     * PlasmaShellSurface gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * Setup this PlasmaShellSurface to manage the @p surface.
     * There is normally no need to call this method as it's invoked by
     * PlasmaShell::createSurface.
     **/
    void setup(org_kde_plasma_surface *surface);

    bool isValid() const;
    operator org_kde_plasma_surface*();
    operator org_kde_plasma_surface*() const;

    enum class Role {
        Normal,
        Desktop,
        Panel
    };
    void setRole(Role role);
    void setPosition(const QPoint &point);

    enum class PanelBehavior {
        AlwaysVisible,
        AutoHide,
        WindowsCanCover,
        WindowsGoBelow
    };
    void setPanelBehavior(PanelBehavior behavior);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif