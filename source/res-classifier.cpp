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
	m_pFormatCtx(NULL), m_pCodecCtx(NULL), m_pCodec(NULL), m_pFrame(NULL), m_videoStream(-1)
{
	if (!initialized)
	{
		av_register_all();
		initialized = true;
	}
}

ResClassifier::~ResClassifier()
{
	if (m_pFrame)		av_free(m_pFrame);
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
	// Scale to 1080p
	// Compute difference to original

	// Scale to 480p
	// Scale to 1080p
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
	for (int i=0; i<m_pFormatCtx->nb_streams; i++)
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
	if(m_pFrame==NULL)
		return -1;


	return 0;
}
