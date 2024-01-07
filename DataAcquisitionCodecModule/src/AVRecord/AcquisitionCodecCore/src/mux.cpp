#include "mux.h"
#include "FFmpegHelper.h"

#include <QDebug>
#include <QTime>
#include <QDateTime>

using namespace std;
//#define RTMP --->自定义编译选项
#ifdef RTMP
static QString rtmpUrl;
#ifndef WIN32
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<linux/fb.h>
#else
#include <windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
bool getHostNameAndIp(std::string& strWLANIp, std::string& strLocalIp);
/**
* 获取机器Ip地址和主机名
*/
bool getHostNameAndIp(std::string& strWLANIp, std::string& strLocalIp)
{
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    //得到结构体大小,用于GetAdaptersInfo参数
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    //调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
    int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    //记录网卡数量
    int netCardNum = 0;
    //记录每张网卡上的IP地址数量
    int IPnumPerNetCard = 0;
    if (ERROR_BUFFER_OVERFLOW == nRel)
    {
        //如果函数返回的是ERROR_BUFFER_OVERFLOW
        //则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
        //这也是说明为什么stSize既是一个输入量也是一个输出量
        //释放原来的内存空间
        delete pIpAdapterInfo;
        //重新申请内存空间用来存储所有网卡信息
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
        //再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
        nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    }

    std::string strMAC;
    strMAC.resize(20);

    if (ERROR_SUCCESS == nRel)
    {
        //输出网卡信息
        //可能有多网卡,因此通过循环去判断
        while (pIpAdapterInfo)
        {
            //可能网卡有多IP,因此通过循环去判断
            IP_ADDR_STRING* pIpAddrString = &(pIpAdapterInfo->IpAddressList);
            if (pIpAddrString)
            {
                sprintf_s(const_cast<char*>(strMAC.c_str()), 20, "%02X-%02X-%02X-%02X-%02X-%02X",
                    pIpAdapterInfo->Address[0], pIpAdapterInfo->Address[1], pIpAdapterInfo->Address[2], pIpAdapterInfo->Address[3], pIpAdapterInfo->Address[4], pIpAdapterInfo->Address[5]);

                // 说明是本地网络
                if (pIpAdapterInfo->Type == MIB_IF_TYPE_ETHERNET)
                {
                    if (!strstr(pIpAdapterInfo->Description, "Virtual"))
                    {
                        std::string strIp = pIpAddrString->IpAddress.String;
                        if (strIp.compare("0.0.0.0") != 0)
                        {
                            strLocalIp = strIp;
                        }
                    }
                }

                // 说明是无线网络
                if (pIpAdapterInfo->Type == 71)
                {
                    std::string strIp = pIpAddrString->IpAddress.String;
                    if (strIp.compare("0.0.0.0") != 0)
                    {
                        strWLANIp = strIp;
                    }
                }
            }
            pIpAdapterInfo = pIpAdapterInfo->Next;
        }
    }
    return true;
}
#endif
#endif

void Mux::getIp()
{
#ifdef WIN32
    std::string ip, host;
#ifdef RTMP
    getHostNameAndIp(ip, host);
    rtmpUrl = QString::fromStdString(host);
    rtmpUrl = "rtmp://" + rtmpUrl;
    rtmpUrl += "/live/stream";
    qCritical() << "[getIp]rtmpUrl: " << rtmpUrl;
#endif
#endif
    return ;
}

int Mux::init(const std::string& filename)
{
    m_filename = filename;
    const char* outFileName = m_filename.c_str();
    #ifndef RTMP
    int ret = avformat_alloc_output_context2(&m_oFmtCtx, nullptr, nullptr, outFileName); //媒体文件
    #else
    getIp();
    int ret = avformat_alloc_output_context2(&m_oFmtCtx, nullptr, "flv", rtmpUrl.toLocal8Bit().data());  //实时流
    #endif
    if (ret < 0)
    {
        qCritical() << "avformat_alloc_output_context2 failed";
        return -1;
    }
    m_isInit = true;
    return 0;
}

void Mux::deinit()
{
    m_isInit = false;
    if (m_oFmtCtx) {
        avio_close(m_oFmtCtx->pb);
        avformat_free_context(m_oFmtCtx);
        m_oFmtCtx = nullptr;
    }
    m_vStream = nullptr; // avformat_free_context时释放
    m_aStream = nullptr;
    m_vEncodeCtx = nullptr;
    m_aEncodeCtx = nullptr;
}

