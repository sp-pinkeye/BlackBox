#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <sstream>
#include <time.h>

#include <vector>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    midiSetup() ;

    ui->setupUi(this);

    getMainParams() ;

    getPresetEdit() ;

    updateDisplay();
}

MainWindow::~MainWindow()
{
    delete midiin ;
    delete midiout ;
    delete ui;
}

/*
 *  All the values can be directed to same method that sends MIDI message
 * Single Parameter Changed to the Blackbox device
 */

void MainWindow::on_selectAmpModel_currentIndexChanged(int index)
{
    std::cout << "Amp " << index << std::endl ;

    // Third param will actually be the message value
    presetParams[AMP_MODEL] = index ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, AMP_MODEL, index ) ;
}

/*
 * User Preset selector
 */

void MainWindow::on_selectPreset_valueChanged( int value )
{
    std::cout << "Preset Select " << value << std::endl ;

    unsigned char selectedValue ;
    unsigned char selectedBank ;

    if( value >= 100 ){
        selectedValue = value - 100 ;
        selectedBank = SELECT_USER_PRESET ;
    }else{
        selectedValue = value;
        selectedBank = SELECT_FACTORY_PRESET ;
    }

    std::vector <unsigned char> message (3);
    message[0] = SELECT_PRESET ;
    message[1] = selectedBank ;
    message[2] = selectedValue ;

    sendSysex( &message ) ;

    mainParams[MAIN_PARAMS_ACTIVE_PRESET] = value ;

    std::cout << "Select Preset " << ( int)value << std::endl ;

}

void MainWindow::on_ampOnOff_clicked(bool checked)
{
    std::cout << "Amp On/Off " << checked << std::endl ;
    midi_set_devices_on_off( AMP_ONOFF, checked) ;

}

void MainWindow::on_dialDrive_valueChanged(int value)
{
    std::cout << "Drive " << value << std::endl ;
    presetParams[AMP_DRIVE] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, AMP_DRIVE, value ) ;

    ui->labelAmpDrive->setText(QString::number(value));

}

void MainWindow::on_dialBass_valueChanged(int value)
{
    std::cout << "Bass " << value << std::endl ;
    presetParams[AMP_BASS] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, AMP_BASS, value ) ;
    ui->labelAmpBass->setText(QString::number(value));

}

void MainWindow::on_dialMid_valueChanged(int value)
{
    std::cout << "Mid " << value << std::endl ;
    presetParams[AMP_MID] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, AMP_MID, value ) ;
    ui->labelAmpMid->setText(QString::number(value));

}

void MainWindow::on_dialTreble_valueChanged(int value)
{
    std::cout << "Treble " << value << std::endl ;
    presetParams[AMP_TREBLE] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, AMP_TREBLE, value ) ;
    ui->labelAmpTreble->setText(QString::number(value));

}

void MainWindow::on_dialVolume_valueChanged(int value)
{
    std::cout << "Volume " << value << std::endl ;
    presetParams[AMP_VOLUME] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, AMP_VOLUME, ( unsigned char)value ) ;
    ui->labelAmpVolume->setText(QString::number(value));
}

// Rename pushbutton to LoadPreset
void MainWindow::on_loadPreset_clicked()
{
    // Load Preset Edit Buffer and init values
    getPresetEdit();
    updateDisplay() ;
}
// Rename to Save preset
void MainWindow::on_savePreset_clicked()
{
    savePreset() ;
}
void MainWindow::savePreset(){

  std::vector<unsigned char>  message  (2);

  message[0] = TRANSMIT_PRESET ;
  message[1] = MIDI_FILE_VERSION ;

  // Save the Preset To device

  encodeData( &presetParams , &message ) ;

  sendSysex( &message ) ;

  waitMidiIn( &message, SAVE_COMPLETE ) ;
}
void MainWindow::on_delayOnOff_clicked(bool checked)
{
    std::cout << "Delay On/Off " << checked << std::endl ;

    /*
     *  For this we must get the current value and or this in
     * 0-118 consisting of 0-99 (fixed rates shown in display) followed by
     * 100-118 (19 tempo-synced rates: 2M, 2Mt, 1Md, 1M, 1Mt, 2d, 2n, 2t, 4d, 4n, 4t,8d, 8n, 8t, 16d, 16n, 16t, 32n, 32t.)
     */
   // midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, , value ) ;

    midi_set_devices_on_off( DELAY_ONOFF, checked) ;
}

