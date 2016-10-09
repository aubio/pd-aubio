/*
  Copyright (C) 2016 Paul Brossier <piem@aubio.org>

  This file is part of pd-aubio.

  pd-aubio is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  aubio is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with aubio.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <m_pd.h>
#define AUBIO_UNSTABLE 1
#include <aubio/aubio.h>
#include <string.h>

char aubiosampler_version[] = "aubiosampler~ version " PACKAGE_VERSION;

static t_class *aubiosampler_tilde_class;

void aubiosampler_tilde_setup (void);

typedef struct _aubiosampler_tilde
{
  t_object x_obj;
  aubio_sampler_t *o;
  fvec_t *out;
  t_inlet *inlet_s;
  t_inlet *inlet_t;
  t_outlet *outlet;
  t_outlet *outlet_done;
  t_clock *clock_done;
  smpl_t stretch;
  smpl_t transpose;
} t_aubiosampler_tilde;

static t_int *
aubiosampler_tilde_perform (t_int * w)
{
  t_aubiosampler_tilde *x = (t_aubiosampler_tilde *)(w[1]);
  t_sample *out  = (t_sample *)(w[2]);
  int s_n = (int)w[3];
  uint_t read = 0;

  // post a message if the sampler was started before a file was loaded
  uint_t waited = aubio_sampler_get_waited_opening(x->o, s_n);
  if (waited) {
    pd_error(x, "aubiosampler~: start while opening, waited %.2fms\t(%d samples)",
        1000. * waited / (float)sys_getsr(), waited);
  }

  fvec_t fakevector;
  fakevector.data = out;
  fakevector.length = s_n;

  aubio_sampler_do(x->o, &fakevector, &read);

  if (aubio_sampler_get_eof(x->o)) {
    clock_delay(x->clock_done, 0);
  }

  return (w + 4);
}

static void
aubiosampler_tilde_tick(t_aubiosampler_tilde * x)
{
  outlet_bang(x->outlet_done);
}

static void
aubiosampler_tilde_dsp (t_aubiosampler_tilde * x, t_signal ** sp)
{
  dsp_add (aubiosampler_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void *
aubiosampler_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
  t_aubiosampler_tilde *x = (t_aubiosampler_tilde *)pd_new(aubiosampler_tilde_class);
  x->out = new_fvec(sys_getblksize());
  if (x->out == NULL) return NULL;
  x->clock_done = clock_new(x, (t_method)aubiosampler_tilde_tick);
  x->transpose = 0.;
  x->stretch = 1.;
  x->o = new_aubio_sampler(sys_getblksize(), sys_getsr());
  if (!x->o) return NULL;
  x->inlet_s = floatinlet_new (&x->x_obj, &x->stretch);
  x->inlet_t = floatinlet_new (&x->x_obj, &x->transpose);
  x->outlet = outlet_new(&x->x_obj, gensym("signal"));
  x->outlet_done = outlet_new(&x->x_obj, &s_bang);
  return (void *)x;
}

static void *
aubiosampler_tilde_open (t_aubiosampler_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
  t_symbol *uri = atom_getsymbolarg(0, argc, argv);
  if (!*uri->s_name) {
    post("aubiosampler~: open: no filename given");
    return;
  }
  aubio_sampler_stop(x->o);
  if (aubio_sampler_queue(x->o, uri->s_name)) {
    error("aubiosampler~: failed queuing %s", uri->s_name);
  }
}

void
aubiosampler_tilde_float(t_aubiosampler_tilde *x, t_floatarg f) {
  if (f == 3) {
    aubio_sampler_set_loop(x->o, !aubio_sampler_get_loop(x->o));
  } else if (f == 2) {
    aubio_sampler_trigger(x->o);
  } else if (f == 1) {
    // rewind to start of if we reached the end
    if (aubio_sampler_get_finished(x->o) && aubio_sampler_seek(x->o, 0)) {
      post("aubiosampler~: failed seeking");
    }
    aubio_sampler_play(x->o);
  } else {
    aubio_sampler_stop(x->o);
  }
}

void
aubiosampler_tilde_del (t_aubiosampler_tilde *x)
{
  del_fvec(x->out);
  if (x->o) del_aubio_sampler(x->o);
  inlet_free(x->inlet_s);
  inlet_free(x->inlet_t);
  outlet_free(x->outlet);
  outlet_free(x->outlet_done);
  clock_free(x->clock_done);
}

void
aubiosampler_tilde_setup (void)
{
  aubiosampler_tilde_class = class_new (gensym ("aubiosampler~"),
      (t_newmethod) aubiosampler_tilde_new,
      (t_method) aubiosampler_tilde_del,
      sizeof (t_aubiosampler_tilde),
      CLASS_DEFAULT, A_GIMME, 0);
  class_addfloat (aubiosampler_tilde_class, (t_method)aubiosampler_tilde_float);
  class_addmethod (aubiosampler_tilde_class,
      (t_method)aubiosampler_tilde_open, gensym ("open"), A_GIMME, 0);
  class_addmethod (aubiosampler_tilde_class,
      (t_method)aubiosampler_tilde_dsp, gensym ("dsp"), A_CANT, 0);
}
