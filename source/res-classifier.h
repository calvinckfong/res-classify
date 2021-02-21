/*
 * res-classifier.h
 *
 *  Created on: 2021年2月20日
 *      Author: calvin.fong
 */

#ifndef SOURCE_RES_CLASSIFIER_H_
#define SOURCE_RES_CLASSIFIER_H_

#include <unistd.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class ResClassifier
{
public:
	ResClassifier();
	~ResClassifier();

	void Classify(const char* filename);

private:
	static bool initialized;

	AVFormatContext *m_pFormatCtx;
	AVCodecContext *m_pCodecCtx;
	AVCodec *m_pCodec;
	uint8_t *m_buffer480p, *m_buffer720p, *m_buffer1080p1, *m_buffer1080p2;
	AVFrame *m_pFrame, *m_pFrame480p, *m_pFrame720p, *m_pFrame1080p1, *m_pFrame1080p2;
	struct SwsContext *m_sws_ctx_480p;
	struct SwsContext *m_sws_ctx_720p;
	struct SwsContext *m_sws_ctx_1080p1;
	struct SwsContext *m_sws_ctx_1080p2;

	int m_videoStream;

	int OpenFile(const char* filename);
	AVFrame* AllocateFrame(AVPixelFormat pix_fmt, int width, int height);
	int SaveFrame(AVFrame* pFrame, const char* filename);
};



#endif /* SOURCE_RES_CLASSIFIER_H_ */
