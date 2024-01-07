#include "VideoEncoder.h"
#include "FFmpegHelper.h"

#include <QDebug>
#include <QTime>
#include <QDateTime>

//��ӡ���������ݴ�С
//#define VIDEO

int VideoEncoder::initH264(int width, int height, int fps)
{
    int ret = -1;

    m_vEncodeCtx = avcodec_alloc_context3(NULL);
    if (nullptr == m_vEncodeCtx)
    {
        qCritical() << "avcodec_alloc_context3 failed";
        return -1;
    }
    m_vEncodeCtx->width = width;
    m_vEncodeCtx->height = height;
    m_vEncodeCtx->time_base.num = 1;
    m_vEncodeCtx->time_base.den = fps;
    m_vEncodeCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    m_vEncodeCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    m_vEncodeCtx->codec_id = AV_CODEC_ID_H264;
#if 0
    m_vEncodeCtx->bit_rate = 800 * 1000;
    m_vEncodeCtx->rc_max_rate = 800 * 1000;
    m_vEncodeCtx->rc_buffer_size = 500 * 1000;
#endif
    //����ͼ�����Ĵ�С, gop_sizeԽ���ļ�ԽС 
    m_vEncodeCtx->gop_size = 1;//30 ����gop��С�������뿪
    m_vEncodeCtx->max_b_frames = 0;
    //����h264����صĲ���,������avcodec_open2��ʧ��
    m_vEncodeCtx->qmin = 10;	//2
    m_vEncodeCtx->qmax = 31;	//31
    m_vEncodeCtx->max_qdiff = 4;
    m_vEncodeCtx->me_range = 16;	//0	
    m_vEncodeCtx->max_qdiff = 4;	//3	
    m_vEncodeCtx->qcompress = 0.6;	//0.5
    m_vEncodeCtx->codec_tag = 0;
    //��ȷ����sps/pps
    m_vEncodeCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

#if 1
    //av_dict_set(&m_dict, "threads", "0", 0); // ����ɱ����ӳټ�֡��avcodec_receive_packet EAGAIN n�κ�ŷ��ص�һ֡��Ӧ��packet
    //av_dict_set(&m_dict, "profile", "high", 0);
    //av_dict_set(&m_dict, "crf", "16", 0);
    //av_dict_set(&m_dict, "qp", "0", 0);
#endif
#if 1
    //�����������ٶ�
    av_dict_set(&m_dict, "preset", "ultrafast", 0); // ���ڱ����ٶȺ�������ƽ�⡣//veryfast > superfast
    av_dict_set(&m_dict, "tune", "zerolatency", 0); // ���ӳ٣�������Ҫ�ǳ��͵��ӳٵ�����£�������ӵ绰����ı���
#endif

    //������Ƶ������
    const AVCodec* encoder;
    encoder = avcodec_find_encoder(m_vEncodeCtx->codec_id);
    if (!encoder)
    {
        qCritical() << "Can not find the encoder, id: " << m_vEncodeCtx->codec_id;
        return -1;
    }
    //����Ƶ������
    ret = avcodec_open2(m_vEncodeCtx, encoder, &m_dict);
    if (ret < 0)
    {
        qCritical() << "Can not open encoder id: " << encoder->id << "error code: " << ret;
        return -1;
    }
    return 0;
}

void VideoEncoder::deinit()
{
    if (m_vEncodeCtx)
    {
        avcodec_free_context(&m_vEncodeCtx);
        m_vEncodeCtx = nullptr;
    }
    av_dict_free(&m_dict);
}

int VideoEncoder::encode(AVFrame* frame, int stream_index, int64_t pts, int64_t time_base, std::vector<AVPacket*>& packets)
{
    if (!m_vEncodeCtx) return -1;

    int ret = 0;
    ret = avcodec_send_frame(m_vEncodeCtx, frame);
    //qDebug() << "avcodec_send_frame duration:" << t.elapsed() << " time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << s_cnt++;
    if (ret != 0)
    {
        qCritical() << "video avcodec_send_frame failed:" << FFmpegHelper::err2Str(ret);
        return -1;
    }

    while (1)
    {
        AVPacket* packet = av_packet_alloc();
        ret = avcodec_receive_packet(m_vEncodeCtx, packet);
        #if VIDEO
        qCritical() << "(avcodec_receive_packet)[VideoEncoder]packet.size: " << packet->size;
        #endif
        packet->stream_index = stream_index;
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            ret = 0;
            av_packet_free(&packet);
            //qDebug() << "EAGAIN time:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
            break;
        }
        else if (ret < 0)
        {
            qCritical() << "avcodec_receive_packet failed:" << FFmpegHelper::err2Str(ret);
            av_packet_free(&packet);
            ret = -1;
            break;
        }
        packets.push_back(packet);
    }
    return ret;
}
