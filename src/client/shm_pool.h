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
#ifndef KWIN_WAYLAND_SHM_POOL_H
#define KWIN_WAYLAND_SHM_POOL_H

#include <QObject>

#include <kwaylandclient_export.h>

class QImage;
class QSize;
class QTemporaryFile;

struct wl_shm;
struct wl_buffer;
struct wl_shm_pool;

namespace KWayland
{
namespace Client
{
class Buffer;

class KWAYLANDCLIENT_EXPORT ShmPool : public QObject
{
    Q_OBJECT
public:
    explicit ShmPool(QObject *parent = nullptr);
    virtual ~ShmPool();
    bool isValid() const;
    void setup(wl_shm *shm);
    void release();
    void destroy();

    wl_buffer *createBuffer(const QImage &image);
    wl_buffer *createBuffer(const QSize &size, int32_t stride, const void *src);
    void *poolAddress() const;
    Buffer *getBuffer(const QSize &size, int32_t stride);
    wl_shm *shm();
Q_SIGNALS:
    void poolResized();
private:
    bool createPool();
    bool resizePool(int32_t newSize);
    wl_shm *m_shm;
    wl_shm_pool *m_pool;
    void *m_poolData;
    int32_t m_size;
    QScopedPointer<QTemporaryFile> m_tmpFile;
    bool m_valid;
    int m_offset;
    QList<Buffer*> m_buffers;
};

inline
bool ShmPool::isValid() const
{
    return m_valid;
}

inline
void* ShmPool::poolAddress() const
{
    return m_poolData;
}

inline
wl_shm *ShmPool::shm()
{
    return m_shm;
}

}
}

#endif
