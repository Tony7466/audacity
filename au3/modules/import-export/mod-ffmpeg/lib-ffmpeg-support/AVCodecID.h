/**********************************************************************

  Audacity: A Digital Audio Editor

  AVCodecID.h

  Dmitry Vedenko

**********************************************************************/

#pragma once

enum AudacityAVCodecIDValue {
    AUDACITY_AV_CODEC_ID_NONE,
    AUDACITY_AV_CODEC_ID_MPEG1VIDEO,
    AUDACITY_AV_CODEC_ID_MPEG2VIDEO,
    AUDACITY_AV_CODEC_ID_MPEG2VIDEO_XVMC,
    AUDACITY_AV_CODEC_ID_H261,
    AUDACITY_AV_CODEC_ID_H263,
    AUDACITY_AV_CODEC_ID_RV10,
    AUDACITY_AV_CODEC_ID_RV20,
    AUDACITY_AV_CODEC_ID_MJPEG,
    AUDACITY_AV_CODEC_ID_MJPEGB,
    AUDACITY_AV_CODEC_ID_LJPEG,
    AUDACITY_AV_CODEC_ID_SP5X,
    AUDACITY_AV_CODEC_ID_JPEGLS,
    AUDACITY_AV_CODEC_ID_MPEG4,
    AUDACITY_AV_CODEC_ID_RAWVIDEO,
    AUDACITY_AV_CODEC_ID_MSMPEG4V1,
    AUDACITY_AV_CODEC_ID_MSMPEG4V2,
    AUDACITY_AV_CODEC_ID_MSMPEG4V3,
    AUDACITY_AV_CODEC_ID_WMV1,
    AUDACITY_AV_CODEC_ID_WMV2,
    AUDACITY_AV_CODEC_ID_H263P,
    AUDACITY_AV_CODEC_ID_H263I,
    AUDACITY_AV_CODEC_ID_FLV1,
    AUDACITY_AV_CODEC_ID_SVQ1,
    AUDACITY_AV_CODEC_ID_SVQ3,
    AUDACITY_AV_CODEC_ID_DVVIDEO,
    AUDACITY_AV_CODEC_ID_HUFFYUV,
    AUDACITY_AV_CODEC_ID_CYUV,
    AUDACITY_AV_CODEC_ID_H264,
    AUDACITY_AV_CODEC_ID_INDEO3,
    AUDACITY_AV_CODEC_ID_VP3,
    AUDACITY_AV_CODEC_ID_THEORA,
    AUDACITY_AV_CODEC_ID_ASV1,
    AUDACITY_AV_CODEC_ID_ASV2,
    AUDACITY_AV_CODEC_ID_FFV1,
    AUDACITY_AV_CODEC_ID_4XM,
    AUDACITY_AV_CODEC_ID_VCR1,
    AUDACITY_AV_CODEC_ID_CLJR,
    AUDACITY_AV_CODEC_ID_MDEC,
    AUDACITY_AV_CODEC_ID_ROQ,
    AUDACITY_AV_CODEC_ID_INTERPLAY_VIDEO,
    AUDACITY_AV_CODEC_ID_XAN_WC3,
    AUDACITY_AV_CODEC_ID_XAN_WC4,
    AUDACITY_AV_CODEC_ID_RPZA,
    AUDACITY_AV_CODEC_ID_CINEPAK,
    AUDACITY_AV_CODEC_ID_WS_VQA,
    AUDACITY_AV_CODEC_ID_MSRLE,
    AUDACITY_AV_CODEC_ID_MSVIDEO1,
    AUDACITY_AV_CODEC_ID_IDCIN,
    AUDACITY_AV_CODEC_ID_8BPS,
    AUDACITY_AV_CODEC_ID_SMC,
    AUDACITY_AV_CODEC_ID_FLIC,
    AUDACITY_AV_CODEC_ID_TRUEMOTION1,
    AUDACITY_AV_CODEC_ID_VMDVIDEO,
    AUDACITY_AV_CODEC_ID_MSZH,
    AUDACITY_AV_CODEC_ID_ZLIB,
    AUDACITY_AV_CODEC_ID_QTRLE,
    AUDACITY_AV_CODEC_ID_TSCC,
    AUDACITY_AV_CODEC_ID_ULTI,
    AUDACITY_AV_CODEC_ID_QDRAW,
    AUDACITY_AV_CODEC_ID_VIXL,
    AUDACITY_AV_CODEC_ID_QPEG,
    AUDACITY_AV_CODEC_ID_PNG,
    AUDACITY_AV_CODEC_ID_PPM,
    AUDACITY_AV_CODEC_ID_PBM,
    AUDACITY_AV_CODEC_ID_PGM,
    AUDACITY_AV_CODEC_ID_PGMYUV,
    AUDACITY_AV_CODEC_ID_PAM,
    AUDACITY_AV_CODEC_ID_FFVHUFF,
    AUDACITY_AV_CODEC_ID_RV30,
    AUDACITY_AV_CODEC_ID_RV40,
    AUDACITY_AV_CODEC_ID_VC1,
    AUDACITY_AV_CODEC_ID_WMV3,
    AUDACITY_AV_CODEC_ID_LOCO,
    AUDACITY_AV_CODEC_ID_WNV1,
    AUDACITY_AV_CODEC_ID_AASC,
    AUDACITY_AV_CODEC_ID_INDEO2,
    AUDACITY_AV_CODEC_ID_FRAPS,
    AUDACITY_AV_CODEC_ID_TRUEMOTION2,
    AUDACITY_AV_CODEC_ID_BMP,
    AUDACITY_AV_CODEC_ID_CSCD,
    AUDACITY_AV_CODEC_ID_MMVIDEO,
    AUDACITY_AV_CODEC_ID_ZMBV,
    AUDACITY_AV_CODEC_ID_AVS,
    AUDACITY_AV_CODEC_ID_SMACKVIDEO,
    AUDACITY_AV_CODEC_ID_NUV,
    AUDACITY_AV_CODEC_ID_KMVC,
    AUDACITY_AV_CODEC_ID_FLASHSV,
    AUDACITY_AV_CODEC_ID_CAVS,
    AUDACITY_AV_CODEC_ID_JPEG2000,
    AUDACITY_AV_CODEC_ID_VMNC,
    AUDACITY_AV_CODEC_ID_VP5,
    AUDACITY_AV_CODEC_ID_VP6,
    AUDACITY_AV_CODEC_ID_VP6F,
    AUDACITY_AV_CODEC_ID_TARGA,
    AUDACITY_AV_CODEC_ID_DSICINVIDEO,
    AUDACITY_AV_CODEC_ID_TIERTEXSEQVIDEO,
    AUDACITY_AV_CODEC_ID_TIFF,
    AUDACITY_AV_CODEC_ID_GIF,
    AUDACITY_AV_CODEC_ID_DXA,
    AUDACITY_AV_CODEC_ID_DNXHD,
    AUDACITY_AV_CODEC_ID_THP,
    AUDACITY_AV_CODEC_ID_SGI,
    AUDACITY_AV_CODEC_ID_C93,
    AUDACITY_AV_CODEC_ID_BETHSOFTVID,
    AUDACITY_AV_CODEC_ID_PTX,
    AUDACITY_AV_CODEC_ID_TXD,
    AUDACITY_AV_CODEC_ID_VP6A,
    AUDACITY_AV_CODEC_ID_AMV,
    AUDACITY_AV_CODEC_ID_VB,
    AUDACITY_AV_CODEC_ID_PCX,
    AUDACITY_AV_CODEC_ID_SUNRAST,
    AUDACITY_AV_CODEC_ID_INDEO4,
    AUDACITY_AV_CODEC_ID_INDEO5,
    AUDACITY_AV_CODEC_ID_MIMIC,
    AUDACITY_AV_CODEC_ID_RL2,
    AUDACITY_AV_CODEC_ID_ESCAPE124,
    AUDACITY_AV_CODEC_ID_DIRAC,
    AUDACITY_AV_CODEC_ID_BFI,
    AUDACITY_AV_CODEC_ID_CMV,
    AUDACITY_AV_CODEC_ID_MOTIONPIXELS,
    AUDACITY_AV_CODEC_ID_TGV,
    AUDACITY_AV_CODEC_ID_TGQ,
    AUDACITY_AV_CODEC_ID_TQI,
    AUDACITY_AV_CODEC_ID_AURA,
    AUDACITY_AV_CODEC_ID_AURA2,
    AUDACITY_AV_CODEC_ID_V210X,
    AUDACITY_AV_CODEC_ID_TMV,
    AUDACITY_AV_CODEC_ID_V210,
    AUDACITY_AV_CODEC_ID_DPX,
    AUDACITY_AV_CODEC_ID_MAD,
    AUDACITY_AV_CODEC_ID_FRWU,
    AUDACITY_AV_CODEC_ID_FLASHSV2,
    AUDACITY_AV_CODEC_ID_CDGRAPHICS,
    AUDACITY_AV_CODEC_ID_R210,
    AUDACITY_AV_CODEC_ID_ANM,
    AUDACITY_AV_CODEC_ID_BINKVIDEO,
    AUDACITY_AV_CODEC_ID_IFF_ILBM,
    AUDACITY_AV_CODEC_ID_IFF_BYTERUN1,
    AUDACITY_AV_CODEC_ID_KGV1,
    AUDACITY_AV_CODEC_ID_YOP,
    AUDACITY_AV_CODEC_ID_VP8,
    AUDACITY_AV_CODEC_ID_PICTOR,
    AUDACITY_AV_CODEC_ID_ANSI,
    AUDACITY_AV_CODEC_ID_A64_MULTI,
    AUDACITY_AV_CODEC_ID_A64_MULTI5,
    AUDACITY_AV_CODEC_ID_R10K,
    AUDACITY_AV_CODEC_ID_MXPEG,
    AUDACITY_AV_CODEC_ID_LAGARITH,
    AUDACITY_AV_CODEC_ID_PRORES,
    AUDACITY_AV_CODEC_ID_JV,
    AUDACITY_AV_CODEC_ID_DFA,
    AUDACITY_AV_CODEC_ID_WMV3IMAGE,
    AUDACITY_AV_CODEC_ID_VC1IMAGE,
    AUDACITY_AV_CODEC_ID_UTVIDEO,
    AUDACITY_AV_CODEC_ID_BMV_VIDEO,
    AUDACITY_AV_CODEC_ID_VBLE,
    AUDACITY_AV_CODEC_ID_DXTORY,
    AUDACITY_AV_CODEC_ID_V410,
    AUDACITY_AV_CODEC_ID_XWD,
    AUDACITY_AV_CODEC_ID_CDXL,
    AUDACITY_AV_CODEC_ID_XBM,
    AUDACITY_AV_CODEC_ID_ZEROCODEC,
    AUDACITY_AV_CODEC_ID_MSS1,
    AUDACITY_AV_CODEC_ID_MSA1,
    AUDACITY_AV_CODEC_ID_TSCC2,
    AUDACITY_AV_CODEC_ID_MTS2,
    AUDACITY_AV_CODEC_ID_CLLC,
    AUDACITY_AV_CODEC_ID_MSS2,
    AUDACITY_AV_CODEC_ID_VP9,
    AUDACITY_AV_CODEC_ID_AIC,
    AUDACITY_AV_CODEC_ID_ESCAPE130_DEPRECATED,
    AUDACITY_AV_CODEC_ID_G2M_DEPRECATED,
    AUDACITY_AV_CODEC_ID_WEBP_DEPRECATED,
    AUDACITY_AV_CODEC_ID_HNM4_VIDEO,
    AUDACITY_AV_CODEC_ID_HEVC_DEPRECATED,
    AUDACITY_AV_CODEC_ID_FIC,
    AUDACITY_AV_CODEC_ID_BRENDER_PIX,
    AUDACITY_AV_CODEC_ID_Y41P,
    AUDACITY_AV_CODEC_ID_ESCAPE130,
    AUDACITY_AV_CODEC_ID_EXR,
    AUDACITY_AV_CODEC_ID_AVRP,
    AUDACITY_AV_CODEC_ID_012V,
    AUDACITY_AV_CODEC_ID_G2M,
    AUDACITY_AV_CODEC_ID_AVUI,
    AUDACITY_AV_CODEC_ID_AYUV,
    AUDACITY_AV_CODEC_ID_TARGA_Y216,
    AUDACITY_AV_CODEC_ID_V308,
    AUDACITY_AV_CODEC_ID_V408,
    AUDACITY_AV_CODEC_ID_YUV4,
    AUDACITY_AV_CODEC_ID_SANM,
    AUDACITY_AV_CODEC_ID_PAF_VIDEO,
    AUDACITY_AV_CODEC_ID_AVRN,
    AUDACITY_AV_CODEC_ID_CPIA,
    AUDACITY_AV_CODEC_ID_XFACE,
    AUDACITY_AV_CODEC_ID_SGIRLE,
    AUDACITY_AV_CODEC_ID_MVC1,
    AUDACITY_AV_CODEC_ID_MVC2,
    AUDACITY_AV_CODEC_ID_SNOW,
    AUDACITY_AV_CODEC_ID_WEBP,
    AUDACITY_AV_CODEC_ID_SMVJPEG,
    AUDACITY_AV_CODEC_ID_HEVC,
    AUDACITY_AV_CODEC_ID_FIRST_AUDIO,
    AUDACITY_AV_CODEC_ID_PCM_S16LE,
    AUDACITY_AV_CODEC_ID_PCM_S16BE,
    AUDACITY_AV_CODEC_ID_PCM_U16LE,
    AUDACITY_AV_CODEC_ID_PCM_U16BE,
    AUDACITY_AV_CODEC_ID_PCM_S8,
    AUDACITY_AV_CODEC_ID_PCM_U8,
    AUDACITY_AV_CODEC_ID_PCM_MULAW,
    AUDACITY_AV_CODEC_ID_PCM_ALAW,
    AUDACITY_AV_CODEC_ID_PCM_S32LE,
    AUDACITY_AV_CODEC_ID_PCM_S32BE,
    AUDACITY_AV_CODEC_ID_PCM_U32LE,
    AUDACITY_AV_CODEC_ID_PCM_U32BE,
    AUDACITY_AV_CODEC_ID_PCM_S24LE,
    AUDACITY_AV_CODEC_ID_PCM_S24BE,
    AUDACITY_AV_CODEC_ID_PCM_U24LE,
    AUDACITY_AV_CODEC_ID_PCM_U24BE,
    AUDACITY_AV_CODEC_ID_PCM_S24DAUD,
    AUDACITY_AV_CODEC_ID_PCM_ZORK,
    AUDACITY_AV_CODEC_ID_PCM_S16LE_PLANAR,
    AUDACITY_AV_CODEC_ID_PCM_DVD,
    AUDACITY_AV_CODEC_ID_PCM_F32BE,
    AUDACITY_AV_CODEC_ID_PCM_F32LE,
    AUDACITY_AV_CODEC_ID_PCM_F64BE,
    AUDACITY_AV_CODEC_ID_PCM_F64LE,
    AUDACITY_AV_CODEC_ID_PCM_BLURAY,
    AUDACITY_AV_CODEC_ID_PCM_LXF,
    AUDACITY_AV_CODEC_ID_S302M,
    AUDACITY_AV_CODEC_ID_PCM_S8_PLANAR,
    AUDACITY_AV_CODEC_ID_PCM_S24LE_PLANAR_DEPRECATED,
    AUDACITY_AV_CODEC_ID_PCM_S32LE_PLANAR_DEPRECATED,
    AUDACITY_AV_CODEC_ID_PCM_S24LE_PLANAR,
    AUDACITY_AV_CODEC_ID_PCM_S32LE_PLANAR,
    AUDACITY_AV_CODEC_ID_PCM_S16BE_PLANAR,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_QT,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_WAV,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_DK3,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_DK4,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_WS,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_SMJPEG,
    AUDACITY_AV_CODEC_ID_ADPCM_MS,
    AUDACITY_AV_CODEC_ID_ADPCM_4XM,
    AUDACITY_AV_CODEC_ID_ADPCM_XA,
    AUDACITY_AV_CODEC_ID_ADPCM_ADX,
    AUDACITY_AV_CODEC_ID_ADPCM_EA,
    AUDACITY_AV_CODEC_ID_ADPCM_G726,
    AUDACITY_AV_CODEC_ID_ADPCM_CT,
    AUDACITY_AV_CODEC_ID_ADPCM_SWF,
    AUDACITY_AV_CODEC_ID_ADPCM_YAMAHA,
    AUDACITY_AV_CODEC_ID_ADPCM_SBPRO_4,
    AUDACITY_AV_CODEC_ID_ADPCM_SBPRO_3,
    AUDACITY_AV_CODEC_ID_ADPCM_SBPRO_2,
    AUDACITY_AV_CODEC_ID_ADPCM_THP,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_AMV,
    AUDACITY_AV_CODEC_ID_ADPCM_EA_R1,
    AUDACITY_AV_CODEC_ID_ADPCM_EA_R3,
    AUDACITY_AV_CODEC_ID_ADPCM_EA_R2,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_EA_SEAD,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_EA_EACS,
    AUDACITY_AV_CODEC_ID_ADPCM_EA_XAS,
    AUDACITY_AV_CODEC_ID_ADPCM_EA_MAXIS_XA,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_ISS,
    AUDACITY_AV_CODEC_ID_ADPCM_G722,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_APC,
    AUDACITY_AV_CODEC_ID_VIMA,
    AUDACITY_AV_CODEC_ID_ADPCM_AFC,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_OKI,
    AUDACITY_AV_CODEC_ID_ADPCM_DTK,
    AUDACITY_AV_CODEC_ID_ADPCM_IMA_RAD,
    AUDACITY_AV_CODEC_ID_ADPCM_G726LE,
    AUDACITY_AV_CODEC_ID_AMR_NB,
    AUDACITY_AV_CODEC_ID_AMR_WB,
    AUDACITY_AV_CODEC_ID_RA_144,
    AUDACITY_AV_CODEC_ID_RA_288,
    AUDACITY_AV_CODEC_ID_ROQ_DPCM,
    AUDACITY_AV_CODEC_ID_INTERPLAY_DPCM,
    AUDACITY_AV_CODEC_ID_XAN_DPCM,
    AUDACITY_AV_CODEC_ID_SOL_DPCM,
    AUDACITY_AV_CODEC_ID_MP2,
    AUDACITY_AV_CODEC_ID_MP3,
    AUDACITY_AV_CODEC_ID_AAC,
    AUDACITY_AV_CODEC_ID_AC3,
    AUDACITY_AV_CODEC_ID_DTS,
    AUDACITY_AV_CODEC_ID_VORBIS,
    AUDACITY_AV_CODEC_ID_DVAUDIO,
    AUDACITY_AV_CODEC_ID_WMAV1,
    AUDACITY_AV_CODEC_ID_WMAV2,
    AUDACITY_AV_CODEC_ID_MACE3,
    AUDACITY_AV_CODEC_ID_MACE6,
    AUDACITY_AV_CODEC_ID_VMDAUDIO,
    AUDACITY_AV_CODEC_ID_FLAC,
    AUDACITY_AV_CODEC_ID_MP3ADU,
    AUDACITY_AV_CODEC_ID_MP3ON4,
    AUDACITY_AV_CODEC_ID_SHORTEN,
    AUDACITY_AV_CODEC_ID_ALAC,
    AUDACITY_AV_CODEC_ID_WESTWOOD_SND1,
    AUDACITY_AV_CODEC_ID_GSM,
    AUDACITY_AV_CODEC_ID_QDM2,
    AUDACITY_AV_CODEC_ID_COOK,
    AUDACITY_AV_CODEC_ID_TRUESPEECH,
    AUDACITY_AV_CODEC_ID_TTA,
    AUDACITY_AV_CODEC_ID_SMACKAUDIO,
    AUDACITY_AV_CODEC_ID_QCELP,
    AUDACITY_AV_CODEC_ID_WAVPACK,
    AUDACITY_AV_CODEC_ID_DSICINAUDIO,
    AUDACITY_AV_CODEC_ID_IMC,
    AUDACITY_AV_CODEC_ID_MUSEPACK7,
    AUDACITY_AV_CODEC_ID_MLP,
    AUDACITY_AV_CODEC_ID_GSM_MS,
    AUDACITY_AV_CODEC_ID_ATRAC3,
    AUDACITY_AV_CODEC_ID_VOXWARE,
    AUDACITY_AV_CODEC_ID_APE,
    AUDACITY_AV_CODEC_ID_NELLYMOSER,
    AUDACITY_AV_CODEC_ID_MUSEPACK8,
    AUDACITY_AV_CODEC_ID_SPEEX,
    AUDACITY_AV_CODEC_ID_WMAVOICE,
    AUDACITY_AV_CODEC_ID_WMAPRO,
    AUDACITY_AV_CODEC_ID_WMALOSSLESS,
    AUDACITY_AV_CODEC_ID_ATRAC3P,
    AUDACITY_AV_CODEC_ID_EAC3,
    AUDACITY_AV_CODEC_ID_SIPR,
    AUDACITY_AV_CODEC_ID_MP1,
    AUDACITY_AV_CODEC_ID_TWINVQ,
    AUDACITY_AV_CODEC_ID_TRUEHD,
    AUDACITY_AV_CODEC_ID_MP4ALS,
    AUDACITY_AV_CODEC_ID_ATRAC1,
    AUDACITY_AV_CODEC_ID_BINKAUDIO_RDFT,
    AUDACITY_AV_CODEC_ID_BINKAUDIO_DCT,
    AUDACITY_AV_CODEC_ID_AAC_LATM,
    AUDACITY_AV_CODEC_ID_QDMC,
    AUDACITY_AV_CODEC_ID_CELT,
    AUDACITY_AV_CODEC_ID_G723_1,
    AUDACITY_AV_CODEC_ID_G729,
    AUDACITY_AV_CODEC_ID_8SVX_EXP,
    AUDACITY_AV_CODEC_ID_8SVX_FIB,
    AUDACITY_AV_CODEC_ID_BMV_AUDIO,
    AUDACITY_AV_CODEC_ID_RALF,
    AUDACITY_AV_CODEC_ID_IAC,
    AUDACITY_AV_CODEC_ID_ILBC,
    AUDACITY_AV_CODEC_ID_OPUS_DEPRECATED,
    AUDACITY_AV_CODEC_ID_COMFORT_NOISE,
    AUDACITY_AV_CODEC_ID_TAK_DEPRECATED,
    AUDACITY_AV_CODEC_ID_METASOUND,
    AUDACITY_AV_CODEC_ID_FFWAVESYNTH,
    AUDACITY_AV_CODEC_ID_SONIC,
    AUDACITY_AV_CODEC_ID_SONIC_LS,
    AUDACITY_AV_CODEC_ID_PAF_AUDIO,
    AUDACITY_AV_CODEC_ID_OPUS,
    AUDACITY_AV_CODEC_ID_TAK,
    AUDACITY_AV_CODEC_ID_EVRC,
    AUDACITY_AV_CODEC_ID_SMV,
    AUDACITY_AV_CODEC_ID_FIRST_SUBTITLE,
    AUDACITY_AV_CODEC_ID_DVD_SUBTITLE,
    AUDACITY_AV_CODEC_ID_DVB_SUBTITLE,
    AUDACITY_AV_CODEC_ID_TEXT,
    AUDACITY_AV_CODEC_ID_XSUB,
    AUDACITY_AV_CODEC_ID_SSA,
    AUDACITY_AV_CODEC_ID_MOV_TEXT,
    AUDACITY_AV_CODEC_ID_HDMV_PGS_SUBTITLE,
    AUDACITY_AV_CODEC_ID_DVB_TELETEXT,
    AUDACITY_AV_CODEC_ID_SRT,
    AUDACITY_AV_CODEC_ID_MICRODVD,
    AUDACITY_AV_CODEC_ID_EIA_608,
    AUDACITY_AV_CODEC_ID_JACOSUB,
    AUDACITY_AV_CODEC_ID_SAMI,
    AUDACITY_AV_CODEC_ID_REALTEXT,
    AUDACITY_AV_CODEC_ID_SUBVIEWER1,
    AUDACITY_AV_CODEC_ID_SUBVIEWER,
    AUDACITY_AV_CODEC_ID_SUBRIP,
    AUDACITY_AV_CODEC_ID_WEBVTT,
    AUDACITY_AV_CODEC_ID_MPL2,
    AUDACITY_AV_CODEC_ID_VPLAYER,
    AUDACITY_AV_CODEC_ID_PJS,
    AUDACITY_AV_CODEC_ID_ASS,
    AUDACITY_AV_CODEC_ID_FIRST_UNKNOWN,
    AUDACITY_AV_CODEC_ID_TTF,
    AUDACITY_AV_CODEC_ID_BINTEXT,
    AUDACITY_AV_CODEC_ID_XBIN,
    AUDACITY_AV_CODEC_ID_IDF,
    AUDACITY_AV_CODEC_ID_OTF,
    AUDACITY_AV_CODEC_ID_SMPTE_KLV,
    AUDACITY_AV_CODEC_ID_DVD_NAV,
    AUDACITY_AV_CODEC_ID_TIMED_ID3,
    AUDACITY_AV_CODEC_ID_PROBE,
    AUDACITY_AV_CODEC_ID_MPEG2TS,
    AUDACITY_AV_CODEC_ID_MPEG4SYSTEMS,
    AUDACITY_AV_CODEC_ID_FFMETADATA,

    AUDACITY_AV_CODEC_ID_LAST
};

using AVCodecIDFwd = int;

//! Define a wrapper struct so that implicit conversion to and from int won't
//! mistakenly happen
struct AudacityAVCodecID {
    AudacityAVCodecID(AVCodecIDFwd) = delete;
    AudacityAVCodecID(AudacityAVCodecIDValue value)
        : value{value} {}
    AudacityAVCodecIDValue value;
};

inline bool operator ==(AudacityAVCodecID x, AudacityAVCodecID y)
{ return x.value == y.value; }

inline bool operator !=(AudacityAVCodecID x, AudacityAVCodecID y)
{ return !(x == y); }