int Mux::writeHeader()
{
    if (!m_isInit || !m_oFmtCtx) return -1;

    //打开输出文件
    if (!(m_oFmtCtx->oformat->flags & AVFMT_NOFILE))
    {
        #ifndef RTMP
        const char* outFileName = m_filename.c_str();
        #else
        const char* outFileName = rtmpUrl.toLocal8Bit().data();
        #endif
        if (avio_open(&m_oFmtCtx->pb, outFileName, AVIO_FLAG_WRITE) < 0)
        {
            qCritical() << "can not open output file handle!";
            return -1;
        }
    }
    //写文件头
    if (avformat_write_header(m_oFmtCtx, nullptr) < 0)
    {
        qCritical() << "can not write the header of the output file!";
        return -1;
    }

    return 0;
}

int Mux::writePacket(AVPacket* packet, int64_t captureTime)
{
    if (!packet || packet->size <= 0 || !packet->data)
    {
        qCritical() << "packet is null";
        if (packet)
            av_packet_free(&packet);
        return -1;
    }

    if (!m_isInit || !m_oFmtCtx) return -1;

    int stream_index = packet->stream_index;
    AVRational src_time_base;  // 编码后的包
    AVRational dst_time_base;  // mp4输出文件对应流的time_base
    if (m_vStream && m_vEncodeCtx && stream_index == m_vIndex)
    {
        src_time_base = m_vEncodeCtx->time_base;
        dst_time_base = m_vStream->time_base;
    }
    else if (m_aStream && m_aEncodeCtx && stream_index == m_aIndex)
    {
        src_time_base = m_aEncodeCtx->time_base;
        dst_time_base = m_aStream->time_base;
    }
#if 0
    packet->pts = av_rescale_q(packet->pts, src_time_base, dst_time_base);
    packet->dts = av_rescale_q(packet->dts, src_time_base, dst_time_base);
    packet->duration = av_rescale_q(packet->duration, src_time_base, dst_time_base);
#else
    packet->pts = av_rescale_q(captureTime, AVRational{ 1, /*1000*/ 1000*1000 }, dst_time_base);
    //qDebug() << "Index:" << stream_index << " pts:" << packet->pts << " captureTime:" << captureTime;
    packet->dts = packet->pts;
#endif
    int ret;
    lock_guard<mutex> lock(m_WriteFrameMtx);
    // av_interleaved_write_frame调用后packet的各个字段变为0
#if 0
    qDebug() << QString("av_interleaved_write_frame, stream_index=%1, pts=%2, dts=%3, duration=%4, size=%5")
                    .arg(stream_index)
                    .arg(packet->pts)
                    .arg(packet->dts)
                    .arg(packet->duration)
                    .arg(packet->size);
#endif
    // 相同dts会导致av_interleaved_write_frame返回Invalid argument（-22）
     ret = av_interleaved_write_frame(m_oFmtCtx, packet);
    av_packet_free(&packet);
    if (ret != 0)
    {
        qCritical() << QString("stream_index=%1, av_interleaved_write_frame failed: %2").arg(stream_index).arg(FFmpegHelper::err2Str(ret));
        return -1;
    }
#if 0
    static int s_writeCnt = 0;
    ++s_writeCnt;
    qDebug() << "s_writeCnt" << s_writeCnt;
#endif
    return 0;
}

int Mux::writeTrailer()
{
    if (!m_isInit || !m_oFmtCtx) return -1;
    int ret = av_write_trailer(m_oFmtCtx);
    return 0;
}

int Mux::addStream(AVCodecContext* encodeCtx)
{
    if (!m_isInit || !m_oFmtCtx || !encodeCtx) return -1;

    AVStream* stream = avformat_new_stream(m_oFmtCtx, nullptr);
	if (!stream)
	{
        qCritical() << "avformat_new_stream failed";
		return -1;
	}

	//将codecCtx中的参数传给输出流
	int ret = avcodec_parameters_from_context(stream->codecpar, encodeCtx);
	if (ret < 0)
	{
		qCritical() << "Output avcodec_parameters_from_context,error code:" << ret;
		return -1;
	}
    if (encodeCtx->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        m_vEncodeCtx = encodeCtx;
        m_vStream = stream;
        m_vIndex = stream->index;
        qInfo() << "Video stream index" << m_vIndex;
    }
    else if (encodeCtx->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        m_aEncodeCtx = encodeCtx;
        m_aStream = stream;
        m_aIndex = stream->index;
        qInfo() << "Audio stream index" << m_aIndex;
    }
    return 0;
}
