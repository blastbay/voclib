#define VOCLIB_IMPLEMENTATION
#include "../voclib.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int write_wave_file ( const char* filename, unsigned int sample_rate, unsigned int channels, float* buffer, drwav_uint64 frames, float gain )
{
    drwav_uint64 i;
    drwav_int16* converted;
    drwav* outfile;
    drwav_data_format format;
    format.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
    format.format = DR_WAVE_FORMAT_PCM;          // <-- Any of the DR_WAVE_FORMAT_* codes.
    format.channels = channels;
    format.sampleRate = sample_rate;
    format.bitsPerSample = 16;

    for ( i = 0; i < frames * channels; ++i )
    {
        buffer[i] *= gain;
        if ( buffer[i] < -1.0f )
        {
            buffer[i] = -1.0f;
        }
        else if ( buffer[i] > 1.0f )
        {
            buffer[i] = 1.0f;
        }
    }

    converted = ( drwav_int16* ) malloc ( ( size_t ) ( frames * channels ) * sizeof ( drwav_int16 ) );
    if ( converted == NULL )
    {
        return 0;
    }
    drwav_f32_to_s16 ( converted, buffer, ( size_t ) frames * channels );

    outfile = drwav_open_file_write_sequential ( filename, &format, frames * channels );
    if ( outfile == NULL )
    {
        free ( converted );
        return 0;
    }

    if ( drwav_write ( outfile, frames * channels, &converted[0] ) != frames * channels )
    {
        drwav_close ( outfile );
        free ( converted );
        return 0;
    }

    drwav_close ( outfile );
    free ( converted );
    return 1;
}

float vocshell_find_peak ( float* buffer, drwav_uint64 frames )
{
    drwav_uint64 i;
    float peak = 0.0f;

    for ( i = 0; i < frames; ++i )
    {
        float x = ( float ) fabs ( buffer[i] );
        if ( x > peak )
        {
            peak = x;
        }
    }
    return peak;
}

