#pragma once

#ifdef DEFINES_ACC_EXPORT
#define ACC_API __declspec(dllexport)
#else
#define ACC_API __declspec(dllimport)
#endif

#include <QVariant>
#include <qdebug.h>
#include <memory>

class ACC_API AcquisitionCodec
{
public:
    virtual ~AcquisitionCodec() {}
    virtual void setRecordInfo(const QVariantMap& recordInfo) = 0;
    virtual int  startRecord(bool b)                          = 0;
    virtual int  pauseRecord()                                = 0;
    virtual int  resumeRecord()                               = 0;
    virtual int  stopRecord()                                 = 0;
};

ACC_API std::unique_ptr<AcquisitionCodec> createRecorder(const QVariantMap& recordInfo);
