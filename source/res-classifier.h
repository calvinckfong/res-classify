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

typedef enum ClassifyMethod_ {
	MSE_Method,
	HighPass_Method
} ClassifyMethod;

class ResClassifier
{
public:
	ResClassifier(int method);
	~ResClassifier();

	void Classify(const char* filename);

private:
	static bool initialized;

	AVFormatContext *m_pFormatCtx;
	AVCodecContext *m_pCodecCtx;
	AVCodec *m_pCodec;
	uint32_t	m_ySize;
	uint8_t *m_buffer480p, *m_buffer720p, *m_buffer1080p1, *m_buffer1080p2;
	uint8_t *m_prevBuffer1, *m_prevBuffer2;
	AVFrame *m_pFrame, *m_pFrame480p, *m_pFrame720p, *m_pFrame1080p1, *m_pFrame1080p2;
	struct SwsContext *m_sws_ctx_480p;
	struct SwsContext *m_sws_ctx_720p;
	struct SwsContext *m_sws_ctx_1080p1;
	struct SwsContext *m_sws_ctx_1080p2;

	int m_videoStream;
	ClassifyMethod m_method;

	int OpenFile(const char* filename);
	AVFrame* AllocateFrame(AVPixelFormat pix_fmt, int width, int height);

	void MseAndCompare(int frameCnt);
	double ComputeMSE(AVFrame* frame1, AVFrame* frame2);

	void HighPassAndCompare(int frameCnt);

	void ComputeHighPass(AVFrame* frame, uint8_t* prevLuma, double* hp_res, double* hor_res, double* ver_res);

	int SaveFrame(AVFrame* pFrame, const char* filename);
};



#endif /* SOURCE_RES_CLASSIFIER_H_ */