void MainWindow::on_dialDelayTime_valueChanged(int value)
{
    std::cout << "Delay Time " << value << std::endl ;
    presetParams[DELAY_TIME] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, DELAY_TIME, ( unsigned char)value ) ;

    /*

    QString milliseconds = {
                                "10ms", "20ms", "30ms", "40ms", "50ms", "60ms", "70ms", "80ms", "90ms", "100ms",

        117 145 172 199
     *  1st 10 10ms steps
     *  Now start at 117 add 27ms
     *
     */
    // 100-118 (19 tempo-synced rates: 2M, 2Mt, 1Md, 1M, 1Mt, 2d, 2n, 2t, 4d, 4n, 4t,    8d, 8n, 8t, 16d, 16n, 16t, 32n, 32t.)
    QString synced[] = { "2M", "2Mt", "1Md", "1M", "1Mt", "2d", "2n", "2t", "4d", "4n", "4t", "8d", "8n", "8t", "16d", "16n", "16t", "32n", "32t" } ;
    QString display ;
    if( value < 100 ){
      if( value < 11 ){
          display = QString::number(( value )* 10 ) ;
      }else{
        // Starts at 10 - 117ms, 12 then add 28ms for each increment

          float result ;
          if( value == 11){
             result = 117 ;
          }else if( value == 12 ){
            result = 145 ;
          }else{
             result = 145 + ( ( value -12) * 27.2 ) ;
            }
          std::cout << "INDEX = " << value << " RESULT = "<< ( int)result << std::endl ;
          display = QString::number( ( int)result ) ;
      }
      display.append( " ms") ;

    }else{

        display = synced[value-100];

    }
    ui->labelDelayTime->setText(display);

}

void MainWindow::on_dialDelayRepeats_valueChanged(int value)
{
    std::cout << "Delay Repeats " << value << std::endl ;
    presetParams[DELAY_REPEATS] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, DELAY_REPEATS, ( unsigned char)value ) ;
    ui->labelDelayRepeats->setText(QString::number(value));
}

void MainWindow::on_dialDelayLevel_valueChanged(int value)
{
    std::cout << "Delay Level " << value << std::endl ;
    presetParams[DELAY_LEVEL] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, DELAY_LEVEL, ( unsigned char)value ) ;
    ui->labelDelayLevel->setText(QString::number(value));
}

void MainWindow::on_dialDrumsDelay_valueChanged(int value)
{
    std::cout << "Treble " << value << std::endl ;
    presetParams[DRUMS_DELAY] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, DRUMS_DELAY, ( unsigned char)value ) ;
    ui->labelDelayDrums->setText(QString::number(value));
}

void MainWindow::on_effectOnOff_clicked(bool checked)
{
    std::cout << "Effect On/Off " << checked << std::endl ;
    midi_set_devices_on_off( EFFECT_ONOFF, checked) ;
}

void MainWindow::on_selectEffect_currentIndexChanged(int index)
{
    std::cout << "Effect  " << index << std::endl ;
    /*This parameter is either FX Speed, FX Freq or unused, depending on which effect is selected:
    1) FX Freq is active if one of the following effects is selected: RNFI4-RNFI24, RNFL4-RNFL24, ATOWA1-8, WAHPDL, TKBOX1-6, FIXFLT,
    FIXFLA, TLKPDL, SCIFI1-4, FLSQ1-20, and all 6 MIDI effects at end of list. In this case, the range is 0-99 for 100 arbitrary filter frequencies.
    2) This parameter is unused for the following effects: TRSQ1-20, ARSQ1-20.
    3) For all other effects, FX Speed is active. In this case, the range is 0-114 consisting of 100 fixed arbitrary speeds from 0 to 99 followed by 15
    tempo-synced speeds 100-114 (8M, 4M, 2M, 1M, 1Mt, 2n, 2t, 4n, 4t, 8n, 8t, 16n, 16t, 32n, 32t).
    6
    FX Depth or FX Key
    This parameter is either FX DEPTH, FX KEY or unused, depending on which effect is selected:
    1) FX Key is active if ARSQ1-20 or MNT2FL is the selected effect. In this case, range is 0-99 (arpeggio sequence may be transposed to one of
    100 semitones).
    2) This parameter is unused for the following effects: FIXFLT and FIXFLA.
    3) For all other effects, FX Depth is active. In this case, range is 0-198, representing -99 (negative modulation depth) to 0 to 99 (positive
    modulation depth).

    FX Feq active 00-99
        RNFI4-RNFI24, RNFL4-RNFL24, ATOWA1-8, WAHPDL, TKBOX1-6, FIXFLT,
        FIXFLA, TLKPDL, SCIFI1-4,
        FLSQ1-20
        21-25, 26-30, 31-38,39,40-45,47-48,49,50-54,75-94, 115-120
    FX Freq Not userData
        TRSQ1-20, ARSQ1-20.
        55-74.95-114
    FX Speed for all other effects 0-114
       0-99 and then tempo-synced speeds 100-114 (8M, 4M, 2M, 1M, 1Mt, 2n, 2t, 4n, 4t, 8n, 8t, 16n, 16t, 32n, 32t)


    FX KEY active 0-99
        ARSQ1-20 or MNT2FL
            95-114, 119
    FX Depth Not used
        FIXFLT and FIXFLA

    FX Depth ALL OTHERS 0-198 => -99 t0 99
    */

    /*
        START WITH THE Speed/Freq Dial Range and labels
        Might need to stash value ( actually will be in the stored preset )
    */
    if( ( ( index >=55 )&&(index<=74)) || ( ( index >=95 )&&(index<=114) ) ){
        // Set to SPEED/FREQ inactive
        ui->labelSpeedFreq->setText("Not Used");
        ui->dialSpeed->setEnabled(false) ;
    }else if( ( ( index >=21 )&&(index<=45)) ||
              ( ( index >=47 )&&(index<=54)) ||
              ( ( index >=75 )&&(index<=94)) ||
              ( ( index >=115 )&&(index<=120))){
            //        21-25, 26-30, 31-38,39,40-45,47-48,49,50-54,75-94, 115-120
        // Set t0 frequency range 0-99
        ui->labelSpeedFreq->setText("Frequency");
        ui->dialSpeed->setRange( 0, 99 );
        ui->dialSpeed->setEnabled(true) ;
    }else{
        //Set to Speed 0-99 plus 100-114
        ui->dialSpeed->setEnabled(true) ;
        ui->labelSpeedFreq->setText("Speed");
        ui->dialSpeed->setRange( 0, 114 );
    }

       /*
        *   Now we do same with Depth
        *     FX KEY active 0-99
        ARSQ1-20 or MNT2FL
            95-114, 119
    FX Depth Not used
        FIXFLT and FIXFLA

    FX Depth ALL OTHERS 0-198 => -99 t0 99

        */
    if(  ( index >=47 )&&(index<=48) ){
        // Set  inactive
        ui->labelDepthKey->setText("Not Used");
        ui->dialDepth->setEnabled(false) ;
    }else if( ( ( index >=95 )&&(index<=114)) ||
              index == 119 ){
            //        21-25, 26-30, 31-38,39,40-45,47-48,49,50-54,75-94, 115-120
        // Set tp frequency range 0-99
        ui->dialDepth->setEnabled(true) ;
        ui->labelDepthKey->setText("Key");
        ui->dialDepth->setRange( 0, 99 );
    }else{
        ui->dialDepth->setEnabled(true) ;
        ui->labelDepthKey->setText("Depth");
        ui->dialDepth->setRange( 0, 198 );
    }
    presetParams[EFFECTS_SELECT] = index ;

    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, EFFECTS_SELECT, ( unsigned char)index ) ;

}

