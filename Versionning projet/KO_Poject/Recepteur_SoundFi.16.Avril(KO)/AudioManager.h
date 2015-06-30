//
//  AudioManager.h
//  Recepteur_SoundFi
//
//  Created by François Le Brun on 16/04/2014.
//  Copyright (c) 2014 SoundFi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>
#import <Accelerate/Accelerate.h>

@interface AudioManager : NSObject <AVAudioSessionDelegate>
{
    @public
    Float64     sampleRate; // Ratio de l'échantillon audio
    
    NSInteger   deviceInputChannel; // Nombre de channel dispo sur le micro du périph
    
    BOOL        inputDeviceAvailable; // Test si un périphérique d'entrée est disponible
    
    AUGraph                         audioGraph;        // Le graph audio de l'app
    
    int micFxType; //1 =>FFT
    BOOL micFxOn;       // toggle for using mic fx
    float micFxControl; // multipurpose mix fx control slider
    
    
    // Before using an AudioStreamBasicDescription struct you must initialize it to 0. However, because these ASBDs
    // are declared in external storage, they are automatically initialized to 0.
    
    AudioStreamBasicDescription     stereoStreamFormat;     // standard stereo 8.24 fixed point
    AudioStreamBasicDescription     monoStreamFormat;       // standard mono 8.24 fixed point
    AudioStreamBasicDescription     SInt16StreamFormat;		// signed 16 bit int sample format
	AudioStreamBasicDescription     floatStreamFormat;		// float sample format (for testing)
    AudioStreamBasicDescription     auEffectStreamFormat;		// audio unit Effect format
    
    
    
    // audio graph nodes
    
    AUNode      iONode;             // node for I/O unit speaker
    AUNode      mixerNode;          // node for Multichannel Mixer unit
    AUNode      auEffectNode;       // master mix effect node
    AUNode      samplerNode;         // sampler node
    AUNode      filePlayerNode;      // fileplayer node
    AUNode      inputEffect1Node;    // input channel fx node
    
    
    
    // some of the audio units in this app
    
    AudioUnit                       ioUnit;                  // remote io unit
    AudioUnit                       mixerUnit;                  // multichannel mixer audio unit
    AudioUnit						auEffectUnit;           // this is the master effect on mixer output
	AudioUnit						auInputEffect1;         // this is a mixer input bus effect
    AudioUnit						auSampler;              // midi sampler
	AudioUnit						auFilePlayer;           // ios5 fileplayer
    
    // working buffers for sample data
    
	void *dataBuffer;               //  input buffer from mic/line
	float *outputBuffer;            //  fft conversion buffer
	float *analysisBuffer;          //  fft analysis buffer
    SInt16 *conversionBufferLeft;   // for data conversion from fixed point to integer
    SInt16 *conversionBufferRight;   // for data conversion from fixed point to integer
    
    // new instance variables for UI display objects
	
    int displayInputFrequency;              // frequency determined by analysis
    float displayInputLevelLeft;            // average input level for meter left channel
    float displayInputLevelRight;           // average input level for meter right channel
    int displayNumberOfInputChannels;       // number of input channels detected on startup

    
}



-(id)init;
-(void)startAudioSession;


@end
