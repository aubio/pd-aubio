/*
  Copyright (C) 2016 Paul Brossier <piem@aubio.org>

  This file is part of pd-aubio.

  aubio is free software: you can redistribute it and/or modify
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

char aubiostretchsf_version[] = "aubiostretchsf~ version " PACKAGE_VERSION;

static t_class *aubiostretchsf_tilde_class;

void aubiostretchsf_tilde_setup (void);

typedef struct _aubiostretchsf_tilde
{
  t_object x_obj;
  aubio_timestretch_t *o;
  fvec_t *out;
  t_inlet *inlet_s;
  t_inlet *inlet_t;
  t_outlet *outlet;
  t_outlet *outlet_done;
  t_clock *clock_done;
  smpl_t stretch;
  smpl_t transpose;
  int playing;
  int ended;
} t_aubiostretchsf_tilde;

static t_int *
aubiostretchsf_tilde_perform (t_int * w)
{
  t_aubiostretchsf_tilde *x = (t_aubiostretchsf_tilde *)(w[1]);
  t_sample *out  = (t_sample *)(w[2]);
  int s_n = (int)w[3];
  int read = 0;
  if (x->playing) {
    aubio_timestretch_set_stretch(x->o, x->stretch);
    aubio_timestretch_set_transpose(x->o, x->transpose);
    aubio_timestretch_do(x->o, x->out, &read);
    if (read > 0) {
      memcpy(out, x->out->data, read * sizeof(smpl_t));
    }
    if (read > 0 && s_n > read) {
      //post("feel-in  %d with zeros", sys_getblksize() - read);
      memset(out + read, 0, (s_n - read) * sizeof(smpl_t));
      //post("sending clock");
      clock_delay(x->clock_done, 0);
      x->playing = 0;
      x->ended = 1;
    }
    if (read <= 0) {
      // occurs when the previous read was a short read of a complete buffer
      clock_delay(x->clock_done, 0);
      x->playing = 0;
      x->ended = 1;
    }
  } else {
    memset(out, 0, s_n * sizeof(smpl_t));
  }
  return (w + 4);
}

static void
aubiostretchsf_tilde_tick(t_aubiostretchsf_tilde * x)
{
  outlet_bang(x->outlet_done);
}

static void
aubiostretchsf_tilde_dsp (t_aubiostretchsf_tilde * x, t_signal ** sp)
{
  dsp_add (aubiostretchsf_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void *
aubiostretchsf_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
  t_aubiostretchsf_tilde *x = (t_aubiostretchsf_tilde *)pd_new(aubiostretchsf_tilde_class);
  x->out = new_fvec(sys_getblksize());
  if (x->out == NULL) return NULL;
  x->clock_done = clock_new(x, (t_method)aubiostretchsf_tilde_tick);
  x->transpose = 0.;
  x->stretch = 1.;
  x->playing = 0;
  x->ended = 0;
  x->inlet_s = floatinlet_new (&x->x_obj, &x->stretch);
  x->inlet_t = floatinlet_new (&x->x_obj, &x->transpose);
  x->outlet = outlet_new(&x->x_obj, gensym("signal"));
  x->outlet_done = outlet_new(&x->x_obj, &s_bang);
  return (void *)x;
}

static void *
aubiostretchsf_tilde_open (t_aubiostretchsf_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
  t_symbol *uri = atom_getsymbolarg(0, argc, argv);
  if (!*uri->s_name) {
    post("aubiostretchsf~: open: no filename given");
    return;
  }
  x->playing = 0;
  // TODO: add aubio_timestretch_open method
  if (x->o) del_aubio_timestretch(x->o);
  x->o = new_aubio_timestretch(uri->s_name, "default", 1., sys_getblksize(),
      sys_getsr());
}

void
aubiostretchsf_tilde_float(t_aubiostretchsf_tilde *x, t_floatarg f) {
  if (x->o == NULL) return;
  if (f == 2) {
    uint_t res = aubio_timestretch_seek(x->o, 0);
    if (res != 0)
      post("failed seeking");
    x->ended = 0;
    x->playing = 1;
  } else if (f == 1) {
    //x->stretch = f;
    if (x->ended) {
      uint_t res = aubio_timestretch_seek(x->o, 0);
      if (res != 0)
        post("failed seeking");
      x->ended = 0;
    }
    x->playing = 1;
  } else {
    x->playing = 0;
  }
}

void
aubiostretchsf_tilde_del (t_aubiostretchsf_tilde *x)
{
  del_fvec(x->out);
  if (x->o) del_aubio_timestretch(x->o);
  inlet_free(x->inlet_s);
  inlet_free(x->inlet_t);
  outlet_free(x->outlet);
  outlet_free(x->outlet_done);
  clock_free(x->clock_done);
}

void
aubiostretchsf_tilde_setup (void)
{
  aubiostretchsf_tilde_class = class_new (gensym ("aubiostretchsf~"),
      (t_newmethod) aubiostretchsf_tilde_new,
      (t_method) aubiostretchsf_tilde_del,
      sizeof (t_aubiostretchsf_tilde),
      CLASS_DEFAULT, A_GIMME, 0);
  class_addfloat (aubiostretchsf_tilde_class, (t_method)aubiostretchsf_tilde_float);
  class_addmethod (aubiostretchsf_tilde_class,
      (t_method)aubiostretchsf_tilde_open, gensym ("open"), A_GIMME, 0);
  class_addmethod (aubiostretchsf_tilde_class,
      (t_method)aubiostretchsf_tilde_dsp, gensym ("dsp"), A_CANT, 0);
}
