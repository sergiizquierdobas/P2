#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "vad.h"
#include "pav_analysis.h"


const float FRAME_TIME = 10.0F; /* in ms. */

/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "MS", "MV", "INIT"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;

/* 
 * TODO: Delete and use your own features!
 */


Features compute_features(const float *x, int N) {
  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */
  /* 
   * DELETE and include a call to your own functions
   *
   * For the moment, compute random value between 0 and 1 
   */
  Features feat;
  feat.zcr = compute_zcr(x, N, 16000);
  feat.am = compute_am(x,N);
  feat.p= compute_power(x,N);

  return feat;
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate, int number_init) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  vad_data->k0 = 6;
  vad_data->alpha=6;
  vad_data->counter_N=number_init;
  vad_data->counterinit=0;
  vad_data->p0=0;
  vad_data->maybesilencecounter=0;
  vad_data->maybevoicecounter=0;
  //vad_data->silencecounter=0;
  //vad_data->voicecounter=0;

  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */
  
  //Venimos de voice, asi que nos quedamos en voice
  if (vad_data->state==ST_MS){
    vad_data->state=ST_VOICE;
  }

   //Venimos de silence, asi que nos quedamos en silence
  if (vad_data->state==ST_MV){
    vad_data->state=ST_SILENCE;
  }
  
  VAD_STATE state = vad_data->state;

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * TODO: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x) {

  /* 
   * TODO: You can change this, using your own features,
   * program finite state automaton, define conditions, etc.
   */

  Features f = compute_features(x, vad_data->frame_length);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state) {
  case ST_INIT:
    if(vad_data->counterinit<vad_data->counter_N){
      vad_data->counterinit++;
      vad_data->p0 = vad_data->p0 + pow(10, f.p/10);

    } else{

      vad_data->k0=10*log10(vad_data->p0/vad_data->counter_N);
      // vad_data->k0=vad_data->p0 + vad_data->alpha;
      vad_data->margen1=-0.065*vad_data->k0+2.25;
      vad_data->k1= vad_data->k0+vad_data->margen1;
      vad_data->histeresis=-0.035*vad_data->margen1+1.8;
      vad_data->k2 = vad_data->histeresis + vad_data->k1; 
      vad_data->state= ST_SILENCE;
    
    }
   
    break;

  case ST_SILENCE:
    vad_data->maybesilencecounter=0;
    vad_data->maybevoicecounter=0;
    if (f.p > vad_data->k1)
      vad_data->state = ST_MV;
    break;

  case ST_VOICE:
    vad_data->maybesilencecounter=0;
    vad_data->maybevoicecounter=0;
    if (f.p < vad_data->k1)
      vad_data->state = ST_MS;
    break;

  case ST_MS:
    if(vad_data->maybesilencecounter < 3){
        vad_data->maybesilencecounter++;
    } else {
      if (f.p < vad_data->k1){
        vad_data->state =ST_SILENCE;
      } else {
      vad_data->state =ST_VOICE;
      }
    }
    break;

  case ST_MV:
    if(vad_data->maybevoicecounter < 3){
        vad_data->maybevoicecounter++;
    } else{
      if (f.p > vad_data->k2 ){
        vad_data->state =ST_VOICE;
      } else {
        vad_data->state =ST_SILENCE;
      }
    }
    break;

  case ST_UNDEF:
    break;
  }
    
  if (vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE|| vad_data->state == ST_MV || vad_data->state == ST_MS){
    return vad_data->state;  
  } else {
     return ST_UNDEF;
  }
    
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}