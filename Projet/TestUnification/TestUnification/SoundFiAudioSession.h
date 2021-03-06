//
//  SoundFiAudioSession.h
//  RealTimeRecord
//
//  Created by François Le Brun on 17/04/2014.
//  Copyright (c) 2014 SoundFi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <UIKit/UITextChecker.h>
#import <UIKit/UIApplication.h>
#import <AVFoundation/AVFoundation.h>
#import <Accelerate/Accelerate.h>
#import <CoreLocation/CoreLocation.h>
#import "AESCrypt.h"

#define SPELLCHECKER 0

@protocol SoundFiEngineDelegate <NSObject>
@optional
/**---------------------------------------------------------------------------------------
 * MessageReceived
 *  ---------------------------------------------------------------------------------------
 */

/** This method is called when you have received a new soundFi message
 
 @param theMessage A string containing the message you just received
 */
- (void) messageReceived:(NSString*) theMessage;
/**---------------------------------------------------------------------------------------
 * StartingReception
 *  ---------------------------------------------------------------------------------------
 */

/** This method is called when you start receiving a message, you can use it to update your UI as you want
 
 */
- (void) startingReception;
/**---------------------------------------------------------------------------------------
 * ProgressStatut
 *  ---------------------------------------------------------------------------------------
 */

/** This method is called during the sending process it will give you the state of the sending by giving you a float from 0 to 1.
 
 @param percent A float from 0 to 1 that giving you the state of emission (esay to use with progress bar ^^)
 
 */
- (void) progressStatut:(float) percent;
/**---------------------------------------------------------------------------------------
 * FinishEmission
 *  ---------------------------------------------------------------------------------------
 */

/** This method is called when you have finish to send a message
 
 */
- (void) finishEmission;
/**---------------------------------------------------------------------------------------
 * SoundToLow
 *  ---------------------------------------------------------------------------------------
 */

/** This method is called when the engine detect a volume to low for sending a message
 
 */
- (void) soundToLow;

// Use for géolocalisation
/**---------------------------------------------------------------------------------------
 * LocalisationData
 *  ---------------------------------------------------------------------------------------
 */

/** This method is called when the engine have detect the place of the user and have a promotion for him
 
 @param lieu The place of the user (NSString)
 @param promotion The promotion that the user can benefit (NSString)
 */
- (void) localisationData:(NSString*)lieu :(NSString*)promotion;

// Use for paiement
/**---------------------------------------------------------------------------------------
 * TransactionStatutChange
 *  ---------------------------------------------------------------------------------------
 */

/** This method is called when a new step start in the paiement processing
 
 @param statut A int from 0 to 6 that give you the state of the paiement.
 @warning This statut is not final one, it will be probably change in the futur implementation
 
 */
- (void) transactionStatutChange:(int)statut;
/**---------------------------------------------------------------------------------------
 * TransactionData
 *  ---------------------------------------------------------------------------------------
 */

/** This method is called when user have received the data of the paiement

 @param transactionData A NSString that countain the price and the ID of the paiement
 */
- (void) transactionData:(NSString*)transactionData;
/**---------------------------------------------------------------------------------------
 * TransactionFinalState
 *  ---------------------------------------------------------------------------------------
 */

/** This method is called when user have received the final state of the paiemen (Is the paiement accepted or not ?).
 
 @param transactionFinalState A string that indicate if the paimement have been accepted, currently there are two possible value {valid:"0"} or {valid:"1"}.
 */
- (void) transactionFinalState:(NSString*)transactionFinalState;    // Call to send paeiment final statut to UI
@end


enum {
    SFReceivingMode = 0,
    SFSendingMode = 1,
};
typedef int SFMessagingMode;

/** 
 
 SoundFiAudioSession is the audio Engine of the SoundFi technologie. This class  include all the differrent mode that SoundFi provide : messaging, locating and paiement
 
 The SoundFiAudioSession mainly use Apple API provided for audio low level management. You will find in there some reference to AudioUnit and AndioGraph. If you'r not at ease with this part of Apple API please take a look as those link :
 
- [Constructing Audio Unit App](https://developer.apple.com/library/ios/documentation/MusicAudio/Conceptual/AudioUnitHostingGuide_iOS/ConstructingAudioUnitApps/ConstructingAudioUnitApps.html)
- [Using Specific Audio Unit](https://developer.apple.com/library/ios/documentation/MusicAudio/Conceptual/AudioUnitHostingGuide_iOS/UsingSpecificAudioUnits/UsingSpecificAudioUnits.html#//apple_ref/doc/uid/TP40009492-CH17-SW1)
 
 I'll try to synthetise how does the engine work. First of all, this engine work in real time, that's why, we get at each moment an audio sample that we will analyse immediatly (ideally before the next one ^^).
 How can we do that ??? With the callBack my dear ! Every time an audio unit will require a sample, they will call a function (a callback function). It's in this function that we will do our task, analysing.
 
 The analyse is done with FFT (Fast Fourrier Transform), that is able to transform a temporal sample to a frequency sample and détect the dominant frequency. In the code, you will see 2 FFT procesing : 
 - A light one that use low CPU ressources and use for the background processing (low accuracy :/)
 - A bigger one that do good work in foreground and can give the frequency close to 1Hz in good condition. (http://www.dspdimension.com/)
 
 This this is global principle. Now, invite you to start reading this doc :).
 
 For user : SoundFi use hight level abstraction to make your life simple, in this way, you will only need 2 or 3 methods to make the technologie working, we will see later witch one.
 
 For SoundFiDev : Welcom to low level, have fun, and hang on. This documenation give you information for the function purpose, if you want to know more about how they processing, you can find a lot of comment in the code that detail the working process.

    
 @warning This documentation is not ended yet. Documentation v1.00.
 
 All the class function are here, but some C function come upon this code, you could find there documentation directly in the code.
 
 */

