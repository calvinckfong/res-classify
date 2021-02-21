/*
 * res-classifier.cpp
 *
 *  Created on: 2021年2月20日
 *      Author: calvin.fong
 */

#include "res-classifier.h"
#include <iostream>
#include <assert.h>

extern "C" {
#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
//#include "libavresample/avresample.h"
#include "libswscale/swscale.h"
}

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
		int frameFinished, h;
		while (av_read_frame(m_pFormatCtx, &packet)>=0)
		{
			if (packet.stream_index == m_videoStream)
			{
				avcodec_decode_video2(m_pCodecCtx, m_pFrame, &frameFinished, &packet);
				if (frameFinished)
				{
					SaveFrame(m_pFrame, "origin.yuv");

					// Scale to 720p
					sws_scale(m_sws_ctx_720p, (const uint8_t* const*)m_pFrame->data, m_pFrame->linesize,
							0, m_pCodecCtx->height, (uint8_t* const*)m_pFrame720p->data, m_pFrame720p->linesize);
					SaveFrame(m_pFrame720p, "720p.yuv");
					// Scale to 1080p
					sws_scale(m_sws_ctx_1080p1, (const uint8_t* const*)m_pFrame720p->data, m_pFrame720p->linesize,
							0, m_pFrame720p->height, (uint8_t* const*)m_pFrame1080p1->data, m_pFrame1080p1->linesize);
					SaveFrame(m_pFrame1080p1, "1080p1.yuv");
					// Compute difference to original

					// Scale to 480p
					sws_scale(m_sws_ctx_480p, m_pFrame->data, m_pFrame->linesize,
							0, m_pCodecCtx->height, m_pFrame480p->data, m_pFrame480p->linesize);
					SaveFrame(m_pFrame480p, "480p.yuv");
					// Scale to 1080p
					sws_scale(m_sws_ctx_1080p2, m_pFrame480p->data, m_pFrame480p->linesize,
							0, m_pFrame480p->height, m_pFrame1080p2->data, m_pFrame1080p2->linesize);
					SaveFrame(m_pFrame1080p2, "1080p2.yuv");
					// Compute difference to original

					assert(false);
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

	m_pFrame480p = AllocateFrame(pix_fmt, 854, 480);
	m_pFrame720p = AllocateFrame(pix_fmt, 1280, 720);
	m_pFrame1080p1 = AllocateFrame(pix_fmt, 1920, 1080);
	m_pFrame1080p2 = AllocateFrame(pix_fmt, 1920, 1080);

	// Create scaling contexts
	m_sws_ctx_720p = sws_getContext(
			m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
			1280, 720, m_pCodecCtx->pix_fmt, SWS_BICUBIC,
			NULL, NULL, NULL);

	m_sws_ctx_480p = sws_getContext(
			m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
			854, 480, m_pCodecCtx->pix_fmt, SWS_BICUBIC,
			NULL, NULL, NULL);

	m_sws_ctx_1080p1 = sws_getContext(
			1280, 720, m_pCodecCtx->pix_fmt,
			m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt, SWS_BILINEAR,
			NULL, NULL, NULL);

	m_sws_ctx_1080p2 = sws_getContext(
			854, 480, m_pCodecCtx->pix_fmt,
			m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt, SWS_BILINEAR,
			NULL, NULL, NULL);

	return 0;
}

AVFrame* ResClassifier::AllocateFrame(AVPixelFormat pix_fmt, int width, int height)
{
   AVFrame *picture;
   int ret;

   picture = av_frame_alloc();
   if (!picture)
	   return NULL;

	picture->format = pix_fmt;
	picture->width  = width;
	picture->height = height;

	/* allocate the buffers for the frame data */
	ret = av_frame_get_buffer(picture, 32);
	if (ret < 0) {
		cerr << "Could not allocate frame data." << endl;
		exit(1);
	}

	return picture;
}

int ResClassifier::SaveFrame(AVFrame* pFrame, const char* filename)
{
	FILE* fp = fopen(filename, "wb");
	if (fp)
	{
		int yHeight = pFrame->height;
		int uvHeight = pFrame->height/2;
		int yWidth = pFrame->width;
		int uvWidth = pFrame->width/2;
		int yLinesize = pFrame->linesize[0];
		int uvLinesize = pFrame->linesize[1];

		//cout << "yHeight " << yHeight << " yWidth " << yWidth << endl;

		// write Y
		for (int y=0; y<yHeight; y++)
		{
			fwrite(pFrame->data[0]+y*yLinesize, 1, yWidth, fp);
		}

		// write U
		for (int y=0; y<uvHeight; y++)
		{
			fwrite(pFrame->data[1]+y*uvLinesize, 1, uvWidth, fp);
		}

		// write V
		for (int y=0; y<uvHeight; y++)
		{
			fwrite(pFrame->data[2]+y*uvLinesize, 1, uvWidth, fp);
		}
		fclose(fp);
	}
	else
	{
		cout << "Unable to save frame" << endl;
		return -1;
	}

	return 0;
}
