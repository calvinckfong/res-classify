/*
 * res-classifier.cpp
 *
 *  Created on: 2021年2月20日
 *      Author: calvin.fong
 */

#include "res-classifier.h"
#include <iostream>

using namespace std;

bool ResClassifier::initialized = false;

ResClassifier::ResClassifier() :
	m_pFormatCtx(NULL), m_pCodecCtx(NULL), m_pCodec(NULL),
	m_buffer480p(NULL), m_buffer720p(NULL), m_buffer1080p1(NULL), m_buffer1080p2(NULL),
	m_pFrame(NULL), m_pFrame480p(NULL), m_pFrame720p(NULL), m_pFrame1080p1(NULL), m_pFrame1080p2(NULL),
	m_sws_ctx_480p(NULL), m_sws_ctx_720p(NULL), m_sws_ctx_1080p1(NULL), m_sws_ctx_1080p2(NULL),
	m_videoStream(-1)
{
	if (!initialized)
	{
		av_register_all();
		initialized = true;
	}
}

ResClassifier::~ResClassifier()
{
	if (m_buffer480p)		av_free(m_buffer480p);
	if (m_buffer720p)		av_free(m_buffer720p);
	if (m_buffer1080p1)	av_free(m_buffer1080p1);
	if (m_buffer1080p2)	av_free(m_buffer1080p2);

	if (m_pFrame)			av_free(m_pFrame);
	if (m_pFrame480p)		av_free(m_pFrame480p);
	if (m_pFrame720p)		av_free(m_pFrame720p);
	if (m_pFrame1080p1)	av_free(m_pFrame1080p1);
	if (m_pFrame1080p2)	av_free(m_pFrame1080p2);

	if (m_pCodecCtx)	avcodec_close(m_pCodecCtx);
	if (m_pFormatCtx)	avformat_close_input(&m_pFormatCtx);
}

void ResClassifier::Classify(const char* filename)
{
	// Open file
	if (OpenFile(filename) == 0)
	{
		// Loop until end of file
		int 		frameCnt=0;
		AVPacket	packet;
		int frameFinished;
		while (av_read_frame(m_pFormatCtx, &packet)>=0)
		{
			if (packet.stream_index == m_videoStream)
			{
				avcodec_decode_video2(m_pCodecCtx, m_pFrame, &frameFinished, &packet);
				if (frameFinished)
				{
					// Scale to 720p
					sws_scale(m_sws_ctx_720p, m_pFrame->data, m_pFrame->linesize, 0, m_pCodecCtx->height, m_pFrame720p->data, m_pFrame720p->linesize);
					// Scale to 1080p
					sws_scale(m_sws_ctx_1080p1, m_pFrame720p->data, m_pFrame720p->linesize, 0, m_pFrame720p->height, m_pFrame1080p1->data, m_pFrame1080p1->linesize);
					// Compute difference to original

					// Scale to 480p
					sws_scale(m_sws_ctx_480p, m_pFrame->data, m_pFrame->linesize, 0, m_pCodecCtx->height, m_pFrame480p->data, m_pFrame480p->linesize);
					// Scale to 1080p
					sws_scale(m_sws_ctx_1080p2, m_pFrame480p->data, m_pFrame480p->linesize, 0, m_pFrame480p->height, m_pFrame1080p2->data, m_pFrame1080p2->linesize);
					// Compute difference to original

#ifdef DEBUG
					cout << "Frame " << frameCnt << endl;
#endif
					frameCnt++;
				}
			}
		}
		av_free_packet(&packet);
	}
	else
		cout << "Unable to open " << filename << endl;
}

int ResClassifier::OpenFile(const char* filename)
{
	if (avformat_open_input(&m_pFormatCtx, filename, NULL, NULL) != 0)
	{
		return -1;
	}

	if (avformat_find_stream_info(m_pFormatCtx, NULL)<0)
	{
		return -1;
	}

#ifdef DEBUG
	av_dump_format(m_pFormatCtx, 0, filename, 0);
#endif

	// Seek Video stream
	m_videoStream = -1;
	for (unsigned int i=0; i<m_pFormatCtx->nb_streams; i++)
	{
		if (m_pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
		{
			m_videoStream = i;
			break;
		}
	}
	if (m_videoStream<0)
	{
		cout << "Unable to find video stream." << endl;
		return -1;
	}

	// Seek decoder
	m_pCodecCtx=m_pFormatCtx->streams[m_videoStream]->codec;
	m_pCodec=avcodec_find_decoder(m_pCodecCtx->codec_id);
	if (m_pCodec==NULL) {
		cout << "No codec supported" << endl;
		return -1; // Codec not found
	}

	// Open codec
	AVDictionary *optionsDict = NULL;
	if(avcodec_open2(m_pCodecCtx, m_pCodec, &optionsDict)<0)
	{
		cout << "Unable to open codec" << endl;
		return -1;
	}

	// Allocate video frame
	m_pFrame=av_frame_alloc();
	if(m_pFrame==NULL) return -1;

	m_pFrame480p=av_frame_alloc();
	if(m_pFrame480p==NULL) return -1;

	m_pFrame720p=av_frame_alloc();
	if(m_pFrame720p==NULL) return -1;

	m_pFrame1080p1=av_frame_alloc();
	if(m_pFrame1080p1==NULL) return -1;

	m_pFrame1080p2=av_frame_alloc();
	if(m_pFrame1080p2==NULL) return -1;


	// Create Frames
	AVPixelFormat pix_fmt = m_pCodecCtx->pix_fmt;
	int numBytes=0;

	numBytes=avpicture_get_size(pix_fmt, 854, 480);
	m_buffer480p = (uint8_t*)av_malloc(numBytes*sizeof(uint8_t));
	avpicture_fill((AVPicture *)m_pFrame480p, m_buffer480p, pix_fmt, 854, 480);

	numBytes=avpicture_get_size(pix_fmt, 1280, 720);
	m_buffer720p = (uint8_t*)av_malloc(numBytes*sizeof(uint8_t));
	avpicture_fill((AVPicture *)m_pFrame720p, m_buffer720p, pix_fmt, 1280, 720);

	numBytes=avpicture_get_size(pix_fmt, 1920, 1080);
	m_buffer1080p1 = (uint8_t*)av_malloc(numBytes*sizeof(uint8_t));
	avpicture_fill((AVPicture *)m_pFrame1080p1, m_buffer1080p1, pix_fmt, 1920, 1080);

	m_buffer1080p2 = (uint8_t*)av_malloc(numBytes*sizeof(uint8_t));
	avpicture_fill((AVPicture *)m_pFrame1080p2, m_buffer1080p2, pix_fmt, 1920, 1080);

	// Create scaling contexts
	m_sws_ctx_480p = sws_getContext(
			m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
			854, 480, m_pCodecCtx->pix_fmt, SWS_BILINEAR,
			NULL, NULL, NULL);

	m_sws_ctx_720p = sws_getContext(
			m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
			1280, 720, m_pCodecCtx->pix_fmt, SWS_BILINEAR,
			NULL, NULL, NULL);

	m_sws_ctx_1080p1 = sws_getContext(
			854, 480, m_pCodecCtx->pix_fmt,
			m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt, SWS_BILINEAR,
			NULL, NULL, NULL);

	m_sws_ctx_1080p2 = sws_getContext(
			1280, 720, m_pCodecCtx->pix_fmt,
			m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt, SWS_BILINEAR,
			NULL, NULL, NULL);

	return 0;
}
