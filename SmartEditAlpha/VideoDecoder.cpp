#include "pch.h"
#include "VideoDecoder.h"
#include <string>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}

VideoDecoder::VideoDecoder()
{
}


VideoDecoder::~VideoDecoder()
{
}

void VideoDecoder::DecodeVideoFile(std::string fileName)
{
	int width, height;
	enum AVPixelFormat pixel_format;

	char* file_name = new char[100];

	strcpy(file_name, fileName.c_str());

	/* open input file, and allocate format context */
	if (avformat_open_input(&format_context, file_name, NULL, NULL) < 0) {
		fprintf(stderr, "Could not open source file %s\n", fileName);
		exit(1);
	}

	/* retrieve stream information */
	if (avformat_find_stream_info(format_context, NULL) < 0) {
		fprintf(stderr, "Could not find stream information\n");
		exit(1);
	}

	if (open_codec_context(&video_stream_index, &video_decode_context, format_context, AVMEDIA_TYPE_VIDEO, fileName) >= 0) {
		video_stream = format_context->streams[video_stream_index];

		video_destination_file = fopen(video_destination_filename, "wb");
		if (!video_destination_file) {
			fprintf(stderr, "Could not open destination file %s\n", video_destination_filename);
			return;	// TODO: HERE WE SHOULD THROW ERROR
		}

		/* allocate image where the decoded image will be put */
		width = video_decode_context->width;
		height = video_decode_context->height;
		pixel_format = video_decode_context->pix_fmt;

		int check = av_image_alloc(
			video_destination_data,
			video_destination_linesize,
			width,
			height,
			pixel_format,
			1
		);

		if (check < 0) {
			fprintf(stderr, "Could not allocate raw video buffer\n");
			return;
		}
		video_destination_bufsize = check;
	}

	if (open_codec_context(&audio_stream_index, &audio_decode_context, format_context, AVMEDIA_TYPE_AUDIO, fileName) >= 0) {
		audio_stream = format_context->streams[audio_stream_index];
		audio_destination_file = fopen(audio_destination_filename, "wb");
		if (!audio_destination_file) {
			fprintf(stderr, "Could not open destination file %s\n", audio_destination_filename);
			return;	// TODO: HERE WE SHOULD THROW ERROR
		}
	}


	/* dump input information to stderr */
	av_dump_format(format_context, 0, file_name, 0);
}

int VideoDecoder::open_codec_context(int * stream_idx, AVCodecContext ** dec_ctx, AVFormatContext * fmt_ctx, AVMediaType type, std::string fileName)
{
	int ret, stream_index;
	AVStream *st;
	AVCodec *dec = NULL;
	AVDictionary *opts = NULL;

	ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not find %s stream in input file '%s'\n",
			av_get_media_type_string(type), fileName);
		return ret;
	}
	else {
		stream_index = ret;
		st = fmt_ctx->streams[stream_index];

		/* find decoder for the stream */
		dec = avcodec_find_decoder(st->codecpar->codec_id);
		if (!dec) {
			fprintf(stderr, "Failed to find %s codec\n",
				av_get_media_type_string(type));
			return AVERROR(EINVAL);
		}

		/* Allocate a codec context for the decoder */
		*dec_ctx = avcodec_alloc_context3(dec);
		if (!*dec_ctx) {
			fprintf(stderr, "Failed to allocate the %s codec context\n",
				av_get_media_type_string(type));
			return AVERROR(ENOMEM);
		}

		/* Copy codec parameters from input stream to output codec context */
		if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
			fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
				av_get_media_type_string(type));
			return ret;
		}

		/* Init the decoders, with or without reference counting */
		/*av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
		if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
			fprintf(stderr, "Failed to open %s codec\n",
				av_get_media_type_string(type));
			return ret;
		}
		*stream_idx = stream_index;*/
	}

	return 0;
}