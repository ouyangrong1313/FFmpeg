
#if defined(TEMPLATE_DITHER_DBL)
#    define RENAME(N) N ## _double
#    define DELEM  double
#    define CLIP(v)

#elif defined(TEMPLATE_DITHER_FLT)
#    define RENAME(N) N ## _float
#    define DELEM  float
#    define CLIP(v)

#elif defined(TEMPLATE_DITHER_S32)
#    define RENAME(N) N ## _int32
#    define DELEM  int32_t
#    define CLIP(v) v = FFMAX(FFMIN(v, INT32_MAX), INT32_MIN)

#elif defined(TEMPLATE_DITHER_S16)
#    define RENAME(N) N ## _int16
#    define DELEM  int16_t
#    define CLIP(v) v = FFMAX(FFMIN(v, INT16_MAX), INT16_MIN)

#else
ERROR
#endif

void RENAME(swri_noise_shaping)(SwrContext *s, AudioData *srcs, AudioData *noises, int count){
    int i, j, pos, ch;
    int taps  = s->dither.ns_taps;
    float S   = s->dither.ns_scale;
    float S_1 = s->dither.ns_scale_1;

    for (ch=0; ch<srcs->ch_count; ch++) {
        const float *noise = ((const float *)noises->ch[ch]) + s->dither.noise_pos;
        DELEM *data = (DELEM*)srcs->ch[ch];
        pos  = s->dither.ns_pos;
        for (i=0; i<count; i++) {
            double d1, d = data[i]*S_1;
            for(j=0; j<taps; j++)
                d -= s->dither.ns_coeffs[j] * s->dither.ns_errors[ch][pos + j];
            pos = pos ? pos - 1 : pos - 1 + taps;
            d1 = rint(d + noise[i]);
            s->dither.ns_errors[ch][pos + taps] = s->dither.ns_errors[ch][pos] = d1 - d;
            d1 *= S;
            CLIP(d1);
            data[i] = d1;
        }
    }

    s->dither.ns_pos = pos;
}

#undef RENAME
#undef DELEM
#undef CLIP