@interface SoundFiAudioSession : NSObject <CLLocationManagerDelegate>
{
    @public
    //Global audio Session
    AVAudioSession *mySession;
    
    //AudioUnit for emission and reception
    AudioUnit           audioUnit;
    AudioUnit           mixerUnit;
    AudioUnit           emissionUnit;
    
    //Graph component
    AUNode              ioNode;
    AUNode              mixerNode;
    AUGraph             myGraph;
    
    //Sample frequency and rate for audioSession initialisation
    int                 sampleFrequency;
    float               sampleRate;
    
    //Ampty sample to clean I/O
    AudioUnitSampleType *emptySample;
    
    //Callback Setup
    SInt16              *samplesBuffer;
    
    // fft
	FFTSetup            fftSetup;			// fft predefined structure required by vdsp fft functions
	COMPLEX_SPLIT       fftA;               // complex variable for fft
	int                 fftLog2n;           // base 2 log of fft size
    int                 fftN;               // fft size
    int                 fftNOver2;          // half fft size
	size_t              fftBufferCapacity;	// fft buffer size (in samples)
	size_t              fftIndex;           // read index pointer in fft buffer
    
    void                *dataBuffer;        //  input buffer from mic/line
	float               *outputBuffer;      //  fft conversion buffer
	float               *analysisBuffer;    //  fft analysis buffer
    
    //Transaction information for the clasic message
    BOOL                isInitiate;
    BOOL                isTimeOut;
    BOOL                receptionMode;
    BOOL                emissionMode;
    
    //Reception mode variables for the clasic message
    NSMutableString     *messageReceive;
    
    //Emission mode variables for the clasic message
    double              amplitude;
    double              theta;
    NSString            *myMessage;
    pthread_mutex_t     emissionMutex;
    int                 nbCaracRepeat;
    BOOL                initSequence;
    int                 nbrRepeatInit;
    BOOL                emissionCanBeStop;
    float                 minimumVolume;
    
    //Share mode's variable
    int                 compteur;           //Use to define when there is a time out
    
    // Delegate to respond back
    id <SoundFiEngineDelegate> _delegate;
    
    //Background boolean
    BOOL                enableBackground;   //Actually use to enable and disable listenning in BG
    
    
    
    
    
    //Géomarketing variable
    int                 *curentFrequencyData;
    int                 nbrVal;
    int                 nbrSpotInZone;              // Nombre de spot dans la zone
    NSMutableArray      *zoneArray;                 // Liste des spots (noms) qui compose la zone
    NSString            *currentPos;
    NSString            *lastPos;
    NSString            *lastPosValidate;
    BOOL                isInZone;
    BOOL                geoIsInitiate;
    
    //Géolocalisation stuff (GPS, latitude, longitude ....)
    CLLocationManager           *locationManager;
    NSTimer                     *timer;
    float                       lastLat;
    float                       lastLong;
    BOOL                        isHightAccuracy;
    
    // Global fonctionnement variable
    BOOL                engineIsRunning;
    int                 nbrEchantillon;
    
    
    
    
    
    //Paiement variables
    int                 transactionState;
    BOOL                timeOut;
    int                 nbEssai;
    int                 compteurProcess;
    int                 timeOutProcess;
    
    //User information for paiment
    NSString            *myID;
    NSString            *cryptedPassAES;
    NSString            *paiementStatut;
    NSString            *paiementInformation;
    
    
    //Modes variable information
    BOOL                geolocalisationMode;
    BOOL                simpleMessagingMode;
    BOOL                paiementMode;
}

//Delegate property
@property (nonatomic,strong) id delegate;


/**---------------------------------------------------------------------------------------
 * @name Start/Stop engine methods
 *  ---------------------------------------------------------------------------------------
 */
