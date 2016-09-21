/**
 *
 * a puredata wrapper for aubio_pitchshift
 *
 * */

#include <m_pd.h>
#define AUBIO_UNSTABLE 1
#include <aubio/aubio.h>

char aubioshift_version[] = "aubioshift~ version " PACKAGE_VERSION;

static t_class *aubioshift_tilde_class;

void aubioshift_tilde_setup (void);

typedef struct _aubioshift_tilde
{
  t_object x_obj;
  t_float transpose;
  t_int pos; /*frames%dspblocksize*/
  t_int hopsize;
  aubio_pitchshift_t * pitchshift;
  fvec_t *input;
  fvec_t *output;
} t_aubioshift_tilde;

static t_int *aubioshift_tilde_perform(t_int *w)
{
  t_aubioshift_tilde *x = (t_aubioshift_tilde *)(w[1]);
  t_sample *in  = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n         = (int)(w[4]);
  int j;
  smpl_t transpose = (x->transpose < -23.9999) ? -23.9999 : (x->transpose > 24.) ? 24.
    : x->transpose;
  for (j=0;j<n;j++) {
    /* read input */
    fvec_set_sample(x->input, in[j], x->pos);
    if (x->pos == x->hopsize-1) {
      /* block loop */
      aubio_pitchshift_set_transpose(x->pitchshift, transpose);
      aubio_pitchshift_do(x->pitchshift, x->input, x->output);
      /* end of block loop */
      x->pos = -1; /* so it will be zero next j loop */
    }
    x->pos++;
    /* write output */
    *out++ = x->output->data[x->pos];
  }
  return (w+5);
}

static void aubioshift_tilde_dsp(t_aubioshift_tilde *x, t_signal **sp)
{
  dsp_add(aubioshift_tilde_perform, 4, x,
      sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void aubioshift_tilde_debug(t_aubioshift_tilde *x)
{
  uint_t latency = aubio_pitchshift_get_latency(x->pitchshift);
  post("aubioshift~ hopsize: %d latency: %.2fms (%d), transposition: %.4f",
      x->hopsize, 1000.*latency/(float)sys_getsr(), latency, x->transpose);
}

static void *aubioshift_tilde_new (t_floatarg f, t_symbol *s)
{
  t_aubioshift_tilde *x =
    (t_aubioshift_tilde *)pd_new(aubioshift_tilde_class);

  if (strcmp(s->s_name, "") == 0) s->s_name = "default";

  x->transpose = (f < -23.9999) ? -23.9999 : (f > 24.) ? 24. : f;
  x->hopsize = sys_getblksize();

  x->input = (fvec_t *)new_fvec(x->hopsize);
  x->output = (fvec_t *)new_fvec(x->hopsize);

  x->pitchshift = (aubio_pitchshift_t *)new_aubio_pitchshift(s->s_name,
      x->transpose, x->hopsize, (uint_t)sys_getsr());
  if (x->pitchshift == NULL) return NULL;
  aubio_pitchshift_set_transpose(x->pitchshift, x->transpose);

  floatinlet_new (&x->x_obj, &x->transpose);
  outlet_new(&x->x_obj, gensym("signal"));
  return (void *)x;
}

void aubioshift_tilde_setup (void)
{
  aubioshift_tilde_class = class_new (gensym ("aubioshift~"),
      (t_newmethod)aubioshift_tilde_new,
      0, sizeof (t_aubioshift_tilde),
      CLASS_DEFAULT, A_DEFFLOAT, A_DEFSYMBOL, 0);
  class_addmethod(aubioshift_tilde_class,
      (t_method)aubioshift_tilde_dsp,
      gensym("dsp"), 0);
  class_addmethod(aubioshift_tilde_class,
      (t_method)aubioshift_tilde_debug,
            gensym("debug"), 0);
  CLASS_MAINSIGNALIN(aubioshift_tilde_class,
      t_aubioshift_tilde, transpose);
}