void MainWindow::on_dialSpeed_valueChanged(int value)
{
    std::cout << "Effect Speed " << value << std::endl ;

    /*
     *  Is it Freqency Speed of Not userData
     *
     * Frequency just  print number
     * Not Used do nothing - will not be called
     * Speed
     *       0-99 and then tempo-synced speeds 100-114 (8M, 4M, 2M, 1M, 1Mt, 2n, 2t, 4n, 4t, 8n, 8t, 16n, 16t, 32n, 32t)
     */

    QString display ;

    if( ui->labelSpeedFreq->text() == "Speed" && ( value > 99 )){

        QString synched[] =  {"8M", "4M", "2M", "1M", "1Mt", "2n", "2t", "4n", "4t", "8n", "8t", "16n", "16t", "32n", "32t" };
        display = synched[value-100] ;
    }else{
            display = QString::number(value) ;
    }
    presetParams[EFFECTS_SPEED] = value ;

    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, EFFECTS_SPEED, ( unsigned char)value ) ;

    ui->labelSpeed->setText(display);
}

void MainWindow::on_dialDepth_valueChanged(int value)
{
    std::cout << "Effect Depth " << value << std::endl ;
    presetParams[EFFECTS_DEPTH] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, EFFECTS_DEPTH, ( unsigned char)value ) ;

    if( ui->labelDepthKey->text() == "Depth"){
        ui->labelDepth->setText(QString::number(( value-99)));
    }else{
        // Key of E is 24 so we have following keys
        //  E,  F,  F#, G,      G#,     A,      A#,     B,  C,  C#,     D,  D#
        //  24, 25, 26, 27,     28,     29,     30,     31, 32, 33,     34, 35
        // E %12 = 0, F %12 = 1
        // USE value /12 to get number
        // So we have key = value%12
        // number = value /12
        QString keys[] = { "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#" };
        QString display = keys[value%12];
        display.append( QString::number( value/12 )) ;
        ui->labelDepth->setText(display);
    }

}

void MainWindow::on_dialWetDry_valueChanged(int value)
{
    std::cout << "Effect Mix " << value << std::endl ;
    presetParams[EFFECTS_LEVEL] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, EFFECTS_LEVEL, ( unsigned char)value ) ;
    ui->labelEffectWet->setText(QString::number(value));

}


void MainWindow::on_reverbOnOff_clicked(bool checked)
{
    std::cout << "Reverb On/Off " << checked << std::endl ;
    midi_set_devices_on_off( REVERB_ONOFF, checked) ;

}

