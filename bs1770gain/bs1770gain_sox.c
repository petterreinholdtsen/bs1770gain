/*
 * bs1770gain_sox.c
 * Copyright (C) 2014 Peter Belkner <pbelkner@snafu.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */
#include <bs1770gain.h>

///////////////////////////////////////////////////////////////////////////////
#define BS1770GAIN_SAMPLE_RATE 192000

///////////////////////////////////////////////////////////////////////////////
// add the SoX FFmpeg input effect to the chain.
static int bs1770gain_sox_input(sox_effects_chain_t *chain,
    sox_signalinfo_t *signal, bs1770gain_read_t *read)
{
  int code=-1;
  sox_effect_t *effect;

  effect=sox_create_effect(bs1770gain_read_handler());
  BS1770GAIN_GOTO(NULL==effect,"creating input",effect);
  BS1770GAIN_GOTO(SOX_SUCCESS!=sox_effect_options(effect,1,(char **)&read),
      "adding arguments to the input",argv);
  BS1770GAIN_GOTO(SOX_SUCCESS!=sox_add_effect(chain,effect,signal,signal),
      "adding input",argv);
  code=0;
argv:
  free(effect);
effect:
  return code;
}

// add the rate effect to the chain.
#if 0 // {
static int bs1770gain_sox_rate(sox_effects_chain_t *chain,
    sox_signalinfo_t *signal)
{
  int code=-1;
  sox_effect_t *effect;
  char arg1[]="-m";
  char arg2[32];
  char *argv[]={ arg1,arg2 };

  effect=sox_create_effect(sox_find_effect("rate"));
  BS1770GAIN_GOTO(NULL==effect,"creating rate",effect);
  sprintf(arg2,"%d",BS1770GAIN_SAMPLE_RATE);
  BS1770GAIN_GOTO(SOX_SUCCESS!=sox_effect_options(effect,2,argv),
      "adding arguments to the output",argv);
  BS1770GAIN_GOTO(SOX_SUCCESS!=sox_add_effect(chain,effect,signal,signal),
      "adding output",argv);
  code=0;
argv:
  free(effect);
effect:
  return code;
}
#else // } {
static int bs1770gain_sox_rate(sox_effects_chain_t *chain,
    sox_signalinfo_t *signal)
{
  int code=-1;
  sox_rate_t rate;
  sox_effect_t *effect;
  char arg1[]="-m";
  char arg2[32];
  char *argv[]={ arg1,arg2 };

  rate=signal->rate;

  while ((rate*=2)<BS1770GAIN_SAMPLE_RATE) {
    //rate*=2;
    effect=sox_create_effect(sox_find_effect("rate"));
    BS1770GAIN_GOTO(NULL==effect,"creating rate",effect);
    sprintf(arg2,"%.0f",rate);
    BS1770GAIN_GOTO(SOX_SUCCESS!=sox_effect_options(effect,2,argv),
        "adding arguments to the output",argv);
    BS1770GAIN_GOTO(SOX_SUCCESS!=sox_add_effect(chain,effect,signal,signal),
        "adding output",argv);
    free(effect);
    effect=NULL;
  }

  code=0;
argv:
  if (NULL!=effect)
    free(effect);
effect:
  return code;
}
#endif // }

// add the output effect to the chain.
static int bs1770gain_sox_output(sox_effects_chain_t *chain,
    sox_signalinfo_t *signal, bs1770gain_read_t *read)
{
  int code=-1;
  sox_effect_t *effect;

  effect=sox_create_effect(bs1770gain_sink_handler());
  BS1770GAIN_GOTO(NULL==effect,"creating output",effect);
  BS1770GAIN_GOTO(SOX_SUCCESS!=sox_effect_options(effect,1,(char **)&read),
      "adding arguments to the output",argv);
  BS1770GAIN_GOTO(SOX_SUCCESS!=sox_add_effect(chain,effect,signal,signal),
      "adding output",argv);
  code=0;
argv:
  free(effect);
effect:
  return code;
}

int bs1770gain_sox(const bs1770gain_options_t *options, const char *path,
    bs1770gain_stats_t *stats)
{
  int code=-1;
  bs1770gain_read_t *read;
  sox_effects_chain_t *chain;
  sox_signalinfo_t signal;

  // open a SoX FFmpeg input.
  read=bs1770gain_read_new(options,path,stats);
  BS1770GAIN_GOTO(NULL==read,"creating SoX FFmpeg input",in);
  signal=read->signal;

  // create a SoX effects chain.
  chain=sox_create_effects_chain(NULL,NULL);
  BS1770GAIN_GOTO(NULL==chain,"creating sox chain",chain);

  // add effects.
  BS1770GAIN_GOTO(bs1770gain_sox_input(chain,&signal,read)<0,
      "adding input",effect);

  if (options->truepeak&&signal.rate<BS1770GAIN_SAMPLE_RATE)
    BS1770GAIN_GOTO(bs1770gain_sox_rate(chain,&signal)<0,
        "adding rate",effect);

  BS1770GAIN_GOTO(bs1770gain_sox_output(chain,&signal,read)<0,
      "adding output",effect);

  // flow the effects.
  sox_flow_effects(chain,NULL,NULL);
  code=0;
effect:
  // destroy the SoX effects chain.
  sox_delete_effects_chain(chain);
chain:
  // destroy the SoX FFmpeg input.
  bs1770gain_read_close(read);
in:
  return code;
}
