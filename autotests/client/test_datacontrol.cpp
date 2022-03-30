/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

// Qt
#include <QHash>
#include <QThread>
#include <QtTest>

// WaylandServer
#include "../../src/server/compositor_interface.h"
#include "../../src/server/datacontroldevice_interface.h"
#include "../../src/server/datacontroldevicemanager_interface.h"
#include "../../src/server/datacontrolsource_interface.h"
#include "../../src/server/datadevice_interface.h"
#include "../../src/server/display.h"
#include "../../src/server/seat_interface.h"
#include "../../src/server/abstract_data_source.h"

#include <KWayland/Client/compositor.h>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/seat.h>
#include <KWayland/Client/datacontroldevicemanager.h>
#include <KWayland/Client/datacontroloffer.h>
#include <KWayland/Client/datacontroldevice.h>
#include <KWayland/Client/datacontrolsource.h>
#include <KWayland/Client/datacontroldevice.h>





using namespace KWayland::Server;
// Faux-client API for tests

Q_DECLARE_OPAQUE_POINTER(::zwlr_data_control_offer_v1 *)
Q_DECLARE_METATYPE(::zwlr_data_control_offer_v1 *)



/*
class testDataControlDeviceManager : public DataControlDeviceManager
{
    Q_OBJECT
};

class MyDataControlOffer : public DataControlOfferV1
{
    Q_OBJECT
public:
    ~MyDataControlOffer()
    {
        destroy();
    }
    QStringList receivedOffers()
    {
        return m_receivedOffers;
    }

protected:
    virtual void zwlr_data_control_offer_v1_offer(const QString &mime_type) override
    {
        m_receivedOffers << mime_type;
    }

private:
    QStringList m_receivedOffers;
};

class DataControlDevice : public QObject, public QtWayland::zwlr_data_control_device_v1
{
    Q_OBJECT
public:
    ~DataControlDevice()
    {
        destroy();
    }
Q_SIGNALS:
    void dataControlOffer(DataControlOffer *offer); // our event receives a new ID, so we make a new object
    void selection(struct ::zwlr_data_control_offer_v1 *id);
    void primary_selection(struct ::zwlr_data_control_offer_v1 *id);

protected:
    void zwlr_data_control_device_v1_data_offer(struct ::zwlr_data_control_offer_v1 *id) override
    {
        auto offer = new DataControlOffer;
        offer->init(id);
        Q_EMIT dataControlOffer(offer);
    }

    void zwlr_data_control_device_v1_selection(struct ::zwlr_data_control_offer_v1 *id) override
    {
        Q_EMIT selection(id);
    }

    void zwlr_data_control_device_v1_primary_selection(struct ::zwlr_data_control_offer_v1 *id) override
    {
        Q_EMIT primary_selection(id);
    }
};

class DataControlSource : public QObject, public QtWayland::zwlr_data_control_source_v1
{
    Q_OBJECT
public:
    ~DataControlSource()
    {
        destroy();
    }

public:
};
*/

class TestDataSource : public AbstractDataSource
{
    Q_OBJECT
public:
    TestDataSource()
        : AbstractDataSource(nullptr)
    {
    }
    ~TestDataSource() override
    {
        Q_EMIT aboutToBeDestroyed();
    }
    void requestData(const QString &mimeType, qint32 fd) override
    {
        Q_UNUSED(mimeType);
        Q_UNUSED(fd);
    }
    void cancel() override{}

    QStringList mimeTypes() const override
    {
        return {"text/plain"};
    }
};
// The test itself

class DataControlInterfaceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void testCopyToControl();
    void testCopyToControlPrimarySelection();
    void testCopyFromControl();
    void testCopyFromControlPrimarySelection();
    void testKlipperCase();

private:
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::EventQueue *m_queue;
    KWayland::Client::Compositor *m_clientCompositor;
    KWayland::Client::Seat *m_clientSeat = nullptr;

    QThread *m_thread;
    Display *m_display;
    SeatInterface *m_seat;
    CompositorInterface *m_serverCompositor;

    DataControlDeviceManagerInterface *m_dataControlDeviceManagerInterface;

    KWayland::Client::DataControlDeviceManager *m_dataControlDeviceManager;

    QVector<SurfaceInterface *> m_surfaces;
};

static const QString s_socketName = QStringLiteral("kwin-wayland-datacontrol-test-0");


