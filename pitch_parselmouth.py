import numpy as np
from matplotlib import pyplot as plt
import parselmouth
from scipy.io.wavfile import read, write

frame_dur=0.032
pitch_max=750

def draw_spectrogram(spectrogram, dynamic_range=70):
    X, Y = spectrogram.x_grid(), spectrogram.y_grid()
    sg_db = 10 * np.log10(spectrogram.values)
    plt.pcolormesh(X, Y, sg_db, vmin=sg_db.max() - dynamic_range, cmap='viridis')
    plt.ylim([spectrogram.ymin, spectrogram.ymax])
    plt.xlabel("time [s]")
    plt.ylabel("frequency [Hz]")

def draw_pitch_praat(pitch,computed_pitch=None):
    # Extract selected pitch contour, and
    # replace unvoiced samples by NaN to not plot
    pitch_values = pitch.selected_array['frequency']
    pitch_values[pitch_values==0] = np.nan
    plt.plot(pitch.xs(), pitch_values, 'o', markersize=5, color='w',label="praat")
    #plt.plot(pitch.xs(), pitch_values, 'o', markersize=2)
    if computed_pitch:
        plt.plot(pitch.xs(), computed_pitch,'o',markersize=5,color="r",label="computed")
        #plt.plot(pitch.xs(), computed_pitch, 'o', markersize=2)

    plt.grid(False)
    plt.ylim(0, pitch.ceiling)
    plt.legend(facecolor='white', framealpha=0.5)
    plt.ylabel("fundamental frequency [Hz]")

def compute_pitch_praat(wav_file,computed_pitch=None,draw_pitch_contour=True):
    '''
    Return pitch contour generated from praat; Draw pitch contour overlaid with spectrogram given the wav file
    wav_file(str): wav file name
    computed_pitch(numpy array): computed pitch values; optional
    draw_pitch_contour(bool): if True, draw pitch contour for praat pitch contour and computed pitch contour if computed_pitch is provided
    Return:
    pitch_values(numpy array): pitch values generated from praat
    '''
    snd=parselmouth.Sound(wav_file)
    pitch=snd.to_pitch(time_step=frame_dur,pitch_ceiling=pitch_max)
    spectrogram=snd.to_spectrogram()
    if draw_pitch_contour:
        plt.figure()
        draw_spectrogram(spectrogram)
        plt.twinx()
        draw_pitch_praat(pitch,computed_pitch)
        plt.xlim([snd.xmin, snd.xmax])
        plt.show()
    return pitch.selected_array['frequency']

pitch_value=compute_pitch_praat("sample_wav.wav")
