
#include <m_pd.h>
#include <aubio/aubio.h>
#include <string.h>

char aubio_version[] = "aubio external for pd, version " PACKAGE_VERSION;

static t_class *aubio_class;

typedef struct aubio
{
  t_object x_ob;
} t_aubio;

void *aubio_new (void);
void aubio_setup (void);
extern void aubioonset_tilde_setup (void);
extern void aubiotempo_tilde_setup (void);
extern void aubiotss_tilde_setup (void);
extern void aubioquiet_tilde_setup (void);
extern void aubiopitch_tilde_setup (void);
extern void aubiozcr_tilde_setup (void);

void *aubio_new (void)
{
  t_aubio *x = (t_aubio *)pd_new(aubio_class);
  return (void *)x;
}

void aubio_custom_log(int level, const char *message, void *data)
{
  // remove the last character, removing the trailing space
  char *pos;
  if ((pos=strchr(message, '\n')) != NULL) {
        *pos = '\0';
  }
  post(message);
}

void aubio_setup (void)
{
  post(aubio_version);
  // register custom log function for errors and warnings
  aubio_log_set_level_function(AUBIO_LOG_ERR, aubio_custom_log, NULL);
  aubio_log_set_level_function(AUBIO_LOG_WRN, aubio_custom_log, NULL);
  aubioonset_tilde_setup();
  aubiotempo_tilde_setup();
  aubiotss_tilde_setup();
  aubioquiet_tilde_setup();
  aubiopitch_tilde_setup();
  aubiozcr_tilde_setup();
  aubio_class = class_new(gensym("aubio"), (t_newmethod)aubio_new, 0,
      sizeof(t_aubio), 0, 0);
}