void DataControlInterfaceTest::init()
{
    qRegisterMetaType<KWayland::Server::DataDeviceInterface*>();
    qRegisterMetaType<KWayland::Client::DataControlDeviceV1*>();
    qRegisterMetaType<KWayland::Client::DataControlOfferV1*>();


    using namespace KWayland::Server;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());

    // setup connection
    m_connection = new KWayland::Client::ConnectionThread;
    QSignalSpy connectedSpy(m_connection, SIGNAL(connected()));
    m_connection->setSocketName(s_socketName);

    m_thread = new QThread(this);
    m_connection->moveToThread(m_thread);
    m_thread->start();

    m_connection->initConnection();
    QVERIFY(connectedSpy.wait());

    m_queue = new KWayland::Client::EventQueue(this);
    QVERIFY(!m_queue->isValid());
    m_queue->setup(m_connection);
    QVERIFY(m_queue->isValid());

    KWayland::Client::Registry registry;
    QSignalSpy datacontrolDeviceManagerSpy(&registry, SIGNAL(dataControlDeviceManagerAnnounced(quint32,quint32)));
    QVERIFY(datacontrolDeviceManagerSpy.isValid());
    QSignalSpy seatSpy(&registry, SIGNAL(seatAnnounced(quint32,quint32)));
    QVERIFY(seatSpy.isValid());
    QSignalSpy compositorSpy(&registry, SIGNAL(compositorAnnounced(quint32,quint32)));
    QVERIFY(compositorSpy.isValid());
    QVERIFY(!registry.eventQueue());
    registry.setEventQueue(m_queue);
    QCOMPARE(registry.eventQueue(), m_queue);
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();

    m_dataControlDeviceManagerInterface = m_display->createDataControlDeviceManager(m_display);
    m_dataControlDeviceManagerInterface->create();
    QVERIFY(m_dataControlDeviceManagerInterface->isValid());

    QVERIFY(datacontrolDeviceManagerSpy.wait());
    m_dataControlDeviceManager = registry.createDataControlDeviceManager(datacontrolDeviceManagerSpy.first().first().value<quint32>(),
                                                           datacontrolDeviceManagerSpy.first().last().value<quint32>(), this);

    m_seat = m_display->createSeat(m_display);
    m_seat->setHasPointer(true);
    m_seat->create();
    QVERIFY(m_seat->isValid());

    QVERIFY(seatSpy.wait());
    m_clientSeat = registry.createSeat(seatSpy.first().first().value<quint32>(),
                                 seatSpy.first().last().value<quint32>(), this);
    QVERIFY(m_clientSeat->isValid());
    QSignalSpy pointerChangedSpy(m_clientSeat, SIGNAL(hasPointerChanged(bool)));
    QVERIFY(pointerChangedSpy.isValid());
    QVERIFY(pointerChangedSpy.wait());

    m_serverCompositor = m_display->createCompositor(m_display);
    m_serverCompositor->create();
    QVERIFY(m_serverCompositor->isValid());

    QVERIFY(compositorSpy.wait());
    m_clientCompositor = registry.createCompositor(compositorSpy.first().first().value<quint32>(),
                                             compositorSpy.first().last().value<quint32>(), this);
    QVERIFY(m_clientCompositor->isValid());
}

void DataControlInterfaceTest::cleanup()
{
#define CLEANUP(variable)                                                                                                                                      \
    if (variable) {                                                                                                                                            \
        delete variable;                                                                                                                                       \
        variable = nullptr;                                                                                                                                    \
    }
    CLEANUP(m_dataControlDeviceManager)
    CLEANUP(m_clientSeat)
    CLEANUP(m_clientCompositor)
    CLEANUP(m_queue)
    if (m_connection) {
        m_connection->deleteLater();
        m_connection = nullptr;
    }
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }
    CLEANUP(m_display)
#undef CLEANUP

    // these are the children of the display
    m_seat = nullptr;
    m_serverCompositor = nullptr;
}

void DataControlInterfaceTest::testCopyToControl()
{
    // we set a dummy data source on the seat using abstract client directly
    // then confirm we receive the offer despite not having a surface

    QScopedPointer<KWayland::Client::DataControlDeviceV1> dataControlDevice(new KWayland::Client::DataControlDeviceV1);
    dataControlDevice.reset(m_dataControlDeviceManager->getDataDevice(m_clientSeat));

    QSignalSpy selectionSpy(dataControlDevice.data(), &KWayland::Client::DataControlDeviceV1::dataOffered);

    QScopedPointer<TestDataSource> testSelection(new TestDataSource);
    m_seat->setSelection(testSelection.data());

    // selection will be sent after we've been sent a new offer object and the mimes have been sent to that object
    selectionSpy.wait();

    QCOMPARE(selectionSpy.count(), 1);
    KWayland::Client::DataControlOfferV1* offer(selectionSpy.first().first().value<KWayland::Client::DataControlOfferV1 *>());
    QCOMPARE(offer->offeredMimeTypes().count(), 1);
    QCOMPARE(offer->offeredMimeTypes()[0], "text/plain");

}

void DataControlInterfaceTest::testCopyToControlPrimarySelection()
{
    // we set a dummy data source on the seat using abstract client directly
    // then confirm we receive the offer despite not having a surface

    QScopedPointer<KWayland::Client::DataControlDeviceV1> dataControlDevice(new KWayland::Client::DataControlDeviceV1);
    dataControlDevice.reset(m_dataControlDeviceManager->getDataDevice(m_clientSeat));

    QSignalSpy selectionSpy(dataControlDevice.data(), &KWayland::Client::DataControlDeviceV1::selectionOffered);
    //QSignalSpy selectionSpy(dataControlDevice.data(), &KWayland::Client::DataControlDeviceV1::primary_selection);

    QSignalSpy dataDeviceCreated(m_dataControlDeviceManagerInterface, &KWayland::Server::DataControlDeviceManagerInterface::dataDeviceCreated);
    dataDeviceCreated.wait();

    QScopedPointer<TestDataSource> testSelection(new TestDataSource);
    m_seat->setPrimarySelection(testSelection.data());

    // selection will be sent after we've been sent a new offer object and the mimes have been sent to that object
    selectionSpy.wait();

    QCOMPARE(selectionSpy.count(), 1);
     KWayland::Client::DataControlOfferV1* offer(selectionSpy.first().first().value<KWayland::Client::DataControlOfferV1 *>());

    QCOMPARE(offer->offeredMimeTypes().count(), 1);
    QCOMPARE(offer->offeredMimeTypes()[0], "text/plain");
}

