/*
 * Qt-based tests for ParameterManager
 *
 * Tests ParameterManager functionality including:
 * - Property getting/setting via Qt property system
 * - Validity state management
 * - Signal emission (paramChanged)
 */
#include "ui/parameterManager.h"
#include <QtTest/QtTest>

// Test subclass to allow setting justInitialized for testing
class TestableParameterManager : public ParameterManager
{
  Q_OBJECT
public:
  TestableParameterManager() : ParameterManager() {}

  // Reset justInitialized flag so setters work
  void enableForTesting() { justInitialized = false; }
};

class TestParameterManager : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  // Property value tests (get/set via Qt property system)
  void testExposureCompProperty();
  void testTemperatureProperty();
  void testTintProperty();
  void testBlackpointProperty();
  void testWhitepointProperty();
  void testVibranceProperty();
  void testSaturationProperty();

  // Validity state tests
  void testValidityInitialState();
  void testSetValid();

  // Default getter tests
  void testDefaultValues();

  // paramChanged signal test
  void testParamChangedSignal();

private:
  TestableParameterManager *pm;
};

void TestParameterManager::initTestCase()
{
  pm = new TestableParameterManager();
  pm->enableForTesting();
}

void TestParameterManager::cleanupTestCase() { delete pm; }

void TestParameterManager::testExposureCompProperty()
{
  // Test that property can be set and retrieved
  bool success = pm->setProperty("exposureComp", 2.5f);
  QVERIFY(success);
  QCOMPARE(pm->getExposureComp(), 2.5f);
}

void TestParameterManager::testTemperatureProperty()
{
  bool success = pm->setProperty("temperature", 5500.0f);
  QVERIFY(success);
  QCOMPARE(pm->getTemperature(), 5500.0f);
}

void TestParameterManager::testTintProperty()
{
  bool success = pm->setProperty("tint", 0.8f);
  QVERIFY(success);
  QCOMPARE(pm->getTint(), 0.8f);
}

void TestParameterManager::testBlackpointProperty()
{
  bool success = pm->setProperty("blackpoint", 0.02f);
  QVERIFY(success);
  QCOMPARE(pm->getBlackpoint(), 0.02f);
}

void TestParameterManager::testWhitepointProperty()
{
  bool success = pm->setProperty("whitepoint", 0.98f);
  QVERIFY(success);
  QCOMPARE(pm->getWhitepoint(), 0.98f);
}

void TestParameterManager::testVibranceProperty()
{
  bool success = pm->setProperty("vibrance", 1.5f);
  QVERIFY(success);
  QCOMPARE(pm->getVibrance(), 1.5f);
}

void TestParameterManager::testSaturationProperty()
{
  bool success = pm->setProperty("saturation", 1.2f);
  QVERIFY(success);
  QCOMPARE(pm->getSaturation(), 1.2f);
}

void TestParameterManager::testValidityInitialState()
{
  // ParameterManager should start with validity = none
  Valid validity = pm->getValid();
  QCOMPARE(validity, Valid::none);
}

void TestParameterManager::testSetValid()
{
  pm->setValid(Valid::postdemosaic);
  Valid validity = pm->getValid();
  QCOMPARE(validity, Valid::postdemosaic);

  // Reset
  pm->setValid(Valid::none);
}

void TestParameterManager::testDefaultValues()
{
  // Test that default getters return reasonable values
  float defTemp = pm->getDefTemperature();
  QVERIFY(defTemp > 0);

  float defTint = pm->getDefTint();
  QVERIFY(defTint > 0);

  float defExp = pm->getDefExposureComp();
  QVERIFY(defExp >= -10.0f && defExp <= 10.0f);
}

void TestParameterManager::testParamChangedSignal()
{
  QSignalSpy spy(pm, &ParameterManager::paramChanged);
  QVERIFY(spy.isValid());

  // Setting a property should emit paramChanged
  pm->setProperty("exposureComp", 3.0f);

  QCOMPARE(spy.count(), 1);

  // Verify the argument was the setter name
  QList<QVariant> args = spy.takeFirst();
  QVERIFY(args.at(0).toString().contains("Exposure"));
}

QTEST_MAIN(TestParameterManager)
#include "test_parameter_manager.moc"
