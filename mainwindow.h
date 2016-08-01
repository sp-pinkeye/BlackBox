#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <RtMidi.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_selectAmpModel_currentIndexChanged(int index);

    void on_dialDrive_valueChanged(int value);

    void on_dialBass_valueChanged(int value);

    void on_dialMid_valueChanged(int value);

    void on_dialTreble_valueChanged(int value);

    void on_dialVolume_valueChanged(int value);

    void on_selectPreset_valueChanged(int arg1);

    void on_loadPreset_clicked();

    void on_savePreset_clicked();

    void on_delayOnOff_clicked(bool checked);

    void on_dialDelayTime_valueChanged(int value);

    void on_dialDelayRepeats_valueChanged(int value);

    void on_dialDelayLevel_valueChanged(int value);

    void on_dialDrumsDelay_valueChanged(int value);

    void on_selectEffect_currentIndexChanged(int index);

    void on_dialSpeed_valueChanged(int value);

    void on_dialDepth_valueChanged(int value);

    void on_dialWetDry_valueChanged(int value);

    void on_reverbOnOff_clicked(bool checked);

    void on_dialReverbTime_valueChanged(int value);

    void on_dialReverbHiCut_valueChanged(int value);

    void on_dialReverbLevel_valueChanged(int value);

    void on_dialCompressor_valueChanged(int value);

    void on_effectOnOff_clicked(bool checked);

    void on_ampOnOff_clicked(bool checked);

    // MIDI STuff

    void midiSetup() ;
    void printMessage( std::vector <unsigned char>* message, bool hex=false );

    void getMainParams( );
    void printMainParams( );
    void getPresetEdit( );
    void savePreset( );

    void printPresetEdit( ) ;
    void updateDisplay() ;

    void midi_transmit_single_parameter( unsigned char selectTarget, unsigned char target, unsigned char index, unsigned char value ) ;
    void midi_set_devices_on_off( int bitOffset, bool value ) ;
    void sendSysex ( std::vector<unsigned char>* message ) ;
    void waitMidiIn(std::vector <unsigned char>* message, unsigned char wait);

    void setMainParam( int index, int value ) ;


   // void displayMainParams();

//    void encodeMainParams( std::vector <unsigned char>* message, std::vector <unsigned char>* mainParams );
    //void transmitMainParams( std::vector <unsigned char>* message );

    void decodeData( std::vector <unsigned char>* message, std::vector <unsigned char>* output );
    void encodeData( std::vector <unsigned char>* data, std::vector <unsigned char>* message ) ;