void MainWindow::on_dialReverbTime_valueChanged(int value)
{
    std::cout << "Reverb Type " << value << std::endl ;
    presetParams[REVERB_TIME] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, REVERB_TIME, ( unsigned char)value ) ;

    // Display the actual setting
    // 0 - TINY, 1 - SMALL, 2 - MEDIUM, 3 - LARGE, 4 - HUGE

    QString display ;
    switch( value ){
    case 0:
        display = "TINY" ;
        break ;
    case 1:
        display = "SMALL" ;
        break ;
    case 2:
        display = "MEDIUM" ;
        break ;
    case 3:
        display = "LARGE" ;
        break ;
    case 4:
        display = "HUGE" ;
        break ;
    }

    ui->labelReverbTime->setText(display);

}

void MainWindow::on_dialReverbHiCut_valueChanged(int value)
{
    std::cout << "Reverb Hi Cut " << value << std::endl ;
    presetParams[REVERB_HICUT] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, REVERB_HICUT, ( unsigned char)value ) ;

    ui->labelReverbHiCut->setText(QString::number(value));

}

void MainWindow::on_dialReverbLevel_valueChanged(int value)
{
    std::cout << "Reverb Level " << value << std::endl ;
    presetParams[REVERB_LEVEL] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, REVERB_LEVEL, ( unsigned char)value ) ;

    ui->labelReverbLevel->setText(QString::number(value));

}

void MainWindow::on_dialCompressor_valueChanged(int value)
{
    std::cout << "Compressor Level " << value << std::endl ;
    presetParams[COMPRESSOR_LEVEL] = value ;
    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, COMPRESSOR_LEVEL, ( unsigned char)value ) ;

    ui->labelCompressor->setText(QString::number(value));
}
/*
 *  MIDI METHODS HERE
 */

void MainWindow::printMessage( std::vector <unsigned char>* message, bool hex ){

unsigned int nBytes = message->size();

    std::cout << "Print Message" << std::endl;

    for ( unsigned int i=0; i<nBytes; i++ ){

        if( hex ){
            std::cout << "Byte " << i << " = " << std::hex<< std::uppercase << (int)message->at(i) <<std::nouppercase << std::dec << std::endl;;
        }else{
            std::cout << "Byte " << i << " = " << (int)message->at(i) << std::endl;;
        }
    }

}

void MainWindow::printMainParams( ){
// 16 Bytes only 13 used

    std::cout << "====================================================================="<< std::endl;
    std::cout << "MAIN PARAMS"<< std::endl;

    std::cout << "Active Preset "<< ( int)mainParams.at(0) <<std::endl;
    std::cout << "Active Drumbeat "<< ( int)mainParams.at(1) <<std::endl;
    std::cout << "Guitar/Drum Balance "<< ( int)mainParams.at(2) <<std::endl;
    std::cout << "Footswitch 1 Assign "<< ( int)mainParams.at(3) <<std::endl;
    std::cout << "Footswitch 1 Assign "<< ( int)mainParams.at(4) <<std::endl;
    std::cout << "Fx Sync Source "<< ( int)mainParams.at(5) <<std::endl;
    std::cout << "Fx Input Source "<< ( int)mainParams.at(6) <<std::endl;
    std::cout << "Gate "<< ( int)mainParams.at(7) <<std::endl;
    std::cout << "Send Midi Clock Enable "<< ( int)mainParams.at(8) <<std::endl;
    std::cout << "Link Drumbeat "<< ( int)mainParams.at(9) <<std::endl;
    std::cout << "Tempo Source "<< ( int)mainParams.at(10) <<std::endl;
    std::cout << "Global Tempo "<< ( int)mainParams.at(11) <<std::endl;
    std::cout << "MIDI Channel "<< ( int)mainParams.at(12) <<std::endl;
    std::cout << "====================================================================="<< std::endl;

}

