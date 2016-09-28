/**
 *
 * a puredata wrapper for aubio onset detection functions
 *
 * Thanks to Johannes M Zmolnig for writing the excellent HOWTO:
 *       http://iem.kug.ac.at/pd/externals-HOWTO/
 *
 * */

#include <m_pd.h>
#include <aubio/aubio.h>

char aubioonset_version[] = "aubioonset~ version " PACKAGE_VERSION;

static t_class *aubioonset_tilde_class;

void aubioonset_tilde_setup (void);

typedef struct _aubioonset_tilde
{
  t_object x_obj;
  t_float threshold;
  char_t *method;
  t_int pos;                    /*frames%dspblocksize */
  t_int bufsize;
  t_int hopsize;
  aubio_onset_t *o;
  fvec_t *in;
  fvec_t *out;
  t_inlet *inlet;
  t_outlet *onsetbang;
} t_aubioonset_tilde;

static t_int *
aubioonset_tilde_perform (t_int * w)
{
  t_aubioonset_tilde *x = (t_aubioonset_tilde *) (w[1]);
  t_sample *in = (t_sample *) (w[2]);
  int n = (int) (w[3]);
  int j;
  for (j = 0; j < n; j++) {
    /* write input to datanew */
    fvec_set_sample (x->in, in[j], x->pos);
    /*time to do something */
    if (x->pos == x->hopsize - 1) {
      /* block loop */
      aubio_onset_do (x->o, x->in, x->out);
      if (fvec_get_sample (x->out, 0) > 0.) {
        outlet_bang (x->onsetbang);
      }
      /* end of block loop */
      x->pos = -1;              /* so it will be zero next j loop */
    }
    x->pos++;
  }
  return (w + 4);
}

static void
aubioonset_tilde_dsp (t_aubioonset_tilde * x, t_signal ** sp)
{
  dsp_add (aubioonset_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void
aubioonset_tilde_debug (t_aubioonset_tilde * x)
{
  post (aubioonset_version);
  post ("aubioonset~ method:\t%s", x->method);
  post ("aubioonset~ threshold:\t%f", x->threshold);
  post ("aubioonset~ bufsize:\t%d", x->bufsize);
  post ("aubioonset~ hopsize:\t%d", x->hopsize);
}

static void *
aubioonset_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
  t_aubioonset_tilde *x = (t_aubioonset_tilde *)pd_new(aubioonset_tilde_class);

  x->threshold = -1;
  x->bufsize = 1024;
  x->method = "default";

  if (argc >= 3) {
    if (argv[2].a_type == A_FLOAT) {
      x->bufsize = (uint_t)(argv[2].a_w.w_float);
    }
    argc--;
  }

  x->hopsize = x->bufsize / 2;

  if (argc == 3) {
    if (argv[3].a_type == A_FLOAT) {
      x->hopsize = (uint_t)(argv[3].a_w.w_float);
    }
    argc--;
  }

  // process 2 remaining arguments
  // (can be 'method threshold' or 'threshold method')
  while (argc > 0) {
    if (argv->a_type == A_FLOAT) {
      t_sample f = argv->a_w.w_float;
      x->threshold = (f < 1e-5) ? 0.1 : (f > 10.) ? 10. : f;
    } else if (argv->a_type == A_SYMBOL) {
      x->method = argv->a_w.w_symbol->s_name;
    }
    argc--;
    argv++;
  }

  x->o = new_aubio_onset (x->method,
      x->bufsize, x->hopsize, (uint_t) sys_getsr ());

  if (x->o == NULL) return NULL;

  if (x->threshold != -1) {
    aubio_onset_set_threshold(x->o, x->threshold);
  }
  x->threshold = aubio_onset_get_threshold(x->o);

  x->in = (fvec_t *) new_fvec (x->hopsize);
  x->out = (fvec_t *) new_fvec (1);

  x->inlet = floatinlet_new (&x->x_obj, &x->threshold);
  x->onsetbang = outlet_new (&x->x_obj, &s_bang);
  return (void *) x;
}

void
aubioonset_tilde_del (t_aubioonset_tilde *x)
{
  inlet_free(x->inlet);
  outlet_free(x->onsetbang);
  del_aubio_onset(x->o);
  del_fvec(x->in);
  del_fvec(x->out);
}

void
aubioonset_tilde_setup (void)
{
  aubioonset_tilde_class = class_new (gensym ("aubioonset~"),
      (t_newmethod) aubioonset_tilde_new,
      (t_method) aubioonset_tilde_del,
      sizeof (t_aubioonset_tilde), CLASS_DEFAULT, A_GIMME, 0);
  class_addmethod (aubioonset_tilde_class,
      (t_method) aubioonset_tilde_dsp, gensym ("dsp"), 0);
  class_addmethod (aubioonset_tilde_class,
      (t_method) aubioonset_tilde_debug, gensym ("debug"), 0);
  CLASS_MAINSIGNALIN (aubioonset_tilde_class, t_aubioonset_tilde, threshold);
}