private:
    Ui::MainWindow *ui;
    RtMidiIn *midiin ;
    RtMidiOut *midiout ;
    std::vector<unsigned char>   mainParams ;
    std::vector<unsigned char>   presetParams ;

    /* These are definitions of MIDI events for the Blackbox
     */
    static const unsigned char TRANSMIT_SINGLE_PARAM    =  0x01 ;

    static const unsigned char TRANSMIT_PRESET          =  0x02 ;
    static const unsigned char REQUEST_STORED_PRESET    =  0x05 ;

    static const unsigned char SELECT_DRUMBEAT          =  0x08 ;
    static const unsigned char SELECT_PRESET            =  0x09 ;

    static const unsigned char REQUEST_PRESET_EDIT      =  0x0A ;
    static const unsigned char TRANSMIT_PRESET_EDIT     =  0x0B ;

    static const unsigned char REQUEST_MAIN_PARAMS      =  0x0E ;
    static const unsigned char TRANSMIT_MAIN_PARAMS     =  0x0F ;

    static const unsigned char SAVE_COMPLETE            =  0x11 ;
    static const unsigned char FACTORY_TO_USER          =  0x13 ;
    static const unsigned char USER_TO_FACTORY          =  0x14 ;


    static const unsigned char MIDI_FILE_VERSION        =  0x02 ;


    static const unsigned char SELECT_FACTORY_PRESET   =  0x00 ;
    static const unsigned char SELECT_USER_PRESET   =  0x01 ;

    static const unsigned char MIDI_SELECT_TARGET = 10 ;                // Param index
    static const unsigned char MIDI_SELECT_TARGET_PRESET =  0x00 ;
    static const unsigned char MIDI_SELECT_TARGET_MAIN   =  0x02 ;


    /*
     *  Main Params index into vector
     */
    static const unsigned char	MAIN_PARAMS_ACTIVE_PRESET       = 0;
    static const unsigned char	MAIN_PARAMS_ACTIVE_DRUMS        = 1;
    static const unsigned char	MAIN_PARAMS_GUITAR_DRUM_BALANCE  = 2;
    static const unsigned char	MAIN_PARAMS_FOOTSWITCH_1        = 3;
    static const unsigned char	MAIN_PARAMS_FOOTSWITCH_2        = 4;
    static const unsigned char	MAIN_PARAMS_FX_SYNC_SOURCE      = 5;
    static const unsigned char	MAIN_PARAMS_FX_INPUT_SOURCE     = 6;
    static const unsigned char	MAIN_PARAMS_GATE                = 7;
    static const unsigned char	MAIN_PARAMS_SEND_MIDI_CLOCK     = 8;
    static const unsigned char	MAIN_PARAMS_LINK_DRUMBEAT       = 9;
    static const unsigned char	MAIN_PARAMS_TEMPO_SOURCE        = 10;
    static const unsigned char	MAIN_PARAMS_GLOBAL_TEMPO        = 11;
    static const unsigned char	MAIN_PARAMS_MIDI_CHANNEL        = 12;

    // PARAMS VALUES
    static const unsigned char	MAIN_PARAMS_FX_SYNC_INT         = 0;
    static const unsigned char	MAIN_PARAMS_FX_SYNC_EXT         = 1;
    static const unsigned char	MAIN_PARAMS_FX_INPUT_GUITAR     = 0;
    static const unsigned char	MAIN_PARAMS_FX_INPUT_MIC        = 1;

    /*
     *  Preset index into vector
     */
    /* Now the AMP values, each will have an index */
    static const unsigned char AMP_MODEL            =  0 ;
    static const unsigned char AMP_DRIVE            =  1 ;
    static const unsigned char AMP_BASS             =  2 ;
    static const unsigned char AMP_MID              =  16 ;
    static const unsigned char AMP_TREBLE           =  3 ;
    static const unsigned char AMP_VOLUME           =  12 ;
    static const unsigned char COMPRESSOR_LEVEL     =  19 ;

    // Delay
    static const unsigned char DELAY_TIME           =  8 ;    // Param value
    static const unsigned char DELAY_REPEATS        =  9 ;    // Param value
    static const unsigned char DELAY_LEVEL          =  10 ;    // Param value
    static const unsigned char DRUMS_DELAY          =  11 ;    // Param value

    // Effects
    static const unsigned char EFFECTS_SELECT       =  4 ;    // Param value
    static const unsigned char EFFECTS_SPEED        =  5 ;    // Param value
    static const unsigned char EFFECTS_DEPTH        =  6 ;    // Param value
    static const unsigned char EFFECTS_LEVEL        =  7 ;    // Param value

    // Reverb
    static const unsigned char REVERB_TIME          =  21 ;    // Param value
    static const unsigned char REVERB_HICUT         =  22 ;    // Param value
    static const unsigned char REVERB_LEVEL         =  20 ;    // Param value

       // (bit 0 = amp, bit 1 = effect, bit 2 = delay, bit 3 = reverb)
    static const unsigned char DEVICE_ONOFF        =  15 ;    // Param value
    static const unsigned char AMP_ONOFF           =  0 ;    // Param value
    static const unsigned char EFFECT_ONOFF        =  1 ;    // Param value
    static const unsigned char DELAY_ONOFF         =  2 ;    // Param value
    static const unsigned char REVERB_ONOFF        =  3 ;    // Param value
};

#endif // MAINWINDOW_H