void MainWindow::printPresetEdit( ){

    std::cout << "====================================================================="<< std::endl;
    std::cout << "Preset PARAMS"<< std::endl;

    std::cout << "Amp Model "<< ( int)presetParams.at(AMP_MODEL) <<std::endl;
    std::cout << "Amp Drive "<< ( int)presetParams.at(AMP_DRIVE) <<std::endl;
    std::cout << "Amp Bass "<< ( int)presetParams.at(AMP_BASS) <<std::endl;
    std::cout << "Amp Treble "<< ( int)presetParams.at(AMP_TREBLE) <<std::endl;
    std::cout << "Effect "<< ( int)presetParams.at(EFFECTS_SELECT) <<std::endl;
    std::cout << "Effect Speed "<< ( int)presetParams.at(EFFECTS_SPEED) <<std::endl;
    std::cout << "Effect Depth "<< ( int)presetParams.at(EFFECTS_DEPTH) <<std::endl;
    std::cout << "Effect Wet/Dry "<< ( int)presetParams.at(EFFECTS_LEVEL) <<std::endl;
    std::cout << "Delay Time "<< ( int)presetParams.at(DELAY_TIME) <<std::endl;
    std::cout << "Delay Repeats "<< ( int)presetParams.at(DELAY_REPEATS) <<std::endl;
    std::cout << "Delay  Volume "<< ( int)presetParams.at(DELAY_LEVEL) <<std::endl;
    std::cout << "Drums to FX "<< ( int)presetParams.at(11) <<std::endl;
    std::cout << "Preset Volume "<< ( int)presetParams.at(12) <<std::endl;
    std::cout << "Exp Pedal Assign "<< ( int)presetParams.at(13) <<std::endl;
    std::cout << "Tempo "<< ( int)presetParams.at(14) <<std::endl;

    std::cout << "AmpFxDlyRevOn 0x"<< std::hex<<( int)presetParams.at(DEVICE_ONOFF)<<std::dec  <<std::endl;
    std::cout << "Amp On 0x"<< std::hex<<( int)(presetParams.at(DEVICE_ONOFF)& 0x01)<<std::dec  <<std::endl;
    std::cout << "Fx On 0x"<< std::hex<<( int)( (presetParams.at(DEVICE_ONOFF)& 0x02 )>>1)<<std::dec  <<std::endl;
    std::cout << "Delay On 0x"<< std::hex<<( int)( (presetParams.at(DEVICE_ONOFF)& 0x04 )>>2)<<std::dec  <<std::endl;
    std::cout << "Reverb On 0x"<< std::hex<<( int)( (presetParams.at(DEVICE_ONOFF)& 0x08 )>>3)<<std::dec  <<std::endl;

    // Goes wrong here
    std::cout << "Amp Mid "<< ( int)presetParams.at(AMP_MID) <<std::endl;
    std::cout << "Linked Drumbeat "<< ( int)presetParams.at(17) <<std::endl;
    std::cout << "Unused "<< ( int)presetParams.at(18) <<std::endl;
    std::cout << "Compressor "<< ( int)presetParams.at(COMPRESSOR_LEVEL) <<std::endl;
    std::cout << "Reverb Level "<< ( int)presetParams.at(REVERB_LEVEL) <<std::endl;
    std::cout << "Reverb Time "<< ( int)presetParams.at(REVERB_TIME) <<std::endl;
    std::cout << "Reverb HiCut "<< ( int)presetParams.at(REVERB_HICUT) <<std::endl;
    std::cout << "Unused "<< ( int)presetParams.at(23) <<std::endl;
    std::cout << "Unused "<< ( int)presetParams.at(24) <<std::endl;
    std::cout << "Unused "<< ( int)presetParams.at(25) <<std::endl;
    std::cout << "Unused "<< ( int)presetParams.at(26) <<std::endl;
    std::cout << "Unused "<< ( int)presetParams.at(27) <<std::endl;
    std::cout << "Unused "<< ( int)presetParams.at(28) <<std::endl;
    std::cout << "Unused "<< ( int)presetParams.at(29) <<std::endl;
    std::cout << "Unused "<< ( int)presetParams.at(30) <<std::endl;
    std::cout << "Unused "<< ( int)presetParams.at(31) <<std::endl;

    std::string text[] = { " ", "0", "1", "2", "3", "4","5", "6", "7", "8", "9", "-", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K","L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "*", "+", "(", ")", "\\", "/", "‘", "<", ">", "?", "_", "[", "]" } ;

    std::string name ;
    for( int i = 32; i < 38; i++ ){
        std::cout << ( int)presetParams.at(i) << std::endl ;
        name += text[presetParams.at(i)] ;
     }

    std::cout << "Name = '" <<  name <<"'"<< std::endl ;

    std::cout << "====================================================================="<< std::endl;

}


void MainWindow::waitMidiIn(std::vector <unsigned char>* message, unsigned char wait){

    std::cout <<"waitMidiIn message "<< (int)wait <<std::endl ;

    bool done = false ;
    int milisec = 10; // length of time to sleep, in milliseconds
    unsigned int nBytes ;

    struct timespec req ;
    req.tv_sec = 0;
    req.tv_nsec = milisec * 1000000L;

    while ( !done ) {

        midiin->getMessage( message );

        nBytes = message->size();

        if ( nBytes > 0 ) {
            printMessage( message, true ) ;
            if( nBytes > 7 && (message->at(8) == wait )){
                std::cout << "Wait = " << ( int)wait << std::endl;
                std::cout << "Message 8 = " << ( int)message->at(8) << std::endl;
                done = true ;
            }
        }
        // Sleep for 10 milliseconds ... platform-dependent.

        nanosleep(&req, (struct timespec *)NULL);
    }

}