void DataControlInterfaceTest::testCopyFromControl()
{
    // we create a data device and set a selection
    // then confirm the server sees the new selection
    QSignalSpy serverSelectionChangedSpy(m_seat, &SeatInterface::selectionChanged);

    QScopedPointer<KWayland::Client::DataControlDeviceV1> dataControlDevice(new KWayland::Client::DataControlDeviceV1);
    dataControlDevice.reset(m_dataControlDeviceManager->getDataDevice(m_clientSeat));

    QScopedPointer<KWayland::Client::DataControlSourceV1> source(new KWayland::Client::DataControlSourceV1);
    source.reset(m_dataControlDeviceManager->createDataSource());
    source->offer("text/plain");

    dataControlDevice->setSelection(0,source.data());

    serverSelectionChangedSpy.wait();
    QVERIFY(m_seat->selection());
    QCOMPARE(m_seat->selection()->mimeTypes()[0], "text/plain");
}

void DataControlInterfaceTest::testCopyFromControlPrimarySelection()
{
    // we create a data device and set a selection
    // then confirm the server sees the new selection
    QSignalSpy serverSelectionChangedSpy(m_seat, &SeatInterface::primarySelectionChanged);

    QScopedPointer<KWayland::Client::DataControlDeviceV1> dataControlDevice(new KWayland::Client::DataControlDeviceV1);
    dataControlDevice.reset(m_dataControlDeviceManager->getDataDevice(m_clientSeat));

    QSignalSpy dataDeviceCreated(m_dataControlDeviceManagerInterface, &KWayland::Server::DataControlDeviceManagerInterface::dataDeviceCreated);
    dataDeviceCreated.wait();

    QScopedPointer<KWayland::Client::DataControlSourceV1> source(new KWayland::Client::DataControlSourceV1);
    source.reset(m_dataControlDeviceManager->createDataSource());
    source->offer("text/plain");

    dataControlDevice->setSelection(0, source.data());

    serverSelectionChangedSpy.wait();
    QVERIFY(m_seat->selection());
    QCOMPARE(m_seat->selection()->mimeTypes()[0], "text/plain");
}

void DataControlInterfaceTest::testKlipperCase()
{
    // This tests the setup of klipper's real world operation and a race with a common pattern seen between clients and klipper
    // The client's behaviour is faked with direct access to the seat

    QScopedPointer<KWayland::Client::DataControlDeviceV1> dataControlDevice(new KWayland::Client::DataControlDeviceV1);
    dataControlDevice.reset(m_dataControlDeviceManager->getDataDevice(m_clientSeat));


    QSignalSpy dataDeviceCreated(m_dataControlDeviceManagerInterface, &KWayland::Server::DataControlDeviceManagerInterface::dataDeviceCreated);
    dataDeviceCreated.wait();

    QSignalSpy newOfferSpy(dataControlDevice.data(), &KWayland::Client::DataControlDeviceV1::dataControlOffered);
    //QSignalSpy selectionSpy(dataControlDevice.data(), &KWayland::Client::DataControlDeviceV1::selection);
    QSignalSpy serverSelectionChangedSpy(m_seat, &SeatInterface::selectionChanged);

    // Client A has a data source
    QScopedPointer<KWayland::Client::DataControlSourceV1> testSelection(new KWayland::Client::DataControlSourceV1);
    testSelection.reset(m_dataControlDeviceManager->createDataSource());
    dataControlDevice->setSelection(0,testSelection.data());
    // klipper gets it
    newOfferSpy.wait();

    // Client A deletes it
    testSelection.reset();


    // Client A sets something else
    QScopedPointer<KWayland::Client::DataControlSourceV1> testSelection2(new KWayland::Client::DataControlSourceV1);
    testSelection2.reset(m_dataControlDeviceManager->createDataSource());
    dataControlDevice->setSelection(0,testSelection2.data());
    // Meanwhile klipper updates with the old content
    QScopedPointer<KWayland::Client::DataControlSourceV1> source(new KWayland::Client::DataControlSourceV1);
    source.reset(m_dataControlDeviceManager->createDataSource());
    source->offer("fromKlipper/test1");
    source->offer("application/x-kde-onlyReplaceEmpty");

    dataControlDevice->setSelection(0,source.data());

    serverSelectionChangedSpy.wait();
    QVERIFY(m_seat->selection());
}

QTEST_GUILESS_MAIN(DataControlInterfaceTest)

#include "test_datacontrol.moc"
