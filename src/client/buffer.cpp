/********************************************************************
Copyright 2013  Martin Gräßlin <mgraesslin@kde.org>

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
#include "buffer.h"
#include "shm_pool.h"
// system
#include <string.h>

namespace KWayland
{
namespace Client
{

const struct wl_buffer_listener Buffer::s_listener = {
    Buffer::releasedCallback
};

Buffer::Buffer(ShmPool *parent, wl_buffer *buffer, const QSize &size, int32_t stride, size_t offset)
    : m_shm(parent)
    , m_nativeBuffer(buffer)
    , m_released(false)
    , m_size(size)
    , m_stride(stride)
    , m_offset(offset)
    , m_used(false)
{
    wl_buffer_add_listener(m_nativeBuffer, &s_listener, this);
}

Buffer::~Buffer()
{
    wl_buffer_destroy(m_nativeBuffer);
}

void Buffer::releasedCallback(void *data, wl_buffer *buffer)
{
    Buffer *b = reinterpret_cast<Buffer*>(data);
    Q_ASSERT(b->m_nativeBuffer == buffer);
    b->setReleased(true);
}

void Buffer::copy(const void *src)
{
    memcpy(address(), src, m_size.height()*m_stride);
}

uchar *Buffer::address()
{
    return reinterpret_cast<uchar*>(m_shm->poolAddress()) + m_offset;
}

}
}
