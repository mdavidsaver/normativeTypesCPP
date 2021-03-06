/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * This software is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */

#include <epicsUnitTest.h>
#include <testMain.h>

#include <pv/nt.h>

using namespace epics::nt;
using namespace epics::pvData;
using std::tr1::dynamic_pointer_cast;

static FieldCreatePtr fieldCreate = getFieldCreate();

void test_builder()
{
    testDiag("test_builder");

    NTMatrixBuilderPtr builder = NTMatrix::createBuilder();
    testOk(builder.get() != 0, "Got builder");

    StructureConstPtr structure = builder->
            //arrayValue(pvDouble)->
            addDim()->
            addDescriptor()->
            addAlarm()->
            addTimeStamp()->
            addDisplay()->
            add("extra1",fieldCreate->createScalar(pvString)) ->
            add("extra2",fieldCreate->createScalarArray(pvString)) ->
            createStructure();
    testOk1(structure.get() != 0);
    if (!structure)
        return;

    testOk1(NTMatrix::is_a(structure));
    testOk1(structure->getID() == NTMatrix::URI);
    testOk1(structure->getNumberFields() == 8);
    testOk1(structure->getField("value").get() != 0);
    testOk1(structure->getField("dim").get() != 0);
    testOk1(structure->getField("descriptor").get() != 0);
    testOk1(structure->getField("alarm").get() != 0);
    testOk1(structure->getField("timeStamp").get() != 0);
    testOk1(structure->getField("display").get() != 0);

    testOk(dynamic_pointer_cast<const ScalarArray>(structure->getField("value")).get() != 0 &&
            dynamic_pointer_cast<const ScalarArray>(structure->getField("value"))->getElementType() == pvDouble, "value type");

    std::cout << *structure << std::endl;
}

void test_ntmatrix()
{
    testDiag("test_ntmatrix");

    NTMatrixBuilderPtr builder = NTMatrix::createBuilder();
    testOk(builder.get() != 0, "Got builder");

    NTMatrixPtr ntScalarArray = builder->
            //arrayValue(pvInt)->
            addDim()->
            addDescriptor()->
            addAlarm()->
            addTimeStamp()->
            addDisplay()->
            create();
    testOk1(ntScalarArray.get() != 0);

    testOk1(ntScalarArray->getPVStructure().get() != 0);
    testOk1(ntScalarArray->getValue().get() != 0);
    testOk1(ntScalarArray->getDim().get() != 0);
    testOk1(ntScalarArray->getDescriptor().get() != 0);
    testOk1(ntScalarArray->getAlarm().get() != 0);
    testOk1(ntScalarArray->getTimeStamp().get() != 0);
    testOk1(ntScalarArray->getDisplay().get() != 0);

    //
    // example how to set values
    //
    PVDoubleArray::svector newValues;
    newValues.push_back(1.0);
    newValues.push_back(2.0);
    newValues.push_back(8.0);

    PVDoubleArrayPtr pvValueField = ntScalarArray->getValue();
    pvValueField->replace(freeze(newValues));

    //
    // example how to get values
    //
    PVDoubleArray::const_svector values(pvValueField->view());

    testOk1(values.size() == 3);
    testOk1(values[0] == 1.0);
    testOk1(values[1] == 2.0);
    testOk1(values[2] == 8.0);

    //
    // timeStamp ops
    //
    PVTimeStamp pvTimeStamp;
    if (ntScalarArray->attachTimeStamp(pvTimeStamp))
    {
        testPass("timeStamp attach");

        // example how to set current time
        TimeStamp ts;
        ts.getCurrent();
        pvTimeStamp.set(ts);

        // example how to get EPICS time
        TimeStamp ts2;
        pvTimeStamp.get(ts2);
        testOk1(ts2.getEpicsSecondsPastEpoch() != 0);
    }
    else
        testFail("timeStamp attach fail");

    //
    // alarm ops
    //
    PVAlarm pvAlarm;
    if (ntScalarArray->attachAlarm(pvAlarm))
    {
        testPass("alarm attach");

        // example how to set an alarm
        Alarm alarm;
        alarm.setStatus(deviceStatus);
        alarm.setSeverity(minorAlarm);
        alarm.setMessage("simulation alarm");
        pvAlarm.set(alarm);
    }
    else
        testFail("alarm attach fail");

    //
    // display ops
    //
    PVDisplay pvDisplay;
    if (ntScalarArray->attachDisplay(pvDisplay))
    {
        testPass("display attach");

        // example how to set an display
        Display display;
        display.setLow(-15);
        display.setHigh(15);
        display.setDescription("This is a test scalar array");
        display.setFormat("%d");
        display.setUnits("A");
        pvDisplay.set(display);
    }
    else
        testFail("display attach fail");

    //
    // set descriptor
    //
    ntScalarArray->getDescriptor()->put("This is a test NTMatrix");

    // dump ntScalarArray
    std::cout << *ntScalarArray->getPVStructure() << std::endl;

}


void test_wrap()
{
    testDiag("test_wrap");

    NTMatrixPtr nullPtr = NTMatrix::wrap(PVStructurePtr());
    testOk(nullPtr.get() == 0, "nullptr wrap");

    nullPtr = NTMatrix::wrap(
                getPVDataCreate()->createPVStructure(
                    NTField::get()->createTimeStamp()
                    )
                );
    testOk(nullPtr.get() == 0, "wrong type wrap");


    NTMatrixBuilderPtr builder = NTMatrix::createBuilder();
    testOk(builder.get() != 0, "Got builder");

    PVStructurePtr pvStructure = builder->
            createPVStructure();
    testOk1(pvStructure.get() != 0);
    if (!pvStructure)
        return;

    testOk1(NTMatrix::isCompatible(pvStructure)==true);
    NTMatrixPtr ptr = NTMatrix::wrap(pvStructure);
    testOk(ptr.get() != 0, "wrap OK");

    ptr = NTMatrix::wrapUnsafe(pvStructure);
    testOk(ptr.get() != 0, "wrapUnsafe OK");
}

MAIN(testNTMatrix) {
    testPlan(36);
    test_builder();
    test_ntmatrix();
    test_wrap();
    return testDone();
}


