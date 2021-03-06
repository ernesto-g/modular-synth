/*
 *  Sound Source. Macro-oscillator using Blue Pill. Based on Mutable Instruments Braids.
    Copyright (C) 2020  Ernesto Gigliotti. ernestogigliotti@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "MainLoop.h"
#include "hal/MultiEngineHAL.h"
#include "hal/Memory.h"

#include "braids/macro_oscillator.h"
#include "braids/envelope.h"
#include "braids/drivers/adc.h"
#include "braids/quantizer.h"
#include "braids/signature_waveshaper.h"
#include "stmlib/system/uid.h"
#include "braids/quantizer_scales.h"
#include "braids/vco_jitter_source.h"


#include "Ui.h"
#include "ChordManager.h"



#define SAMPLES_BUFFER_SIZE		48
static uint8_t samplesBuffer[SAMPLES_BUFFER_SIZE];
static volatile uint8_t flagRender;
static volatile uint8_t indexBufferRender;

using namespace braids;

static uint8_t sync_samples[SAMPLES_BUFFER_SIZE/2];
static MacroOscillator osc;
static Envelope envelope;
static Adc adc;
static volatile bool trigger_flag;
static Quantizer quantizer;
//static SignatureWaveshaper ws;
static uint8_t current_scale = 0xff;
static Ui userInterface;
static uint8_t sysTickDivisor=0;
static Memory memory;
//static VcoJitterSource jitter_source;  // Not implemented. speed is not good enough
static ChordManager chordManager;

const uint16_t bit_reduction_masks[] = {
    0xc000,
    0xe000,
    0xf000,
    0xf800,
    0xff00,
    0xfff0,
    0xffff };
const uint16_t decimation_factors[] = { 24, 12, 6, 4, 3, 2, 1 };


static uint32_t flagTriggerInEvent=0;




static void callbackRender(int flagEndHalf)
{
	flagRender = 1;
	if(flagEndHalf==1)
	{
		// there is end of transfer, so I can start writing the second half of the buffer
		indexBufferRender=1;
	}
	else
	{
		// transfer reach half, so I can start writing the first half
		indexBufferRender=0;
	}
}


void MainLoop::init(void)
{
	int i;
	for(i=0; i<SAMPLES_BUFFER_SIZE; i++)
	{
		samplesBuffer[i] = 0;
	}
    memset(sync_samples, 0, SAMPLES_BUFFER_SIZE/2);

	mehal_init((uint32_t*)samplesBuffer,SAMPLES_BUFFER_SIZE,callbackRender);

	settings.Init();
	osc.Init();
	quantizer.Init();
	envelope.Init();
	//jitter_source.Init();
	adc.Init();
	userInterface.init(&adc,&memory);
	//ws.Init(GetUniqueId(1));
	chordManager.init();


	// calibration
	int32_t adc_code_c2=memory.readUInt32(Memory::ADDR_ADC_CODE_C2);
	if(adc_code_c2<0 || adc_code_c2>4095) adc_code_c2=82;

	int32_t adc_code_c4=memory.readUInt32(Memory::ADDR_ADC_CODE_C4);
	if(adc_code_c4<0 || adc_code_c4>4095) adc_code_c4=671;

	int32_t adc_code_fm=memory.readUInt32(Memory::ADDR_ADC_CODE_FM);
	if(adc_code_fm<0 || adc_code_fm>4095) adc_code_fm=0;

	int32_t adc_code_p0_min= memory.readUInt32(Memory::ADDR_ADC_CODE_P0_MIN);
	if(adc_code_p0_min<0 || adc_code_p0_min>4095) adc_code_p0_min=0;

	int32_t adc_code_p0_max= memory.readUInt32(Memory::ADDR_ADC_CODE_P0_MAX);
	if(adc_code_p0_max<0 || adc_code_p0_max>4095) adc_code_p0_max=4095;

	int32_t adc_code_p1_min= memory.readUInt32(Memory::ADDR_ADC_CODE_P1_MIN);
	if(adc_code_p1_min<0 || adc_code_p1_min>4095) adc_code_p1_min=0;

	int32_t adc_code_p1_max= memory.readUInt32(Memory::ADDR_ADC_CODE_P1_MAX);
	if(adc_code_p1_max<0 || adc_code_p1_max>4095) adc_code_p1_max=4095;

	settings.Calibrate(adc_code_c2,adc_code_c4,adc_code_fm,adc_code_p0_min,adc_code_p0_max,adc_code_p1_min,adc_code_p1_max);
	//________________________________

	settings.SetValue(SETTING_RESOLUTION,RESOLUTION_8_BIT);
	settings.SetValue(SETTING_PITCH_OCTAVE,1); // C2: Lowest octave

}



void MainLoop::loop(void)
{
	userInterface.loop();

	// Gate event
	/*
	static uint8_t prevGate=1;
	uint8_t currentGate = mehal_readGateInput();
	if(currentGate==0 && prevGate==1)
	{
		trigger_flag=true;
		flagTriggerInEvent=1;
	}
	prevGate = currentGate;
	*/
	//____________

	// Scale manager
    if (current_scale != settings.GetValue(SETTING_QUANTIZER_SCALE)) {
      current_scale = settings.GetValue(SETTING_QUANTIZER_SCALE);
      quantizer.Configure(scales[current_scale]);
    }
    //______________

    // Read Analog values
    this->adcStateMachine();
    //____________________

    // Render event
	if(flagRender==1)
	{
		uint8_t* out;
		if(indexBufferRender==0)
		{
			out = samplesBuffer;
		}
		else
		{
			out = samplesBuffer+(SAMPLES_BUFFER_SIZE/2);
		}

		// render signal
		mehal_debugPinSet();
		this->render(out,SAMPLES_BUFFER_SIZE/2);
		mehal_debugPinReset();
		//______________

		// update UI
		sysTickDivisor++;
		if(sysTickDivisor>=4)
		{
			userInterface.sysTick(); // 1ms
			sysTickDivisor=0;
		}
		//__________


		userInterface.justFinishedRender(); // notify the UI the render has finished. Now there is time to update the display

		flagRender=0;
	}
	//______________
}