int main ( int argc, const char** argv )
{
    unsigned long bands = 24;
    unsigned long filters_per_band = 4;
    double reaction_time = 0.03;
    double formant_shift = 1.0;
    voclib_instance vocoder;
    drwav_uint64 carrier_frames, modulator_frames;
    unsigned int carrier_sample_rate, carrier_channels, modulator_sample_rate, modulator_channels;
    float* carrier_buffer;
    float* modulator_buffer;
    const char* carrier_filename = NULL;
    const char* modulator_filename = NULL;
    const char* output_filename = NULL;
    int i;
    float carrier_peak;
    float output_peak;

    if ( argc == 1 )
    {
        printf ( "Usage:\n\nMandatory parameters:\n-c carrier file\n-m modulator file\n-o output file\n\nOptional parameters:\n-b number of bands\n-f filters per band\n-r reaction time (in seconds)\n-s formant shift (in octaves)\n" );
        return 0;
    }

    for ( i = 1; i < argc; ++i )
    {
        if ( strcmp ( argv[i], "-c" ) == 0 && i < ( argc - 1 ) )
        {
            ++i;
            carrier_filename = argv[i];
            continue;
        }
        if ( strcmp ( argv[i], "-m" ) == 0 && i < ( argc - 1 ) )
        {
            ++i;
            modulator_filename = argv[i];
            continue;
        }
        if ( strcmp ( argv[i], "-o" ) == 0 && i < ( argc - 1 ) )
        {
            ++i;
            output_filename = argv[i];
            continue;
        }
        if ( strcmp ( argv[i], "-b" ) == 0 && i < ( argc - 1 ) )
        {
            ++i;
            bands = strtoul ( argv[i], NULL, 10 );
            if ( bands < 4 || bands > VOCLIB_MAX_BANDS )
            {
                printf ( "Error: The bands parameter is out of range.\nThe allowed range is between 4 and %d (inclusive).\n", VOCLIB_MAX_BANDS );
                return 1;
            }
            continue;
        }
        if ( strcmp ( argv[i], "-f" ) == 0 && i < ( argc - 1 ) )
        {
            ++i;
            filters_per_band = strtoul ( argv[i], NULL, 10 );
            if ( filters_per_band < 1 || filters_per_band > VOCLIB_MAX_FILTERS_PER_BAND )
            {
                printf ( "Error: The filters per band parameter is out of range.\nThe allowed range is between 1 and %d (inclusive).\n", VOCLIB_MAX_FILTERS_PER_BAND );
                return 1;
            }
            continue;
        }

        if ( strcmp ( argv[i], "-r" ) == 0 && i < ( argc - 1 ) )
        {
            ++i;
            reaction_time = strtod ( argv[i], NULL );
            if ( reaction_time < 0.002 || reaction_time > 2.0 )
            {
                printf ( "Error: The reaction time parameter is out of range.\nThe allowed range is between 0.002 and 2.0 (inclusive).\n" );
                return 1;
            }
            continue;
        }

        if ( strcmp ( argv[i], "-s" ) == 0 && i < ( argc - 1 ) )
        {
            ++i;
            formant_shift = strtod ( argv[i], NULL );
            if ( formant_shift < 0.25 || formant_shift > 4.0 )
            {
                printf ( "Error: The formant shift parameter is out of range.\nThe allowed range is between 0.25 and 4.0 (inclusive).\n" );
                return 1;
            }
            continue;
        }

    }

    if ( carrier_filename == NULL )
    {
        printf ( "Error: Missing carrier file (use the -c option followed by a filename).\n" );
        return 1;
    }
    if ( modulator_filename == NULL )
    {
        printf ( "Error: Missing modulator file (use the -m option followed by a filename).\n" );
        return 1;
    }
    if ( output_filename == NULL )
    {
        printf ( "Error: Missing output file (use the -o option followed by a filename).\n" );
        return 1;
    }

    carrier_buffer = drwav_open_file_and_read_pcm_frames_f32 ( carrier_filename, &carrier_channels, &carrier_sample_rate, &carrier_frames );
    if ( carrier_buffer == NULL )
    {
        printf ( "Error: Could not read carrier file \"%s\".\n", carrier_filename );
        return 1;
    }

    modulator_buffer = drwav_open_file_and_read_pcm_frames_f32 ( modulator_filename, &modulator_channels, &modulator_sample_rate, &modulator_frames );
    if ( modulator_buffer == NULL )
    {
        free ( carrier_buffer );
        printf ( "Error: Could not read modulator file \"%s\".\n", modulator_filename );
        return 1;
    }

    if ( carrier_channels > 2 )
    {
        free ( carrier_buffer );
        free ( modulator_buffer );
        printf ( "Error: The carrier has %u channels.\nThe maximum allowed number of channels is 2.\nThis restriction may be lifted in a future version.\n", carrier_channels );
        return 1;
    }
    if ( modulator_channels > 1 )
    {
        free ( carrier_buffer );
        free ( modulator_buffer );
        printf ( "Error: The modulator has %u channels.\nThe maximum allowed number of channels is 1.\nThis restriction may be lifted in a future version.\n", modulator_channels );
        return 1;
    }

    if ( modulator_sample_rate != carrier_sample_rate )
    {
        free ( carrier_buffer );
        free ( modulator_buffer );
        printf ( "Error: The carrier has a sample rate of %u and the modulator has %u.\nThe carrier and modulator currently need to have the same sample rate.\nThis restriction may be lifted in a future version.\n", carrier_sample_rate, modulator_sample_rate );
        return 1;
    }

    if ( modulator_frames > carrier_frames )
    {
        printf ( "Info: The modulator is longer than the carrier.\nThe output will only be as long as the carrier.\n" );
        modulator_frames = carrier_frames;
    }
    else if ( carrier_frames > modulator_frames )
    {
        printf ( "Info: The carrier is longer than the modulator.\nThe output will only be as long as the modulator.\n" );
        carrier_frames = modulator_frames;
    }

    assert ( modulator_frames == carrier_frames );
    assert ( modulator_channels == carrier_channels );
    assert ( modulator_sample_rate == carrier_sample_rate );

    voclib_initialize ( &vocoder, ( unsigned char ) bands, ( unsigned char ) filters_per_band, carrier_sample_rate, ( unsigned char ) carrier_channels );
    voclib_set_reaction_time ( &vocoder, ( float ) reaction_time );
    voclib_set_formant_shift ( &vocoder, ( float ) formant_shift );

    carrier_peak = vocshell_find_peak ( carrier_buffer, carrier_frames * carrier_channels );

    voclib_process ( &vocoder, carrier_buffer, modulator_buffer, carrier_buffer, ( unsigned int ) carrier_frames );

    free ( modulator_buffer );

    output_peak = vocshell_find_peak ( carrier_buffer, carrier_frames * carrier_channels );
    output_peak = carrier_peak / output_peak;

    if ( !write_wave_file ( output_filename, carrier_sample_rate, carrier_channels, carrier_buffer, carrier_frames, output_peak ) )
    {
        free ( carrier_buffer );
        printf ( "Error: Could not write output file \"%s\".\n", output_filename );
        return 1;
    }

    free ( carrier_buffer );

    printf ( "Success.\n" );
    return 0;
}
