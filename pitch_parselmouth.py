import numpy as np
from matplotlib import pyplot as plt
import parselmouth
from scipy.io.wavfile import read, write

frame_dur=0.032
pitch_ceil=750

def draw_spectrogram(spectrogram, dynamic_range=70):
    X, Y = spectrogram.x_grid(), spectrogram.y_grid()
    sg_db = 10 * np.log10(spectrogram.values)
    plt.pcolormesh(X, Y, sg_db, vmin=sg_db.max() - dynamic_range, cmap='viridis')
    plt.ylim([spectrogram.ymin, spectrogram.ymax])
    plt.xlabel("time [s]")
    plt.ylabel("frequency [Hz]")

def draw_pitch(pitch):
    # Extract selected pitch contour, and
    # replace unvoiced samples by NaN to not plot
    pitch_values = pitch.selected_array['frequency']
    pitch_values[pitch_values==0] = np.nan
    plt.plot(pitch.xs(), pitch_values, 'o', markersize=5, color='w')
    plt.plot(pitch.xs(), pitch_values, 'o', markersize=2)
    plt.grid(False)
    plt.ylim(0, pitch.ceiling)
    plt.ylabel("fundamental frequency [Hz]")

def compute_pitch(wav_file,frame_dur,pitch_max):
    ### returns pitch contour value, pitch value = 0 for unvoiced frames
    snd=parselmouth.Sound(wav_file)
    pitch=snd.to_pitch(time_step=frame_dur,pitch_ceiling=pitch_max)
    spectrogram=snd.to_spectrogram()
    ### Comment out below section if dont' want to plot the pitch contour ###
    plt.figure()
    draw_spectrogram(spectrogram)
    plt.twinx()
    draw_pitch(pitch)
    plt.xlim([snd.xmin, snd.xmax])
    #plt.savefig()
    ####################
    return pitch.selected_array['frequency']

pitch_value=compute_pitch("sample_wav.wav", frame_dur,pitch_max)