/*
    Get Main Params and decode for storage
*/
void MainWindow::getMainParams( ){

    std::vector<unsigned char> message ;

    std::vector<unsigned char> sysex ( 1, REQUEST_MAIN_PARAMS ) ;

    sendSysex( &sysex ) ;

    waitMidiIn( &message, TRANSMIT_MAIN_PARAMS ) ;

   //printMessage( &message );

   std::vector<unsigned char> data ( message.begin()+10,message.end()-1 ) ;

   //printMessage( &data ) ;

    decodeData( &data, &mainParams );

//    printMainParams()  ;
}

/*
    Get Preset Edit and decode for storage
*/
void MainWindow::getPresetEdit( ){

    std::vector<unsigned char> message ;

    std::vector<unsigned char> sysex ( 1,REQUEST_PRESET_EDIT ) ;

    sendSysex( &sysex ) ;

    waitMidiIn( &message, TRANSMIT_PRESET_EDIT ) ;

 //  printMessage( &message ) ;

    // Now strip the sysex wrapper
    std::vector<unsigned char> data ( message.begin()+10,message.end()-1 ) ;

    decodeData( &data, &presetParams );
}

/*
    Take the loaded Main Params and Preset buffer and update controls
*/
void MainWindow::updateDisplay(){

    // Main Params
    // Current Preset set appropriate Widget(MAIN_PARAMS_ACTIVE_PRESET)-100) ;
    // Is it the User or Factory Bank
    ui->selectPreset->setValue( mainParams.at(MAIN_PARAMS_ACTIVE_PRESET)) ;
    // There will be more !

    // Preset
    // Here we have a lot of values to update

    // Amplifier Model
    ui->selectAmpModel->setCurrentIndex( presetParams.at(AMP_MODEL)) ;
    ui->dialDrive->setValue( presetParams.at(AMP_DRIVE)) ;
    ui->dialBass->setValue( presetParams.at(AMP_BASS)) ;
    ui->dialMid->setValue( presetParams.at(AMP_MID)) ;
    ui->dialTreble->setValue( presetParams.at(AMP_TREBLE)) ;
    ui->dialVolume->setValue( presetParams.at(AMP_VOLUME)) ;
    ui->dialCompressor->setValue( presetParams.at(COMPRESSOR_LEVEL)) ;

    // Delay
    ui->dialDelayTime->setValue( presetParams.at(DELAY_TIME)) ;
    ui->dialDelayRepeats->setValue( presetParams.at(DELAY_REPEATS)) ;
    ui->dialDelayLevel->setValue( presetParams.at(DELAY_LEVEL)) ;
    ui->dialDrumsDelay->setValue( presetParams.at(DRUMS_DELAY)) ;

    // Effect
    ui->selectEffect->setCurrentIndex( presetParams.at(EFFECTS_SELECT)) ;
    ui->dialSpeed->setValue( presetParams.at(EFFECTS_SPEED)) ;
    ui->dialDepth->setValue( presetParams.at(EFFECTS_DEPTH)) ;
    ui->dialWetDry->setValue( presetParams.at(EFFECTS_LEVEL)) ;


    // Effect
    ui->dialReverbTime->setValue( presetParams.at(REVERB_TIME)) ;
    ui->dialReverbHiCut->setValue( presetParams.at(REVERB_HICUT)) ;
    ui->dialReverbLevel->setValue( presetParams.at(REVERB_LEVEL)) ;


    // ON OFF FOR EACH is DEVICE_ONOFF
    // bits 0-3 set so we need to extract and shift
    // AMP = presetParams.at( DEVICE_ONOFF ) & 0x01
    // EFFECT = ( presetParams.at( DEVICE_ONOFF ) & 0x02 )>>1
    // DELAY = ( presetParams.at( DEVICE_ONOFF ) & 0x04 )>>2
    // REVERB = ( presetParams.at( DEVICE_ONOFF ) & 0x08 )>>3

    ui->ampOnOff->setChecked( presetParams.at( DEVICE_ONOFF ) & 0x01 );
    ui->effectOnOff->setChecked(( presetParams.at( DEVICE_ONOFF ) & 0x02 )>>1 );
    ui->delayOnOff->setChecked( ( presetParams.at( DEVICE_ONOFF ) & 0x04 )>>2 );
    ui->reverbOnOff->setChecked( ( presetParams.at( DEVICE_ONOFF ) & 0x08 )>>3 );

}