void MainLoop::render(uint8_t* out, uint32_t outSize)
{
	  static int16_t previous_pitch = 0;
	  static uint16_t gain_lp;

	  // Envelope management
	  envelope.Update(
	      settings.GetValue(SETTING_AD_ATTACK) * 8,
	      settings.GetValue(SETTING_AD_DECAY) * 8);
	  uint32_t ad_value = envelope.Render();
	  //____________________

	  // Set shape (not used)
	  osc.set_shape(settings.shape());
	  // Set waveform with param0
	  osc.set_parameters(userInterface.getParamFromCurrentWaveTable(),0);


	  // SET PITCH
	  // Apply hysteresis to ADC reading to prevent a single bit error to move
	  // the quantized pitch up and down the quantization boundary.
	  int32_t pitch = quantizer.Process(
	      settings.adc_to_pitch(adc.channel(ADC_CHANNEL_PITCH)),
	      (60 + settings.quantizer_root()) << 7);
	  // Check if the pitch has changed to cause an auto-retrigger
	  int32_t pitch_delta = pitch - previous_pitch;
	  if (settings.data().auto_trig &&
	      //(pitch_delta >= 0x40 || -pitch_delta >= 0x40)) {
			(pitch_delta >= 0x70 || -pitch_delta >= 0x70)) { // too much noise in this hardware
		  trigger_flag = true;
	  }
	  previous_pitch = pitch;
	  if (pitch > 16383) {
	    pitch = 16383;
	  } else if (pitch < 0) {
	    pitch = 0;
	  }
	  //_________________________________________________________________


	  // Set chords and pitch
	  int32_t qualityValue = settings.adc_to_parameter(0, adc.channel(ADC_CHANNEL_PARAM0));
	  int32_t variationValue = settings.adc_to_parameter(1, adc.channel(ADC_CHANNEL_PARAM1));
	  int32_t voicingValue = settings.adc_to_fm(adc.channel(ADC_CHANNEL_FM));

	  chordManager.calculateVoicings( pitch + settings.pitch_transposition(),
			  	  	  	  	  	  	  qualityValue,
									  variationValue,
									  voicingValue,
									  (60 + settings.quantizer_root()) << 7);
	  osc.set_chords_parameters(chordManager.getChord1(),chordManager.getChord2(),chordManager.getChord3());
	  osc.set_pitch(chordManager.getPitch());
	  //_________________________________________________



	  // TRIGGER
	  if (trigger_flag) {
	    osc.Strike();
	    envelope.Trigger(ENV_SEGMENT_ATTACK);
	    trigger_flag = false;
	  }
	  //_________


	  // RENDER
	  int16_t audio_samples[outSize];
	  uint8_t* sync_buffer = sync_samples;
	  int16_t* render_buffer = audio_samples;

	  if (settings.GetValue(SETTING_AD_VCA) != 0
	    || settings.GetValue(SETTING_AD_TIMBRE) != 0
	    || settings.GetValue(SETTING_AD_COLOR) != 0
	    || settings.GetValue(SETTING_AD_FM) != 0) {
	    memset(sync_buffer, 0, outSize);
	  }
	  else
	  {
		  //Trigger in sync event (this is placed in sampler interrupt timer in braids)
		  memset(sync_buffer, 0, outSize);
		  if(flagTriggerInEvent==1)
		  {
			  flagTriggerInEvent=0;
			  sync_buffer[0]=1;
		  }
		  //______________________
	  }

	  osc.Render(sync_buffer, render_buffer, outSize);

	  // Copy to DAC buffer with sample rate and bit reduction applied.
	    int16_t sample = 0;
	    size_t decimation_factor = decimation_factors[settings.data().sample_rate];
	    uint16_t bit_mask = bit_reduction_masks[settings.data().resolution];
	    int32_t gain = settings.GetValue(SETTING_AD_VCA) ? ad_value : 65535;
	    uint16_t signature = settings.signature() * settings.signature() * 4095;
	    for (size_t i = 0; i < outSize; ++i) {
	      if ((i % decimation_factor) == 0) {
	        sample = render_buffer[i] & bit_mask;
	      }
	      sample = sample * gain_lp >> 16;
	      gain_lp += (gain - gain_lp) >> 4;
	      int16_t warped = 0; //ws.Transform(sample); // there is not enough RAM for this
	      render_buffer[i] = Mix(sample, warped, signature);
	    }

	  // Copy rendered buffer to DMA buffer (8bits)
	  uint32_t i;
	  for(i=0; i<outSize; i++)
	  {
		  out[i] = (uint8_t) (((uint16_t)(  -(render_buffer[i]) + 32768U )) >>8)  ;
	  }
	  //___________________________________________

}


void MainLoop::adcStateMachine(void)
{
	static uint8_t chn=0;

	adc.updateChannelValue(chn,mehal_readADC(chn));

	chn++;
	if(chn>=ADC_CHANNELS)
	{
		chn=0;
	}

}

