/* ntndarray.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvDataCPP is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */

#include <algorithm>

#include <pv/ntndarray.h>

using namespace std;
using namespace epics::pvData;

namespace epics { namespace nt {

const std::string NTNDArray::URI("uri:ev4:nt/2012/pwd:NTNDArray");
const std::string ntAttrStr("uri:ev4:nt/2012/pwd:NTAttribute");

static FieldCreatePtr fieldCreate = getFieldCreate();
static PVDataCreatePtr pvDataCreate = getPVDataCreate();

bool NTNDArray::is_a(StructureConstPtr const & structure)
{
    return structure->getID() == URI;
}


NTNDArrayPtr NTNDArray::create(epics::pvData::PVStructurePtr const &pvStructure)
{
     return NTNDArrayPtr(new NTNDArray(pvStructure));
}

NTNDArrayPtr NTNDArray::create(bool hasDescriptor,
        bool hasTimeStamp, bool hasAlarm, bool hasDisplay)
{
    PVStructurePtr pvStructure = pvDataCreate->createPVStructure(
        createStructure(hasDescriptor, hasTimeStamp, hasAlarm, hasDisplay));
    return NTNDArrayPtr(new NTNDArray(pvStructure));
}


NTNDArrayPtr NTNDArray::create()
{
    return create(true,true,true,true);
}



StructureConstPtr NTNDArray::createStructure(bool hasDescriptor,
    bool hasTimeStamp, bool hasAlarm, bool hasDisplay)
{
    enum
    {
        DISCRIPTOR_INDEX,
        TIMESTAMP_INDEX,
        ALARM_INDEX,
        DISPLAY_INDEX
    };

    const size_t NUMBER_OF_INDICES = DISPLAY_INDEX+1;
    const size_t NUMBER_OF_STRUCTURES = 1 << NUMBER_OF_INDICES;

    static epics::pvData::StructureConstPtr ntndarrayStruc[NUMBER_OF_STRUCTURES];
    static epics::pvData::StructureConstPtr codecStruc;
    static epics::pvData::StructureConstPtr dimensionStruc;
    static epics::pvData::StructureConstPtr attributeStruc;

    static Mutex mutex;
    Lock xx(mutex);

    size_t index = 0;
    if (hasDescriptor) index  |= 1 << DISCRIPTOR_INDEX;
    if (hasTimeStamp)  index  |= 1 << TIMESTAMP_INDEX;
    if (hasAlarm)      index  |= 1 << ALARM_INDEX;
    if (hasDisplay)    index  |= 1 << DISPLAY_INDEX;

    if (ntndarrayStruc[index] == NULL)
    {
        StandardFieldPtr standardField = getStandardField();
        FieldBuilderPtr fb = fieldCreate->createFieldBuilder();

        UnionConstPtr codecParameters = fb->createUnion();

        if (codecStruc == NULL)
        {
            codecStruc = fb->setId("codec_t")->
                add("name", pvString)->
                add("parameters", codecParameters)->
                createStructure();
        }

        if (dimensionStruc == NULL)
        {
            dimensionStruc = fb->setId("dimension_t")->
                add("size", pvInt)->
                add("offset",  pvInt)->
                add("fullSize",  pvInt)->
                add("binning",  pvInt)->
                add("reverse",  pvBoolean)->
                createStructure();
        }

        if (attributeStruc == NULL)
        {
            attributeStruc = fb->setId(ntAttrStr)->
		        add("name", pvString)->
			    add("value", fieldCreate->createVariantUnion())->
			    add("description", pvString)->
                add("sourceType", pvInt)->
                add("source", pvString)->
               createStructure();
        }

        fb->setId(URI)->
            add("value", makeValueType())->
            add("compressedSize", pvLong)->
            add("uncompressedSize", pvLong)->
            add("codec", codecStruc)->
            addArray("dimension", dimensionStruc)->
            add("dataTimeStamp", standardField->timeStamp())->
            add("uniqueId", pvInt)->
            addArray("attribute", attributeStruc);

        if (hasDescriptor) fb->add("descriptor", pvString);
        if (hasAlarm)      fb->add("alarm", standardField->alarm());
        if (hasTimeStamp)  fb->add("timeStamp", standardField->timeStamp());
        if (hasDisplay)  fb->add("display", standardField->display());

        ntndarrayStruc[index] = fb->createStructure();
    }

    return ntndarrayStruc[index];
}

UnionConstPtr NTNDArray::makeValueType()
{
    static epics::pvData::UnionConstPtr valueType;

    if (valueType == NULL)
    {
        FieldBuilderPtr fb = getFieldCreate()->createFieldBuilder();
        
        for (int i = pvBoolean; i < pvString; ++i)
        {
            ScalarType st = static_cast<ScalarType>(i);
            fb->addArray(std::string(ScalarTypeFunc::name(st)) + "Value", st);
        }

        valueType = fb->createUnion();                
    }
    return valueType;
}


bool NTNDArray::attachTimeStamp(PVTimeStamp &pvTimeStamp) const
{
    PVStructurePtr ts = getTimeStamp();
    if (ts)
        return pvTimeStamp.attach(ts);
    else
        return false;
}

bool NTNDArray::attachDataTimeStamp(PVTimeStamp &pvTimeStamp) const
{
    PVStructurePtr ts = getDataTimeStamp();
    if (ts)
        return pvTimeStamp.attach(ts);
    else
        return false;
}

bool NTNDArray::attachAlarm(PVAlarm &pvAlarm) const
{
    PVStructurePtr al = getAlarm();
    if (al)
        return pvAlarm.attach(al);
    else
        return false;
}

PVStructurePtr NTNDArray::getPVStructure() const
{
    return pvNTNDArray;
}

PVUnionPtr NTNDArray::getValue() const
{
    return pvNTNDArray->getSubField<PVUnion>("value");
}

PVLongPtr NTNDArray::getCompressedDataSize() const
{
    return pvNTNDArray->getSubField<PVLong>("compressedSize");
}

PVLongPtr NTNDArray::getUncompressedDataSize() const
{
    return pvNTNDArray->getSubField<PVLong>("uncompressedSize");
}

PVStructurePtr NTNDArray::getCodec() const
{
    return pvNTNDArray->getSubField<PVStructure>("codec");
}

PVStructureArrayPtr NTNDArray::getAttribute() const
{
    return pvNTNDArray->getSubField<PVStructureArray>("attribute");
}

PVStructureArrayPtr NTNDArray::getDimension() const
{
    return pvNTNDArray->getSubField<PVStructureArray>("dimension");
}

PVStructurePtr NTNDArray::getDataTimeStamp() const
{
    return pvNTNDArray->getSubField<PVStructure>("dataTimeStamp");
}

PVStringPtr NTNDArray::getDescriptor() const
{
    return pvNTNDArray->getSubField<PVString>("descriptor");
}

PVStructurePtr NTNDArray::getTimeStamp() const
{
    return pvNTNDArray->getSubField<PVStructure>("timeStamp");
}

PVStructurePtr NTNDArray::getAlarm() const
{
    return pvNTNDArray->getSubField<PVStructure>("alarm");
}


NTNDArray::NTNDArray(PVStructurePtr const & pvStructure) :
    pvNTNDArray(pvStructure)
{}


}}