void MainWindow::decodeData( std::vector <unsigned char>* message, std::vector <unsigned char>* output ){

    int  count = message->size();

    // Find the remainder
    unsigned int remainder = count % 8 ;

    // Jump 8 bytes at a time starting at 10
    unsigned int loop = count - remainder ;
    unsigned int lastIndex = 0 ;

    // Make sure output vector is not already filled in
    output->clear();

    for( unsigned int i = 0 ; i < loop; i += 8 ){

        output->push_back( (message->at(i) & 0x01)?(message->at(i+1)^0x80):message->at(i+1) ) ;
        output->push_back( (message->at(i) & 0x02)?(message->at(i+2)^0x80):message->at(i+2) ) ;
        output->push_back( (message->at(i) & 0x04)?(message->at(i+3)^0x80):message->at(i+3) ) ;
        output->push_back( (message->at(i) & 0x08)?(message->at(i+4)^0x80):message->at(i+4) ) ;
        output->push_back( (message->at(i) & 0x10)?(message->at(i+5)^0x80):message->at(i+5) ) ;
        output->push_back( (message->at(i) & 0x20)?(message->at(i+6)^0x80):message->at(i+6) ) ;
        output->push_back( (message->at(i) & 0x40)?(message->at(i+7)^0x80):message->at(i+7) ) ;

        lastIndex = i + 8 ;
    }

    for( unsigned int j = 1; j < remainder  ; j++){
        output->push_back( (message->at(lastIndex) & ( unsigned char)j)?(message->at(lastIndex+j)^0x80):message->at(lastIndex+j) ) ;
    }

    std::cout << "presetParams  size: " << output->size() << '\n';
}

/*
    This NEEDS TESTING!
*/
void MainWindow::encodeData( std::vector <unsigned char>* data, std::vector <unsigned char>* message  ){

    int  count = data->size();

    std::cout << "encodeData count "<< count <<std::endl ;

    // Find the remainder
    unsigned int remainder = count % 7 ;

    // Jump 8 bytes at a time starting at 10
    unsigned int loop = count - remainder ;
    unsigned int last_i = 0 ;

    for( unsigned int i = 0,j = 0 ; i < loop; i += 7 ){

        message->push_back(	( ( data->at(i+6) >> 7 ) << 6 ) |
        (( data->at(i+5) >> 7 ) << 5 ) |
        (( data->at(i+4) >> 7 ) << 4 ) |
        (( data->at(i+3) >> 7 ) << 3 ) |
        (( data->at(i+2) >> 7 ) << 2 ) |
        (( data->at(i+1) >> 7 ) << 1 ) |
        ( data->at(i) >> 7 )	);
        message->push_back( data->at(i) & ~0x80 );
        message->push_back( data->at(i+1) & ~0x80 ) ;
        message->push_back(data->at(i+2) & ~0x80 ) ;
        message->push_back( data->at(i+3) & ~0x80 ) ;
        message->push_back( data->at(i+4) & ~0x80 ) ;
        message->push_back( data->at(i+5) & ~0x80 ) ;
        message->push_back( data->at(i+6) & ~0x80 ) ;

        last_i = i + 7 ;
    }

    // Create the mask
    unsigned char mask ;

    for( unsigned int i  = last_i, j=0 ; i < ( last_i + remainder  ); i++,j++ ){
        mask |= ( data->at(i) >> 7 ) << j ;
    }
    message->push_back( mask ) ;
    for( unsigned int i  = last_i ; i < ( last_i + remainder  ); i++ ){
        message->push_back( data->at(i) & ~0x80 ) ;
    }
//    printMessage( message ) ;
}

/*
void MainWindow::encodeMainParams( std::vector <unsigned char>* message, std::vector <unsigned char>* mainParams ){

    std::cout <<"encodeMainParams"<<std::endl ;

    // 1st byte is made up of the 1st bit from following seven bytes
    message->at(10) =
    ( ( mainParams->at(6) >> 7 ) << 6 ) |
    (( mainParams->at(5) >> 7 ) << 5 ) |
    (( mainParams->at(4) >> 7 ) << 4 ) |
    (( mainParams->at(3) >> 7 ) << 3 ) |
    (( mainParams->at(2) >> 7 ) << 2 ) |
    (( mainParams->at(1) >> 7 ) << 1 ) |
    ( mainParams->at(0) >> 7 )	;
    message->at(11) = mainParams->at(0) & ~0x80 ;
    message->at(12) = mainParams->at(1) & ~0x80 ;
    message->at(13) = mainParams->at(2) & ~0x80 ;
    message->at(14) = mainParams->at(3) & ~0x80 ;
    message->at(15) = mainParams->at(4) & ~0x80 ;
    message->at(16) = mainParams->at(5) & ~0x80 ;
    message->at(17) = mainParams->at(6) & ~0x80 ;

    message->at(18) =
    ( ( mainParams->at(13) >> 7 ) << 6 ) |
    (( mainParams->at(12) >> 7 ) << 5 ) |
    (( mainParams->at(11) >> 7 ) << 4 ) |
    (( mainParams->at(10) >> 7 ) << 3 ) |
    (( mainParams->at(9) >> 7 ) << 2 ) |
    (( mainParams->at(8) >> 7 ) << 1 ) |
    ( mainParams->at(7) >> 7 )	;
    message->at(19) = mainParams->at(7) & ~0x80 ;
    message->at(20) = mainParams->at(8) & ~0x80 ;
    message->at(21) = mainParams->at(9) & ~0x80 ;
    message->at(22) = mainParams->at(10) & ~0x80 ;
    message->at(23) = mainParams->at(11) & ~0x80 ;
    message->at(24) = mainParams->at(12) & ~0x80 ;
    message->at(25) = mainParams->at(13) & ~0x80 ;

    message->at(26) =
    (( mainParams->at(14) >> 7 ) << 1 ) |
    ( mainParams->at(15) >> 7 )	;
    message->at(27) = mainParams->at(14) & ~0x80 ;
    message->at(28) = mainParams->at(15) & ~0x80 ;

}
*/

