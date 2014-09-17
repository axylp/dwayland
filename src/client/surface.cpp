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
#include "surface.h"

#include <QRegion>
#include <QVector>

namespace KWayland
{
namespace Client
{

QList<Surface*> Surface::s_surfaces = QList<Surface*>();

Surface::Surface(QObject *parent)
    : QObject(parent)
    , m_surface(nullptr)
    , m_frameCallbackInstalled(false)
{
    s_surfaces << this;
}

Surface::~Surface()
{
    s_surfaces.removeAll(this);
    release();
}

void Surface::release()
{
    if (!m_surface) {
        return;
    }
    wl_surface_destroy(m_surface);
    m_surface = nullptr;
}

void Surface::destroy()
{
    if (!m_surface) {
        return;
    }
    free(m_surface);
    m_surface = nullptr;
}

void Surface::setup(wl_surface *surface)
{
    Q_ASSERT(surface);
    Q_ASSERT(!m_surface);
    m_surface = surface;
}

void Surface::frameCallback(void *data, wl_callback *callback, uint32_t time)
{
    Q_UNUSED(time)
    Surface *s = reinterpret_cast<Surface*>(data);
    if (callback) {
        wl_callback_destroy(callback);
    }
    s->handleFrameCallback();
}

void Surface::handleFrameCallback()
{
    m_frameCallbackInstalled = false;
    frameRendered();
}

const struct wl_callback_listener Surface::s_listener = {
        Surface::frameCallback
};

void Surface::setupFrameCallback()
{
    Q_ASSERT(isValid());
    Q_ASSERT(!m_frameCallbackInstalled);
    wl_callback *callback = wl_surface_frame(m_surface);
    wl_callback_add_listener(callback, &s_listener, this);
    m_frameCallbackInstalled = true;
}

void Surface::commit(Surface::CommitFlag flag)
{
    Q_ASSERT(isValid());
    if (flag == CommitFlag::FrameCallback) {
        setupFrameCallback();
    }
    wl_surface_commit(m_surface);
}

void Surface::damage(const QRegion &region)
{
    for (const QRect &r : region.rects()) {
        damage(r);
    }
}

void Surface::damage(const QRect &rect)
{
    Q_ASSERT(isValid());
    wl_surface_damage(m_surface, rect.x(), rect.y(), rect.width(), rect.height());
}

void Surface::attachBuffer(wl_buffer *buffer, const QPoint &offset)
{
    Q_ASSERT(isValid());
    wl_surface_attach(m_surface, buffer, offset.x(), offset.y());
}

void Surface::setSize(const QSize &size)
{
    if (m_size == size) {
        return;
    }
    m_size = size;
    emit sizeChanged(m_size);
}

Surface *Surface::get(wl_surface *native)
{
    auto it = std::find_if(s_surfaces.constBegin(), s_surfaces.constEnd(),
        [native](Surface *s) {
            return s->m_surface == native;
        }
    );
    if (it != s_surfaces.constEnd()) {
        return *(it);
    }
    return nullptr;
}

const QList< Surface* > &Surface::all()
{
    return s_surfaces;
}

}
}
