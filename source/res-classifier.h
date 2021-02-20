/*
 * res-classifier.h
 *
 *  Created on: 2021年2月20日
 *      Author: calvin.fong
 */

#ifndef SOURCE_RES_CLASSIFIER_H_
#define SOURCE_RES_CLASSIFIER_H_

extern "C" {
#include <libavformat/avformat.h>
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
	AVFrame *m_pFrame;
	int m_videoStream;

	int OpenFile(const char* filename);

};



#endif /* SOURCE_RES_CLASSIFIER_H_ */
