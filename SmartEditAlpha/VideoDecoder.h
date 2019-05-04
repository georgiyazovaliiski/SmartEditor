#pragma once
#include <string>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}

class VideoDecoder
{
public:
	VideoDecoder();
	~VideoDecoder();

	AVFormatContext *format_context;
	AVCodecContext *video_decode_context = NULL, *audio_decode_context;


	AVStream *video_stream = NULL, *audio_stream = NULL;
	int video_stream_index = -1, audio_stream_index = -1;
	const char *video_destination_filename = NULL;
	const char *audio_destination_filename = NULL;
	FILE *video_destination_file = NULL;
	FILE *audio_destination_file = NULL;

	uint8_t *video_destination_data[4] = { NULL };
	int video_destination_linesize[4];
	int video_destination_bufsize;

	void DecodeVideoFile(std::string fileName);
	int open_codec_context(int * stream_idx,
					AVCodecContext ** dec_ctx, 
					AVFormatContext * fmt_ctx, 
					AVMediaType type,
					std::string fileName);
	std::string exportingDestination;	// Where the audio/video channel will be exported
};