/** Allow you to start SoundFi engine for receiving or sending function.
 
 This function start the audio unit for send of receiving, and proceed to a sound check in case of sendingMode is use.
 
 @param mode The mode that you want to use, SFReceivingMode or SFSendingMode
 @param message The string to send (only if you use SFSendingMode, otherwise put nil)
 @return Return 1 if every thing is ok, or 0 in the other case.
 
 @warning Here some error that can be raised by the function
 
 - AUGraphStart error
 
 */
-(int)startAudioUnit:(SFMessagingMode)mode : (NSString*)message;



/**---------------------------------------------------------------------------------------
 * @name Emission methods
 * EmissionSampleCalcul
 *  ---------------------------------------------------------------------------------------
 */

/** Create the audio wave with the required frequency for a given sample.
 
 This function will generate an audio sinusoidal wave for each audio sample needed. This function is called in the renderCallBack.
 
 @param frequence This is the desired frequency for this sample (int).
 @param numFrames The number of frame that required the audio hardware (int).
 @param buffer The audio sample, store as a Float32 array.
 */
-(void)emissionSampleCalcul:(int)frequence : (int)numFrames : (Float32 *)buffer;



/**---------------------------------------------------------------------------------------
 * GetASCIIfrequency
 *  ---------------------------------------------------------------------------------------
 */

/** Coordinate the state of the emission and asign a frequency to a caracter.
 
 This function make the link between the audio and the string to send. For each audioSample needed, this function will tell what frequency we need.
 
 @return Return the frequency associate with the caracter or the init/stop frequency
 */
-(int)getASCIIFrequency;



/**---------------------------------------------------------------------------------------
 * @name TimeOut methods
 * checkTimeOut
 *  ---------------------------------------------------------------------------------------
 */
/** Check if the comunication time out, use by all soundFi mode.
 
 @see checkProcessTimeOut
 */
-(void)checkTimeOut;



/**---------------------------------------------------------------------------------------
 * checkProcessTimeOut
 *  ---------------------------------------------------------------------------------------
 */
/** Check if there is TimeOut during one step of the paiement process.
 
 @see checkTimeOut
 
 */
-(void)checkProcessTimeOut;



/**---------------------------------------------------------------------------------------
 * @name Deprecated methods
 * switchBackgroundChanged
 *  ---------------------------------------------------------------------------------------
 */
/** Fonction Call to enable or disable background listenning.
 
 @deprecated Replace with : enableBackGround and disableBackground.
 @see enableBackground
 @see disableBackground
 */
-(void)switchBackgroundChanged:(id)sender;


/**---------------------------------------------------------------------------------------
 * @name Reception methods
 * SampleTreatment
 *  ---------------------------------------------------------------------------------------
 */

/** This function deal with the recepetion state, for background and foreground.
 
 It perform the FFT method according to the UIApplication state. It will call three other function geolocalisationReceptionSampleTreatment, messagingReceptionSampleTreatment and paiementReceptionSampleTreatment.
 
 NOTE: The FFT for background is less accurate than the other but use only 2%CPU
 
 @param numFrames The number of frame (int) that the FFT must perform.
 
 @see geolocalisationReceptionSampleTreatment
 @see messagingReceptionSampleTreatment
 @see paiementReceptionSampleTreatment
 */
-(void)sampleTreatment:(int)numFrames;



/**---------------------------------------------------------------------------------------
 * @name Engine informations methods
 * LocalisationModeIsEnable
 *  ---------------------------------------------------------------------------------------
 */
/** Check if the localisation mode is enable
 
 @return TRUE if the localisation mode is enable
 @see activateSoundFiLocalisation
 @see desactivateSoundFiLocalisation
 */
-(BOOL)localisationModeIsEnable;
-(BOOL)messagingModeIsEnable;
-(BOOL)paiementModeIsEnable;
-(BOOL)backgroundIsEnable;
-(BOOL)engineIsRunning;

/**---------------------------------------------------------------------------------------
 * @name Engine control methods
 * EnableBackground
 *  ---------------------------------------------------------------------------------------
 */
/** Allow background processing
 
 @see desableBackground
 @see backgroundIsEnable
 */
-(void)enableBackground;
-(void)desableBackground;


/**---------------------------------------------------------------------------------------
 * @name Paiement methods
 * StartpaiementProcessing
 *  ---------------------------------------------------------------------------------------
 */
/** Call at start of paiement
 
 This method permit to start processing a paiement, it enable the paiement mode and disable the other mode. To resume, it prepare the battlefield.
 
 @see endPaiementProcess
 */
-(void)startpaiementProcessing;



/**---------------------------------------------------------------------------------------
* @name Volume control methods
 * ChangeMinimumVolume
 *  ---------------------------------------------------------------------------------------
 */
/** Allow you to set the minimum required volume for sending message
 
 The value must be between 0 and 1. It's higtly recommended to use a value between 0.6 and 0.9.
 @param newMinVolume The new minimum volume you want to set
 @see volumeControl
 */
-(void)changeMinimumVolume:(float)newMinVolume;

@end
