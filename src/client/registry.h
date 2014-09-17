/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>

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
#ifndef KWIN_WAYLAND_REGISTRY_H
#define KWIN_WAYLAND_REGISTRY_H

#include <QHash>
#include <QObject>

#include <wayland-client-protocol.h>

#include <kwaylandclient_export.h>

struct _wl_fullscreen_shell;

namespace KWayland
{
namespace Client
{

class KWAYLANDCLIENT_EXPORT Registry : public QObject
{
    Q_OBJECT
public:
    enum class Interface {
        Compositor, // wl_compositor
        Shell,      // wl_shell
        Seat,       // wl_seat
        Shm,        // wl_shm
        Output,     // wl_output
        FullscreenShell, // _wl_fullscreen_shell
        Unknown
    };
    explicit Registry(QObject *parent = nullptr);
    virtual ~Registry();

    void release();
    void destroy();
    void create(wl_display *display);
    void setup();

    bool isValid() const {
        return m_registry != nullptr;
    }
    bool hasInterface(Interface interface) const;

    wl_compositor *bindCompositor(uint32_t name, uint32_t version) const;
    wl_shell *bindShell(uint32_t name, uint32_t version) const;
    wl_seat *bindSeat(uint32_t name, uint32_t version) const;
    wl_shm *bindShm(uint32_t name, uint32_t version) const;
    wl_output *bindOutput(uint32_t name, uint32_t version) const;
    _wl_fullscreen_shell *bindFullscreenShell(uint32_t name, uint32_t version) const;

    operator wl_registry*() {
        return m_registry;
    }
    operator wl_registry*() const {
        return m_registry;
    }
    wl_registry *registry() {
        return m_registry;
    }

    static void globalAnnounce(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
    static void globalRemove(void *data, struct wl_registry *registry, uint32_t name);

Q_SIGNALS:
    void compositorAnnounced(quint32 name, quint32 version);
    void shellAnnounced(quint32 name, quint32 version);
    void seatAnnounced(quint32 name, quint32 version);
    void shmAnnounced(quint32 name, quint32 version);
    void outputAnnounced(quint32 name, quint32 version);
    void fullscreenShellAnnounced(quint32 name, quint32 version);
    void compositorRemoved(quint32 name);
    void shellRemoved(quint32 name);
    void seatRemoved(quint32 name);
    void shmRemoved(quint32 name);
    void outputRemoved(quint32 name);
    void fullscreenShellRemoved(quint32 name);

private:
    static const struct wl_registry_listener s_registryListener;
    void handleAnnounce(uint32_t name, const char *interface, uint32_t version);
    void handleRemove(uint32_t name);
    void *bind(Interface interface, uint32_t name, uint32_t version) const;

    wl_registry *m_registry;
    struct InterfaceData {
        Interface interface;
        uint32_t name;
        uint32_t version;
    };
    QList<InterfaceData> m_interfaces;
};

}
}

#endif