void MainWindow::setMainParam( int index, int value ){
std::vector <unsigned char> message ;
std::vector <unsigned char> mainParams ;

    std::cout <<"setMainParam"<<std::endl ;

    std::vector<unsigned char> sysex ( 1,REQUEST_MAIN_PARAMS ) ;

    sendSysex( &sysex ) ;

    waitMidiIn( &message, TRANSMIT_MAIN_PARAMS ) ;

    decodeData( &message, &mainParams );

    std::cout << "Setting parameter " << index << " = " << value << std::endl ;

    mainParams.at( index ) = value ;

}


/*
 *  Here ideally we need to keep current value and just
 * or in the changed value
 */
void MainWindow::midi_set_devices_on_off( int bitOffset, bool value ){


    // Get last value

    presetParams[DEVICE_ONOFF] ^= (-value ^ presetParams[DEVICE_ONOFF]) & (1 << bitOffset);

    midi_transmit_single_parameter( MIDI_SELECT_TARGET, MIDI_SELECT_TARGET_PRESET, DEVICE_ONOFF, presetParams[DEVICE_ONOFF] ) ;

}

/*
 *  Generic function that wraps the appropriate Sysex manufacturers ID around message and the tailing F7
 *
 *  It is best to hard wire or can we append vectors
 */

void MainWindow::sendSysex ( std::vector<unsigned char>* message ){

    unsigned char sysex[] = { 0xF0, 0x00, 0x01, 0x05, 0x01, 0x00,0x02, 0x00, 0xF7 } ;

    std::vector<unsigned char> midiMessage( sysex, sysex + sizeof(sysex) / sizeof(unsigned char) );

    std::vector<unsigned char>::iterator it = midiMessage.begin() ;

    midiMessage.insert( it+8, message->begin(), message->end()) ;

    // Sorted send it
    midiout->sendMessage( &midiMessage ) ;
}


void MainWindow::midi_transmit_single_parameter( unsigned char selectTarget, unsigned char target, unsigned char index, unsigned char value ){

    unsigned char lsb = value & 0x0F ;
    unsigned char msb = ( value & 0xF0 ) >> 4  ;

    unsigned char sysex[] = { TRANSMIT_SINGLE_PARAM, MIDI_FILE_VERSION,target, index, lsb, msb  } ;

    std::vector<unsigned char> midiMessage(sysex,sysex + sizeof(sysex) / sizeof(unsigned char) );

    sendSysex( &midiMessage ) ;

//    std::cout << "Transmit " <<( int) selectTarget << " " << ( int)target << " "<< ( int)index << " " << ( int)value << std::endl ;
}

void MainWindow::midiSetup(){

    std::cout <<"MIDISetup"<<std::endl ;

        // RtMidiIn constructor
        try {
            midiin = new RtMidiIn();
        }
        catch ( RtMidiError &error ) {
            error.printMessage();
            exit( EXIT_FAILURE );
        }
        // Check inputs.
        unsigned int nPorts = midiin->getPortCount();
//        std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
        std::string portName;
        for ( unsigned int i=0; i<nPorts; i++ ) {
            try {
                portName = midiin->getPortName(i);
                 // different member versions of find in the same order as above:

                if ( portName.find( "Black Box") !=std::string::npos)
                {
                    std::cout << "\nThis port is Black Box " << i <<"\n";
                  //  inPort = i ;
                    midiin->openPort( i );
                    midiin->ignoreTypes( false, false, false );
                }


            }
            catch ( RtMidiError &error ) {
                error.printMessage();
            }
  //          std::cout << " Input Port #" << i+1 << ": " << portName << '\n';
        }

        // RtMidiOut constructor
        try{
            midiout = new RtMidiOut();
        }
        catch ( RtMidiError &error ) {
            error.printMessage();
            exit( EXIT_FAILURE );
        }
        // Check outputs.
        nPorts = midiout->getPortCount();
    //    std::cout << "\nThere are " << nPorts << " MIDI output ports available.\n";
        for ( unsigned int i=0; i<nPorts; i++ ) {
            try {
                portName = midiout->getPortName(i);
                if ( portName.find( "Black Box") !=std::string::npos)
                {
                    std::cout << "\nThis port is Black Box " << i <<"\n";
           //         outPort = i ;
                    midiout->openPort( i );
                }
            }
            catch (RtMidiError &error) {
                error.printMessage();
            }
      //      std::cout << " Output Port #" << i+1 << ": " << portName << '\n';
        }
        //std::cout << '\n';

}



